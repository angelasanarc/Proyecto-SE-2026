#include "wifi_monitor.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "esp_log.h"

#define NVS_NS          "robot_cfg"
#define WIFI_GOT_IP_BIT BIT0

static const char *TAG = "wifi_mon";

/* ---- Sincronizacion WiFi ---- */
static EventGroupHandle_t s_wifi_eg;

/* ---- Telemetria ---- */
static SemaphoreHandle_t s_telem_mutex;
static robot_telemetry_t s_telem;

/* ---- Parametros en tiempo real ---- */
static SemaphoreHandle_t s_params_mutex;
static robot_params_t    s_params;
static bool              s_params_changed = false;

/* ---- MQTT ---- */
static esp_mqtt_client_handle_t s_mqtt           = NULL;
static volatile bool            s_mqtt_connected = false;

/* ---- NVS ---- */
static void nvs_save_params(const robot_params_t *p)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    size_t sz = sizeof(float);
    nvs_set_blob(h, "kp",  &p->kp, sz);
    nvs_set_blob(h, "ki",  &p->ki, sz);
    nvs_set_blob(h, "kd",  &p->kd, sz);
    nvs_set_i32 (h, "spd", (int32_t)p->base_speed);
    nvs_commit(h);
    nvs_close(h);
    ESP_LOGI(TAG, "NVS guardado kp=%.4f ki=%.4f kd=%.4f spd=%d",
             (double)p->kp, (double)p->ki, (double)p->kd, p->base_speed);
}

static void nvs_load_params(robot_params_t *p)
{
    p->kp         = CONTROL_KP;
    p->ki         = CONTROL_KI;
    p->kd         = CONTROL_KD;
    p->base_speed = BASE_SPEED;

    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return;
    size_t sz = sizeof(float);
    nvs_get_blob(h, "kp", &p->kp, &sz);
    sz = sizeof(float);
    nvs_get_blob(h, "ki", &p->ki, &sz);
    sz = sizeof(float);
    nvs_get_blob(h, "kd", &p->kd, &sz);
    int32_t spd;
    if (nvs_get_i32(h, "spd", &spd) == ESP_OK) p->base_speed = (int)spd;
    nvs_close(h);
    ESP_LOGI(TAG, "NVS cargado kp=%.4f ki=%.4f kd=%.4f spd=%d",
             (double)p->kp, (double)p->ki, (double)p->kd, p->base_speed);
}

/* ---- JSON helper ---- */
static float json_get_float(const char *json, const char *key)
{
    char search[32];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char *p = strstr(json, search);
    if (!p) return 0.0f;
    p += strlen(search);
    while (*p == ' ') p++;
    return (float)atof(p);
}

/* ---- Procesar comando recibido por MQTT ---- */
static void process_cmd(const char *txt)
{
    if (!strstr(txt, "\"cmd\":\"set\"")) return;

    robot_params_t np;
    xSemaphoreTake(s_params_mutex, portMAX_DELAY);
    np = s_params;
    xSemaphoreGive(s_params_mutex);

    if (strstr(txt, "\"kp\":"))    np.kp         = json_get_float(txt, "kp");
    if (strstr(txt, "\"ki\":"))    np.ki         = json_get_float(txt, "ki");
    if (strstr(txt, "\"kd\":"))    np.kd         = json_get_float(txt, "kd");
    if (strstr(txt, "\"speed\":")) np.base_speed = (int)json_get_float(txt, "speed");

    xSemaphoreTake(s_params_mutex, portMAX_DELAY);
    s_params         = np;
    s_params_changed = true;
    xSemaphoreGive(s_params_mutex);

    nvs_save_params(&np);
}

/* ---- MQTT event handler ---- */
static void mqtt_event_handler(void *arg, esp_event_base_t base,
                               int32_t id, void *data)
{
    esp_mqtt_event_handle_t ev = (esp_mqtt_event_handle_t)data;

    switch ((esp_mqtt_event_id_t)id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT conectado");
        s_mqtt_connected = true;
        esp_mqtt_client_subscribe(s_mqtt, MQTT_CMD_TOPIC, 0);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT desconectado");
        s_mqtt_connected = false;
        break;

    case MQTT_EVENT_DATA:
        if (ev->data_len > 0) {
            char buf[256] = {0};
            int  len = ev->data_len < (int)(sizeof(buf) - 1)
                       ? ev->data_len : (int)(sizeof(buf) - 1);
            memcpy(buf, ev->data, len);
            process_cmd(buf);
        }
        break;

    default:
        break;
    }
}

/* ---- WiFi event handler ---- */
static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t id, void *data)
{
    if (base == WIFI_EVENT) {
        if (id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if (id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGW(TAG, "WiFi desconectado, reconectando...");
            s_mqtt_connected = false;
            esp_wifi_connect();
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *ev = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&ev->ip_info.ip));
        xEventGroupSetBits(s_wifi_eg, WIFI_GOT_IP_BIT);
    }
}

/* ---- Task de telemetria: publica JSON a MQTT cada 100 ms ---- */
static void telemetry_task(void *arg)
{
    char json[420];
    char sens[64];

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (!s_mqtt_connected) continue;

        robot_telemetry_t t;
        xSemaphoreTake(s_telem_mutex, portMAX_DELAY);
        t = s_telem;
        xSemaphoreGive(s_telem_mutex);

        robot_params_t p;
        xSemaphoreTake(s_params_mutex, portMAX_DELAY);
        p = s_params;
        xSemaphoreGive(s_params_mutex);

        int off = snprintf(sens, sizeof(sens), "[");
        for (int i = 0; i < NUM_SENSORS; i++) {
            off += snprintf(sens + off, (int)sizeof(sens) - off,
                            "%d%s", t.sensors[i], i < NUM_SENSORS - 1 ? "," : "");
        }
        snprintf(sens + off, sizeof(sens) - off, "]");

        snprintf(json, sizeof(json),
                 "{\"err\":%d,\"pos\":%d,\"lspd\":%d,\"rspd\":%d,"
                 "\"corr\":%.2f,\"line\":%d,\"start\":%d,\"mot\":%d,"
                 "\"sens\":%s,"
                 "\"kp\":%.4f,\"ki\":%.4f,\"kd\":%.5f,\"speed\":%d}",
                 t.error, t.position, t.left_speed, t.right_speed,
                 (double)t.correction,
                 t.line_detected ? 1 : 0, t.start_active, t.motors_on,
                 sens,
                 (double)p.kp, (double)p.ki, (double)p.kd, p.base_speed);

        esp_mqtt_client_publish(s_mqtt, MQTT_TELEM_TOPIC, json, 0, 0, 0);
    }
}

/* ---- API publica ---- */
esp_err_t wifi_monitor_init(void)
{
    s_telem_mutex  = xSemaphoreCreateMutex();
    s_params_mutex = xSemaphoreCreateMutex();
    s_wifi_eg      = xEventGroupCreate();
    memset(&s_telem, 0, sizeof(s_telem));

    /* NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ret = nvs_flash_erase();
        if (ret != ESP_OK) return ret;
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) return ret;
    nvs_load_params(&s_params);
    s_params_changed = false;

    /* WiFi STA */
    ret = esp_netif_init();
    if (ret != ESP_OK) return ret;

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK) return ret;

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&wifi_cfg);
    if (ret != ESP_OK) return ret;

    ret = esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL);
    if (ret != ESP_OK) return ret;

    ret = esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, NULL);
    if (ret != ESP_OK) return ret;

    wifi_config_t sta_cfg = {0};
    strncpy((char *)sta_cfg.sta.ssid,     STA_WIFI_SSID,
            sizeof(sta_cfg.sta.ssid)     - 1);
    strncpy((char *)sta_cfg.sta.password, STA_WIFI_PASSWORD,
            sizeof(sta_cfg.sta.password) - 1);

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) return ret;

    ret = esp_wifi_set_config(WIFI_IF_STA, &sta_cfg);
    if (ret != ESP_OK) return ret;

    ret = esp_wifi_start();
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Conectando a WiFi SSID=%s ...", STA_WIFI_SSID);
    EventBits_t bits = xEventGroupWaitBits(s_wifi_eg, WIFI_GOT_IP_BIT,
                                           pdFALSE, pdTRUE, pdMS_TO_TICKS(15000));

    if (!(bits & WIFI_GOT_IP_BIT)) {
        ESP_LOGW(TAG, "WiFi sin IP tras 15s — continuando sin red");
        return ESP_FAIL;
    }

    /* MQTT — ID unico por dispositivo usando MAC */
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    char client_id[32];
    snprintf(client_id, sizeof(client_id),
             "robot_%02x%02x%02x", mac[3], mac[4], mac[5]);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri     = MQTT_BROKER_URI,
        .credentials.client_id  = client_id,
    };
    s_mqtt = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(s_mqtt, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_mqtt);

    /* Task en core 0 junto con WiFi/MQTT */
    xTaskCreatePinnedToCore(telemetry_task, "telemetry", 4096, NULL, 1, NULL, 0);

    ESP_LOGI(TAG, "MQTT iniciado broker=%s topic=%s", MQTT_BROKER_URI, MQTT_TELEM_TOPIC);
    return ESP_OK;
}

void wifi_monitor_update(const robot_telemetry_t *telem)
{
    if (xSemaphoreTake(s_telem_mutex, 0) == pdTRUE) {
        s_telem = *telem;
        xSemaphoreGive(s_telem_mutex);
    }
}

void wifi_monitor_get_params(robot_params_t *out)
{
    xSemaphoreTake(s_params_mutex, portMAX_DELAY);
    *out = s_params;
    xSemaphoreGive(s_params_mutex);
}

bool wifi_monitor_params_changed(robot_params_t *out)
{
    bool changed = false;
    xSemaphoreTake(s_params_mutex, portMAX_DELAY);
    if (s_params_changed) {
        *out             = s_params;
        s_params_changed = false;
        changed          = true;
    }
    xSemaphoreGive(s_params_mutex);
    return changed;
}

#include "wifi_monitor.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_log.h"

#define MAX_CLIENTS 4
#define NVS_NS      "robot_cfg"

static const char *TAG = "wifi_mon";

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

/* ---- Telemetria ---- */
static SemaphoreHandle_t s_telem_mutex;
static robot_telemetry_t s_telem;

/* ---- Parametros en tiempo real ---- */
static SemaphoreHandle_t s_params_mutex;
static robot_params_t    s_params;
static bool              s_params_changed = false;

/* ---- Clientes WebSocket ---- */
static SemaphoreHandle_t s_fds_mutex;
static int               s_fds[MAX_CLIENTS];
static httpd_handle_t    s_server = NULL;

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

/* ---- Extraccion simple de campos JSON ---- */
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

/* ---- WebSocket handler ---- */
static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        int fd = httpd_req_to_sockfd(req);
        xSemaphoreTake(s_fds_mutex, portMAX_DELAY);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (s_fds[i] < 0) { s_fds[i] = fd; break; }
        }
        xSemaphoreGive(s_fds_mutex);
        ESP_LOGI(TAG, "WS conectado fd=%d", fd);
        return ESP_OK;
    }

    uint8_t buf[256] = {0};
    httpd_ws_frame_t frame = { .payload = buf };
    esp_err_t ret = httpd_ws_recv_frame(req, &frame, sizeof(buf) - 1);
    if (ret != ESP_OK) return ret;

    if (frame.type == HTTPD_WS_TYPE_CLOSE) {
        int fd = httpd_req_to_sockfd(req);
        xSemaphoreTake(s_fds_mutex, portMAX_DELAY);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (s_fds[i] == fd) { s_fds[i] = -1; break; }
        }
        xSemaphoreGive(s_fds_mutex);
        return ESP_OK;
    }

    if (frame.type == HTTPD_WS_TYPE_TEXT && frame.len > 0) {
        buf[frame.len] = '\0';
        const char *txt = (const char *)buf;

        if (strstr(txt, "\"cmd\":\"set\"")) {
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
    }

    return ESP_OK;
}

/* ---- HTTP: sirve la pagina web ---- */
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)index_html_start,
                           index_html_end - index_html_start);
}

/* ---- Task de telemetria: envia JSON a clientes WS cada 100 ms ---- */
static void telemetry_task(void *arg)
{
    char json[420];
    char sens[64];

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (!s_server) continue;

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

        httpd_ws_frame_t ws_frame = {
            .final = true, .fragmented = false,
            .type  = HTTPD_WS_TYPE_TEXT,
            .payload = (uint8_t *)json,
            .len     = strlen(json),
        };

        xSemaphoreTake(s_fds_mutex, portMAX_DELAY);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (s_fds[i] < 0) continue;
            if (httpd_ws_send_frame_async(s_server, s_fds[i], &ws_frame) != ESP_OK) {
                ESP_LOGW(TAG, "Cliente fd=%d desconectado", s_fds[i]);
                s_fds[i] = -1;
            }
        }
        xSemaphoreGive(s_fds_mutex);
    }
}

/* ---- Evento WiFi ---- */
static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t id, void *data)
{
    if (id == WIFI_EVENT_AP_STACONNECTED)
        ESP_LOGI(TAG, "Dispositivo conectado al AP");
    else if (id == WIFI_EVENT_AP_STADISCONNECTED)
        ESP_LOGI(TAG, "Dispositivo desconectado del AP");
}

/* ---- API publica ---- */
esp_err_t wifi_monitor_init(void)
{
    s_telem_mutex  = xSemaphoreCreateMutex();
    s_params_mutex = xSemaphoreCreateMutex();
    s_fds_mutex    = xSemaphoreCreateMutex();
    memset(&s_telem, 0, sizeof(s_telem));
    for (int i = 0; i < MAX_CLIENTS; i++) s_fds[i] = -1;

    /* NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    nvs_load_params(&s_params);
    s_params_changed = false;

    /* Red */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL));

    wifi_config_t ap_cfg = {
        .ap = {
            .channel        = WIFI_CHANNEL,
            .max_connection = WIFI_MAX_STA,
            .authmode       = WIFI_AUTH_WPA2_PSK,
        }
    };
    strncpy((char *)ap_cfg.ap.ssid,     WIFI_SSID,     sizeof(ap_cfg.ap.ssid)     - 1);
    strncpy((char *)ap_cfg.ap.password, WIFI_PASSWORD, sizeof(ap_cfg.ap.password) - 1);
    ap_cfg.ap.ssid_len = (uint8_t)strlen(WIFI_SSID);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    /* HTTP + WebSocket */
    httpd_config_t srv_cfg = HTTPD_DEFAULT_CONFIG();
    srv_cfg.server_port = 80;
    ESP_ERROR_CHECK(httpd_start(&s_server, &srv_cfg));

    static const httpd_uri_t uri_index = {
        .uri = "/", .method = HTTP_GET, .handler = index_handler
    };
    static const httpd_uri_t uri_ws = {
        .uri = "/ws", .method = HTTP_GET,
        .handler = ws_handler, .is_websocket = true
    };
    httpd_register_uri_handler(s_server, &uri_index);
    httpd_register_uri_handler(s_server, &uri_ws);

    xTaskCreate(telemetry_task, "telemetry", 4096, NULL, 1, NULL);

    ESP_LOGI(TAG, "AP iniciado  SSID=%s  IP=192.168.4.1  puerto=80", WIFI_SSID);
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

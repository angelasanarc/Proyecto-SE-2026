## 1. Información general del documento

### 1.1 Nombre del proyecto

El proyecto corresponde al desarrollo de un robot seguidor de línea tipo velocista basado en un microcontrolador ESP32. El sistema está diseñado para detectar una línea de referencia sobre una pista, procesar esta información mediante firmware y controlar dos motores DC de manera diferencial para mantener el seguimiento autónomo de la trayectoria.

El firmware fue desarrollado en lenguaje C utilizando ESP-IDF como framework de desarrollo dentro del entorno PlatformIO. La implementación se organiza en componentes funcionales separados, siguiendo una arquitectura modular que permite diferenciar la lectura de sensores, el control de motores, el cálculo de control, la visualización en pantalla OLED, la lectura de la señal de inicio, el registro de datos, la comunicación inalámbrica, la configuración de parámetros y el monitoreo remoto del sistema.

### 1.2 Objetivo del firmware

El objetivo del firmware es implementar la lógica necesaria para que el robot pueda operar de forma autónoma como seguidor de línea. Para ello, el sistema inicializa los periféricos principales, lee los sensores infrarrojos, determina la posición relativa de la línea, calcula el error de seguimiento, ejecuta un controlador proporcional-integral-derivativo y genera señales de velocidad para los motores izquierdo y derecho.

El firmware también debe garantizar que el robot solo se mueva cuando exista una condición válida de inicio. Para esto, se utiliza una entrada física de START que habilita o deshabilita la operación de los motores. Si la señal de inicio no está activa o si la línea no es detectada por los sensores durante el tiempo permitido, el sistema ejecuta una parada segura, detiene los motores y reinicia el estado interno del controlador.

Adicionalmente, el firmware permite visualizar información relevante en una pantalla OLED mediante comunicación I2C y registrar datos de operación mediante salida serial. Esta información incluye el estado de inicio, la detección de línea, la posición calculada, el error, la corrección de control, las velocidades aplicadas a los motores y el estado individual de los sensores.

Como complemento a la visualización local, el firmware implementa comunicación inalámbrica mediante WiFi y MQTT. Esta comunicación permite publicar telemetría del robot hacia una interfaz web y recibir parámetros de configuración en tiempo real. A través de este mecanismo, el sistema puede enviar variables como error, posición, velocidades, corrección, estado de línea, estado de START, estado de motores y sensores, además de recibir ajustes de `kp`, `ki`, `kd` y velocidad base.

### 1.3 Alcance del firmware

El firmware cubre las funciones principales necesarias para el funcionamiento del robot seguidor de línea. Estas funciones incluyen la inicialización del sistema, la lectura de sensores, la detección de línea, el cálculo de posición y error, el control de trayectoria mediante un algoritmo PID, el control diferencial de motores mediante PWM, la lectura de una entrada física de inicio, la visualización de estado en pantalla OLED, el registro de información de depuración y la comunicación inalámbrica con una interfaz web.

El sistema opera mediante un ciclo principal temporizado. En cada iteración se leen los sensores, se verifica la condición de inicio, se valida si la línea está presente y, si las condiciones son correctas, se calcula la corrección de control y se actualizan las velocidades de los motores. Si alguna condición necesaria no se cumple, el firmware detiene los motores como acción de seguridad o mantiene una estrategia de recuperación durante el tiempo configurado para pérdida de línea.

Dentro del ciclo de operación también se actualiza la información enviada a la pantalla OLED, se generan mensajes de logging y se envía telemetría al módulo de comunicación WiFi/MQTT. Esta telemetría permite que una página web externa visualice el estado del robot durante la operación.

Los parámetros principales del sistema, como pines, límites de velocidad, constantes iniciales del controlador, tiempo de control, configuración de sensores, dirección de la pantalla OLED y datos de comunicación, se encuentran definidos en el componente de configuración `robot_config`. Además, los parámetros de control `kp`, `ki`, `kd` y velocidad base pueden ser actualizados desde la interfaz web mediante mensajes MQTT y almacenados en memoria no volátil para conservar los ajustes después de reiniciar el sistema.

### 1.4 Relación con requisitos del sistema

El firmware implementa las funciones asociadas a los requisitos principales del sistema embebido. La lectura de sensores infrarrojos permite cumplir con el requisito de sensado físico real; el control de los motores mediante el driver TB6612FNG permite cumplir con el requisito de actuación electrónica; la pantalla OLED utiliza comunicación I2C para visualizar información del sistema; la salida serial permite observar información de operación y depuración; y la comunicación WiFi/MQTT permite integrar una interfaz web para monitoreo remoto y ajuste de parámetros.

El diseño modular del firmware facilita relacionar cada requisito con una parte específica del código. El componente `line_sensors` se asocia con la detección de línea y cálculo de posición; `line_control` se asocia con el cálculo de control PID; `motor_driver` se relaciona con la actuación sobre los motores; `start_input` gestiona la habilitación física de operación; `oled_display` permite visualizar estado del sistema; `logger` genera información de seguimiento; `wifi_monitor` gestiona la conexión WiFi, la comunicación MQTT, la telemetría, la recepción de parámetros y la persistencia de configuración; y `robot_config` centraliza los parámetros base del sistema.

## 2. Descripción general del sistema embebido

### 2.1 Descripción funcional del sistema

El sistema embebido desarrollado corresponde a un robot seguidor de línea tipo velocista basado en una ESP32. Su función principal es desplazarse de forma autónoma sobre una pista marcada con una línea de referencia, detectando la trayectoria mediante sensores infrarrojos y ajustando la velocidad de sus motores para mantenerse sobre dicha línea.

El robot utiliza un arreglo de sensores de línea como entrada principal del sistema. Estos sensores permiten identificar si la línea se encuentra centrada, desplazada hacia la izquierda, desplazada hacia la derecha o si no está siendo detectada. La información obtenida se procesa en el firmware para calcular una posición estimada de la línea y un error de seguimiento respecto al centro del robot.

A partir del error calculado, el firmware ejecuta un algoritmo de control de línea basado en PID. La corrección generada por el controlador se utiliza para modificar diferencialmente la velocidad de los motores izquierdo y derecho. De esta manera, el robot puede realizar correcciones de trayectoria en tiempo real, aumentando o disminuyendo la velocidad relativa de cada motor según la dirección en la que se encuentre la línea.

El sistema también incorpora una entrada física de inicio o `START`, encargada de habilitar la operación del robot. Mientras esta señal no se encuentre activa, el firmware mantiene los motores detenidos. Esta condición permite que el robot no inicie su movimiento de forma automática al encenderse, sino únicamente cuando se recibe la señal física correspondiente.

Adicionalmente, el sistema cuenta con una pantalla OLED conectada por I2C, utilizada para visualizar información relevante del estado del robot. Esta interfaz permite mostrar datos como el estado de operación, la detección de línea, la posición calculada, el error o valores asociados al control. De forma complementaria, el firmware utiliza salida serial mediante `printf` para registrar información de depuración y seguimiento durante las pruebas.

El sistema también implementa comunicación inalámbrica mediante WiFi y MQTT. Esta comunicación permite enviar telemetría del robot hacia una página web y recibir parámetros de configuración en tiempo real. Desde la interfaz web se pueden observar variables como error, posición, velocidades, corrección, estado de línea, estado de START, estado de motores y sensores. Además, se pueden modificar parámetros de control como `KP`, `KI`, `KD` y velocidad base.

La estrategia de seguridad principal consiste en detener los motores cuando no se cumplen las condiciones necesarias para operar. Esto ocurre cuando, por ejemplo, la señal de inicio no está activa o si la línea no es detectada por los sensores. En estos casos, el firmware ejecuta una parada segura para evitar que el robot continúe avanzando sin información confiable de la pista.

### 2.2 Diagrama de bloques del sistema embebido

El diagrama de bloques del sistema embebido representa la relación entre los elementos físicos del robot, los módulos principales del firmware y la interfaz remota de monitoreo. El flujo general inicia en los sensores infrarrojos, continúa con el procesamiento realizado por la ESP32 y finaliza con la actuación sobre los motores mediante el driver TB6612FNG.

![Diagrama de bloques del sistema embebido](DocImages/Esquematico.png)

El diagrama debe representar la conexión entre los sensores, la ESP32, el controlador PID, el driver de motores y los motores DC. También debe incluir la entrada física START, la pantalla OLED por I2C, la salida serial de depuración, la comunicación WiFi/MQTT y la página web de monitoreo.

Dentro de esta estructura, los sensores entregan información de entrada al firmware, la ESP32 procesa dicha información mediante los módulos de sensores y control, y la salida del sistema corresponde a señales de dirección y PWM enviadas hacia el driver de motores. La pantalla OLED permite visualizar información local, mientras que la comunicación WiFi/MQTT permite enviar telemetría y recibir ajustes de parámetros desde la página web.

### 2.3 Componentes de hardware relacionados con firmware

Los componentes de hardware que interactúan directamente con el firmware son la ESP32, los sensores infrarrojos de línea, la entrada física de inicio, el driver de motores TB6612FNG, los motores DC, la pantalla OLED, la salida serial de depuración, la comunicación WiFi y la fuente de alimentación.

La ESP32 actúa como unidad principal de procesamiento. En ella se ejecuta el firmware encargado de inicializar los módulos del sistema, leer los sensores, verificar la señal de inicio, calcular la posición de la línea, ejecutar el control PID, controlar los motores, actualizar la pantalla OLED, generar mensajes de depuración y gestionar la comunicación inalámbrica con la página web.

Los sensores infrarrojos permiten detectar la presencia o ausencia de la línea en diferentes posiciones del arreglo. Sus lecturas son procesadas por el módulo `line_sensors`, que interpreta el estado de cada sensor y calcula una posición estimada de la línea. Esta información constituye la entrada principal del algoritmo de seguimiento.

La entrada física START permite habilitar o deshabilitar el movimiento del robot. Esta señal es gestionada por el módulo `start_input` y funciona como una condición de seguridad operacional. Si la señal de inicio no está activa, el firmware mantiene los motores detenidos aunque el resto del sistema esté inicializado correctamente.

El driver de motores TB6612FNG recibe las señales generadas por la ESP32 para controlar los motores DC. Desde el firmware se envían señales de dirección y PWM para establecer la velocidad de cada motor. El módulo `motor_driver` se encarga de traducir las velocidades calculadas por el controlador en señales físicas hacia el driver.

Los motores DC son los actuadores principales del robot. El movimiento diferencial permite corregir la trayectoria y, cuando el robot detecta que la línea se desvía hacia un lado, el firmware modifica la velocidad relativa del motor izquierdo y del motor derecho para recuperar la trayectoria.

La pantalla OLED funciona como interfaz visual local del sistema. Está conectada mediante comunicación I2C y permite mostrar información de estado durante la operación. Esta pantalla es gestionada por el módulo `oled_display`, el cual actualiza los datos visibles para facilitar la supervisión del robot durante pruebas y demostración.

La salida serial permite registrar información de depuración y operación. Aunque no se utiliza como canal principal de comandos en la implementación actual, sí permite observar mensajes generados por el firmware, lo cual es útil para validar el funcionamiento del sistema y obtener evidencia durante pruebas.

La comunicación WiFi permite conectar la ESP32 con una página web de monitoreo mediante MQTT. Esta interfaz remota permite visualizar el estado del robot y ajustar parámetros de control durante la ejecución. El módulo `wifi_monitor` se encarga de gestionar la conexión, publicar telemetría, recibir comandos de configuración y conservar ciertos parámetros en memoria no volátil.

Las baterías proporcionan la energía necesaria para la ESP32, los sensores, la pantalla, el driver y los motores. Su estabilidad es importante para el correcto funcionamiento del robot, ya que variaciones de alimentación pueden afectar la lectura de sensores, la comunicación inalámbrica, la visualización y la respuesta de los motores.

### 2.4 Flujo general de funcionamiento

El funcionamiento general del sistema inicia con la inicialización de los módulos principales del firmware. Durante esta etapa se configuran los parámetros del sistema, los sensores de línea, el driver de motores, la entrada de inicio, la pantalla OLED, el sistema de logging y el módulo de comunicación WiFi/MQTT.

Una vez inicializado, el robot entra en un ciclo principal de ejecución. En cada iteración, el firmware lee el estado de la señal START para determinar si el movimiento del robot está habilitado. Si la señal de inicio no está activa, el sistema ejecuta una parada segura y mantiene los motores detenidos.

Cuando la señal START está activa, el firmware procede a leer los sensores infrarrojos. A partir de estas lecturas se determina si la línea está presente y se calcula su posición relativa. Si no se detecta línea, el sistema detiene los motores como medida de seguridad, reinicia el estado del controlador y registra la condición correspondiente.

Si la señal de inicio está activa y la línea es detectada correctamente, el firmware calcula el error de seguimiento respecto al centro del robot. Este error se entrega al módulo de control, donde se ejecuta el algoritmo PID para obtener una corrección de trayectoria.

La corrección calculada se combina con la velocidad base configurada para determinar la velocidad del motor izquierdo y del motor derecho. Posteriormente, estas velocidades se envían al módulo `motor_driver`, que actualiza las señales PWM y de dirección aplicadas al driver TB6612FNG.

Durante el ciclo de operación, el sistema también actualiza la pantalla OLED con información relevante, genera mensajes de depuración mediante salida serial y envía telemetría hacia la página web mediante WiFi/MQTT. Si desde la interfaz web se modifican parámetros como `KP`, `KI`, `KD` o velocidad base, el firmware recibe los nuevos valores, actualiza el controlador y continúa operando con la configuración ajustada.

Espacio para imagen del diagrama de flujo general del firmware:
![Diagrama de Flujo](DocsImages/FlujoFirmware.png)

## 3. Arquitectura de firmware

### 3.1 Enfoque arquitectónico

El firmware del robot seguidor de línea se desarrolló bajo un enfoque modular utilizando ESP-IDF en lenguaje C dentro del entorno PlatformIO. Aunque la lógica principal de operación se coordina desde `src/main.c`, las funciones específicas del sistema se encuentran separadas en componentes independientes dentro de la carpeta `components/`.

La arquitectura actual busca mantener una separación clara entre las responsabilidades principales del firmware: configuración general del sistema, calibración de sensores, lectura de sensores, control de línea, actuación sobre motores, lectura de la señal de inicio, visualización en pantalla OLED, registro de información mediante logger y comunicación inalámbrica con la interfaz web. Esta organización permite que cada bloque funcional del robot tenga una representación clara en el código.

El archivo `src/main.c` actúa como punto de entrada y coordinador principal del sistema. Desde este archivo se inicializan todos los módulos, se ejecuta el ciclo principal de control y se toman las decisiones de operación de acuerdo con la señal de inicio, la detección de línea, la salida del controlador y los parámetros recibidos desde la interfaz web.

El firmware opera mediante un ciclo principal temporizado. En cada iteración se leen los sensores de línea, se verifica la entrada física `START`, se determina si el robot puede moverse, se calcula la corrección de control cuando la línea está presente y se actualizan los motores. Además, de forma periódica se actualiza la pantalla OLED, se generan mensajes de seguimiento por salida serial y se envía telemetría mediante WiFi/MQTT hacia la página web.

Este enfoque permite cumplir con las funciones principales del sistema embebido sin concentrar toda la implementación en un único archivo. Aunque `main.c` contiene la lógica de coordinación, los detalles de sensores, calibración, motores, control, pantalla, entrada de inicio, logger, configuración y comunicación WiFi/MQTT se encuentran encapsulados en módulos especializados.

### 3.2 Estructura modular del firmware

La estructura modular del firmware se organiza alrededor de nueve bloques principales: `config`, `calibration`, `line_sensors`, `control`, `motor_driver`, `start_input`, `oled_display`, `logger` y `wifi_monitor`.

La estructura general del firmware es la siguiente:

![Estructura de Firmware](DocsImages/ArquitecturaFirmware.png)

Esta estructura permite que cada módulo tenga una interfaz pública definida en su archivo `.h` y una implementación específica en su archivo `.c`. De esta manera, `main.c` puede utilizar las funciones de cada componente sin depender directamente de los detalles internos de implementación.

`config` centraliza los parámetros de hardware, control y comunicación. Allí se definen pines, dirección I2C de la pantalla OLED, nivel activo de la señal START, número de sensores, constantes iniciales del controlador, velocidades máximas y mínimas, centro de línea, periodo de control y datos necesarios para la comunicación WiFi/MQTT.

La calibración de sensores se gestiona desde `calibration`. Esta parte del firmware permite obtener valores de referencia durante una fase inicial del sistema y normalizar las lecturas antes de calcular la posición de la línea. Su función es mejorar la interpretación de los sensores antes de que la información sea usada por el algoritmo de control.

`line_sensors` se encarga de la lectura y procesamiento de los sensores de línea. Este módulo entrega una estructura de datos con el estado de los sensores, la posición estimada de la línea, el error de seguimiento y una bandera que indica si la línea fue detectada.

Desde `control` se implementa el controlador de línea mediante la estructura `line_control_t`. Esta parte recibe el error calculado por los sensores y genera una corrección limitada entre valores mínimos y máximos configurados.

El control físico de los motores se realiza desde `motor_driver`, encargado de manejar el driver TB6612FNG mediante señales GPIO y PWM. Allí se inicializan los pines de control, se asignan velocidades para los motores izquierdo y derecho y se define la función de parada cuando el sistema lo requiere.

`start_input` gestiona la entrada física de inicio. Esta parte del firmware permite leer el estado del pin START y determinar si la operación del robot está habilitada.

La pantalla OLED es controlada mediante I2C desde `oled_display`. Allí se inicializa la interfaz visual local, se muestra información durante la calibración y se actualiza el estado del robot durante la operación.

`logger` genera mensajes de sistema y mensajes de control. Su función es entregar información útil para depuración, pruebas y evidencia de funcionamiento mediante salida serial.

Finalmente, `wifi_monitor` gestiona la comunicación inalámbrica del sistema. Esta parte del firmware inicializa la conexión WiFi, maneja la comunicación MQTT, publica telemetría del robot hacia la página web, recibe parámetros de configuración y conserva ciertos ajustes en memoria no volátil.

### 3.3 Responsabilidad Módulos

El archivo `src/main.c` es responsable de coordinar el funcionamiento general del firmware. En este archivo se inicializan los módulos, se crea la estructura de datos de sensores, se ejecuta la fase de calibración, se inicializa el controlador de línea, se ejecuta el ciclo principal y se toman decisiones sobre movimiento, recuperación o parada segura. También contiene funciones auxiliares como la limitación de velocidad y la detención segura del robot.

`robot_config` define los parámetros globales del sistema. En este módulo se encuentran valores como los pines de la pantalla OLED, START, TB6612FNG y sensores. También se definen parámetros de control, límites de velocidad, periodo de ejecución, configuración de sensores y datos de comunicación inalámbrica. Centralizar estos valores permite modificar el comportamiento del robot sin tener que buscar constantes dispersas en varios archivos.

La fase de calibración se desarrolla desde `qtr_calibration`. Durante esta etapa, el firmware toma muestras de los sensores, actualiza los valores de referencia y normaliza las lecturas. Esta información permite que el sistema procese los datos de línea con mayor consistencia antes de calcular la posición y el error de seguimiento.

`line_sensors` lee el arreglo de sensores y procesa sus valores normalizados. A partir de estas lecturas, determina qué sensores detectan la línea, calcula una posición ponderada, calcula el error respecto al centro definido en `LINE_CENTER_POSITION` y actualiza la bandera `line_detected`. Esta información se entrega a `main.c` mediante la estructura `line_sensor_data_t`.

La lógica del controlador PID se implementa en `line_control`. Allí se almacenan las ganancias proporcional, integral y derivativa, el error anterior, el acumulado integral, el filtrado derivativo y los límites de salida. La función de cómputo recibe el error actual y retorna una corrección que posteriormente se utiliza para calcular las velocidades de los motores.

Por otro lado, `motor_driver` se encarga de la actuación física del robot. Su responsabilidad es convertir las velocidades calculadas por el firmware en señales eléctricas para el driver TB6612FNG. Este módulo configura los pines de dirección, habilitación y PWM, permite asignar velocidad a cada motor y ofrece una función de parada para detener ambos motores.

`start_input` tiene la responsabilidad de leer la señal física de inicio. Esta señal funciona como habilitación de movimiento. Si la entrada START no está activa, `main.c` mantiene los motores detenidos y reinicia el controlador. Esta lógica evita que el robot se desplace al encenderse sin una autorización física de operación.

Para la interfaz visual local está `oled_display`. Este bloque inicializa la pantalla OLED y actualiza información de estado, como la fase de calibración, la condición de START, el estado de los motores, la información de sensores y las velocidades aplicadas. Esta visualización permite observar el comportamiento del sistema durante pruebas y demostración.

El seguimiento por salida serial se maneja desde `logger`. Allí se registran eventos generales, como la inicialización y disponibilidad del sistema, y también información de control relacionada con sensores, error, corrección, velocidades y estado de START. Esta información permite validar el comportamiento del firmware desde el monitor serial.

La comunicación remota se concentra en `wifi_monitor`. Este módulo conecta la ESP32 a la red WiFi, establece comunicación mediante MQTT, publica telemetría del robot y recibe parámetros enviados desde la página web. La información recibida desde la interfaz permite actualizar valores como `KP`, `KI`, `KD` y velocidad base durante la ejecución, sin necesidad de recompilar el firmware.

### 3.4 Justificación técnica de la arquitectura

La arquitectura modular fue seleccionada porque el proyecto requiere un firmware claro, mantenible y trazable. Separar las funcionalidades en componentes permite identificar con mayor facilidad qué archivo implementa cada parte del sistema y cómo se relaciona cada módulo con los requisitos funcionales y no funcionales del proyecto.

La separación entre calibración, lectura de sensores, control de línea y actuación sobre motores permite que el flujo de datos sea claro. Los sensores generan la información de entrada, la calibración permite normalizar los valores, el controlador calcula una corrección a partir del error y el driver de motores convierte esa corrección en movimiento físico. Esta división facilita la depuración porque cada bloque puede revisarse de forma independiente.

Centralizar la configuración en `robot_config` permite que los parámetros del sistema estén agrupados en un único lugar. Esto es importante en un robot seguidor de línea porque valores como ganancias PID, velocidad base, límites de velocidad, nivel lógico de detección, periodo de control y parámetros de comunicación pueden requerir ajustes durante las pruebas físicas.

El uso de una entrada física START mejora la seguridad operacional del sistema. El robot no inicia movimiento únicamente por estar energizado, sino que necesita una señal de habilitación. A su vez, la integración de una pantalla OLED mediante I2C permite cumplir con una función de interfaz local, mostrando información relevante sin depender exclusivamente del monitor serial. Esto facilita la demostración del sistema y permite observar estados básicos del robot directamente en el montaje físico.

La incorporación de `wifi_monitor` amplía la capacidad de supervisión y ajuste del robot. Gracias a este bloque, el sistema puede enviar telemetría hacia una página web y recibir parámetros de control durante la ejecución. Esto permite ajustar el comportamiento del robot de forma más flexible durante las pruebas, sin modificar el código fuente ni recompilar cada vez que se cambian valores como `KP`, `KI`, `KD` o velocidad base.

La arquitectura también facilita cambios futuros. Si posteriormente se desea modificar la estrategia de control, el cambio se concentraría principalmente en `line_control`. Si se requiere ajustar pines o parámetros base, se modificaría `robot_config`. Si se cambia el tipo de sensor o la forma de interpretación de línea, el cambio se concentraría en `line_sensors` o `calibration`. Si se desea modificar la interfaz remota, el cambio se enfocaría principalmente en `wifi_monitor` y en la página web.

En conjunto, esta arquitectura permite que el firmware del robot mantenga una relación clara entre hardware, lógica de control, actuación, visualización, comunicación remota y evidencia de funcionamiento.

## 4. Estructura del proyecto y flujo de repositorio

### 4.1 Estructura de directorios

El proyecto está organizado bajo la estructura de PlatformIO utilizando ESP-IDF como framework de desarrollo. La organización del repositorio separa el archivo principal de aplicación, los componentes funcionales del firmware, la interfaz web, la documentación técnica y los archivos de configuración necesarios para la compilación.

La estructura general del firmware es la siguiente:

carro_seguidor_2/
├── components/
│   ├── calibration/
│   │   ├── qtr_calibration.c
│   │   ├── include/
│   │   │   └── qtr_calibration.h
│   │   └── CMakeLists.txt
│   ├── config/
│   │   ├── robot_config.c
│   │   ├── include/
│   │   │   └── robot_config.h
│   │   └── CMakeLists.txt
│   ├── control/
│   │   ├── line_control.c
│   │   ├── include/
│   │   │   └── line_control.h
│   │   └── CMakeLists.txt
│   ├── line_sensors/
│   │   ├── line_sensors.c
│   │   ├── include/
│   │   │   └── line_sensors.h
│   │   └── CMakeLists.txt
│   ├── logger/
│   │   ├── logger.c
│   │   ├── include/
│   │   │   └── logger.h
│   │   └── CMakeLists.txt
│   ├── motor_driver/
│   │   ├── motor_driver.c
│   │   ├── include/
│   │   │   └── motor_driver.h
│   │   └── CMakeLists.txt
│   ├── oled_display/
│   │   ├── oled_display.c
│   │   ├── include/
│   │   │   └── oled_display.h
│   │   └── CMakeLists.txt
│   ├── start_input/
│   │   ├── start_input.c
│   │   ├── include/
│   │   │   └── start_input.h
│   │   └── CMakeLists.txt
│   └── wifi_monitor/
│       ├── wifi_monitor.c
│       ├── include/
│       │   └── wifi_monitor.h
│       └── CMakeLists.txt
├── src/
│   ├── main.c
│   └── CMakeLists.txt
├── include/
├── lib/
├── test/
├── CMakeLists.txt
├── platformio.ini
├── sdkconfig.az-delivery-devkit-v4
└── sdkconfig.defaults
Web/
└── index.html
docs/
├── embedded_firmware_design.md
└── images/

La carpeta src/ contiene el archivo principal del firmware, main.c, donde se encuentra la función app_main(). Este archivo actúa como punto de entrada de la aplicación y coordina la inicialización, calibración, operación general, control del robot, actualización de interfaz y comunicación remota.

La carpeta components/ contiene los módulos funcionales del firmware y cada uno agrupa una responsabilidad específica del sistema. Esta organización permite mantener el código separado por funciones y facilita la trazabilidad entre requisitos e implementación.

Dentro de components/, la carpeta calibration contiene la lógica asociada a la calibración de sensores QTR. config centraliza los parámetros del sistema, control implementa el controlador de línea, line_sensors procesa las lecturas de sensores, motor_driver gestiona la actuación sobre motores, start_input lee la señal física de inicio, oled_display controla la pantalla OLED, logger genera mensajes de depuración y wifi_monitor gestiona la comunicación WiFi/MQTT con la interfaz web.

La carpeta Web/ contiene el archivo index.html, correspondiente a la interfaz web del sistema. Esta página permite visualizar telemetría del robot y modificar parámetros de control durante la ejecución, de acuerdo con la comunicación establecida desde la ESP32 mediante WiFi/MQTT.

La carpeta docs/ contiene la documentación técnica del proyecto. En ella se ubica el Embedded Firmware Design Document y las imágenes asociadas a los diagramas del sistema, arquitectura de firmware, flujo de operación y estados del firmware.

El archivo platformio.ini define la configuración del entorno de desarrollo, incluyendo la tarjeta utilizada, la plataforma de compilación y el framework ESP-IDF. Los archivos CMakeLists.txt permiten registrar los componentes y definir las dependencias necesarias para que el proyecto compile correctamente.

### 4.2 Archivos principales del firmware

El archivo principal del firmware es src/main.c. Este archivo contiene la función app_main(), que corresponde al punto de entrada de una aplicación desarrollada en ESP-IDF. Desde allí se inicializan los módulos del sistema, se ejecuta la calibración de sensores, se entra al ciclo principal y se toman las decisiones de operación del robot.

main.c coordina la lectura de la señal física START, la lectura de sensores de línea, la ejecución del control de seguimiento, la actualización de motores, la actualización de la pantalla OLED, la generación de mensajes de depuración y la comunicación con la interfaz web. También contiene la lógica de parada segura cuando el robot no debe moverse o cuando se presenta una condición no válida de operación.

En config se encuentran los archivos robot_config.c y robot_config.h. Estos archivos centralizan los parámetros del sistema, como pines, número de sensores, dirección I2C de la pantalla OLED, límites de velocidad, velocidad base, constantes iniciales del controlador, configuración de comunicación WiFi/MQTT y otros valores necesarios para la operación del robot.

La carpeta calibration contiene los archivos asociados a la calibración de los sensores QTR. Esta parte del firmware toma muestras de referencia, ajusta los valores mínimos y máximos esperados y permite mejorar la interpretación de los sensores antes de calcular posición y error.

En line_sensors se encuentran los archivos line_sensors.c y line_sensors.h. Allí se realiza la lectura de los sensores infrarrojos y el procesamiento de los valores normalizados para calcular la posición de la línea, el error de seguimiento y la condición de detección de línea.

Los archivos line_control.c y line_control.h, ubicados en control, implementan el controlador de línea basado en PID. Su función es calcular una corrección a partir del error de seguimiento entregado por el módulo de sensores.

motor_driver contiene los archivos motor_driver.c y motor_driver.h. Esta parte controla el driver TB6612FNG mediante señales de dirección y PWM, permitiendo modificar la velocidad de los motores izquierdo y derecho.

Los archivos start_input.c y start_input.h gestionan la lectura de la entrada física de inicio. Esta señal se utiliza como habilitación de movimiento y evita que el robot avance sin autorización física de operación.

oled_display agrupa los archivos responsables de la configuración y funcionamiento de la pantalla OLED mediante comunicación I2C. Esta interfaz permite mostrar información del estado del sistema durante calibración, espera, seguimiento y parada segura.

Desde logger se generan mensajes de sistema y mensajes de control mediante salida serial, facilitando la depuración y la obtención de información durante pruebas.

La comunicación remota se implementa en wifi_monitor, mediante los archivos wifi_monitor.c y wifi_monitor.h. Allí se gestiona la conexión WiFi, la comunicación MQTT, el envío de telemetría hacia la página web, la recepción de parámetros de control y la persistencia de algunos valores en memoria no volátil.

Finalmente, Web/index.html corresponde a la interfaz web del proyecto. Esta página permite visualizar el estado del robot y modificar parámetros como KP, KI, KD y velocidad base durante la ejecución del sistema.

### 4.3 Dependencias y configuración de compilación

El proyecto utiliza ESP-IDF como framework principal y PlatformIO como entorno de construcción. Cada componente del firmware posee un archivo CMakeLists.txt donde se registran los archivos fuente, las carpetas de inclusión y las dependencias necesarias.

El archivo .c contiene la implementación del componente, mientras que el archivo .h define su interfaz pública. El archivo CMakeLists.txt permite que ESP-IDF reconozca el componente dentro del proceso de compilación.

Los módulos que interactúan con periféricos de la ESP32 requieren declarar dependencias específicas. Por ejemplo, el módulo motor_driver requiere los drivers de GPIO y PWM; el módulo oled_display requiere el driver de I2C; y el módulo start_input requiere el driver de GPIO; y wifi_monitor requiere dependencias asociadas a WiFi, eventos, red, MQTT, FreeRTOS y memoria no volátil.

La correcta declaración de dependencias es necesaria para evitar errores de compilación asociados a archivos de cabecera como driver/gpio.h, driver/ledc.h, driver/i2c_master.h, esp_wifi.h, mqtt_client.h o nvs_flash.h. Por esta razón, cada componente debe declarar explícitamente los drivers o servicios de ESP-IDF que utiliza.

El archivo platformio.ini define la tarjeta utilizada, la plataforma espressif32 y el framework espidf. La compilación del proyecto se realiza desde la terminal de PlatformIO mediante el comando: pio run

Cuando se modifican archivos CMakeLists.txt, dependencias de componentes o estructura de carpetas, se recomienda limpiar la compilación antes de reconstruir el proyecto:
- pio run -t clean
- pio run

### 4.4 Flujo de trabajo en GitHub

El repositorio de GitHub se utiliza como medio principal de entrega y control de versiones del proyecto. El desarrollo se realiza de forma local en PlatformIO. Una vez implementado un cambio, se debe compilar el proyecto para verificar que no existan errores. Posteriormente, el cambio debe registrarse mediante un commit descriptivo y subirse al repositorio remoto.

## 5. Funcionamiento del firmware

### 5.1 Inicialización del sistema

El funcionamiento del firmware inicia en la función `app_main()`, ubicada en el archivo `src/main.c`. Esta función corresponde al punto de entrada principal de una aplicación desarrollada en ESP-IDF. Desde allí se inicializan los módulos necesarios para la operación del robot y se prepara el sistema para entrar al ciclo principal de control.

Durante la inicialización se cargan los parámetros definidos en `robot_config`. Posteriormente, se inicializan los módulos principales del sistema, tales como `logger`, `oled_display`, `wifi_monitor`, `line_sensors`, `start_input` y `motor_driver`. Después de esta etapa, el robot queda energizado y preparado para operar, pero no inicia movimiento automáticamente.

El firmware también crea una tarea independiente para la pantalla OLED. Esta tarea permite actualizar la interfaz visual sin bloquear el ciclo principal de control. De esta manera, la pantalla puede mostrar información de calibración, espera o funcionamiento mientras el control del robot mantiene su propio periodo de ejecución.

Después de inicializar los módulos, el sistema carga los parámetros de control almacenados mediante el módulo `wifi_monitor`. Estos valores incluyen `KP`, `KI`, `KD` y velocidad base. Si no existen parámetros guardados previamente, el sistema utiliza los valores definidos inicialmente en `robot_config`.

Antes de iniciar el seguimiento de línea, el firmware ejecuta una fase de calibración de sensores QTR. Durante esta etapa, se toman muestras de los sensores, se actualizan valores de referencia y se normalizan las lecturas. Esta calibración permite que el sistema interprete de mejor manera el contraste entre línea y fondo antes de calcular posición y error.

Una vez finalizada la calibración, el robot queda en espera de la señal física `START`. Esta condición evita que el sistema se desplace inmediatamente después del encendido. El movimiento solo inicia cuando el firmware detecta una activación válida de START.

### 5.2 Lectura y procesamiento de sensores

El sistema utiliza un arreglo de sensores QTR-8RC para detectar la presencia de una línea sobre la pista. Estos sensores se leen desde el firmware mediante tiempos de descarga, los cuales posteriormente se normalizan utilizando los valores obtenidos durante la calibración.

En cada iteración del ciclo principal, el firmware lee los sensores en estado crudo, normaliza sus valores y entrega esta información al módulo `line_sensors`. A partir de estos datos, se determina cuáles sensores detectan la línea y si existe una detección válida para continuar con el seguimiento.

Cuando la línea es detectada, el módulo calcula una posición estimada de la línea en relación con el arreglo de sensores. Esta posición permite saber si la línea se encuentra centrada, desplazada hacia la izquierda o desplazada hacia la derecha. La posición calculada se almacena dentro de una estructura de datos que también contiene el estado individual de los sensores, el error de seguimiento y la bandera `line_detected`.

El uso de una estructura de datos para sensores permite concentrar toda la información de entrada en una sola variable lógica, facilitando que el archivo principal `main.c` pueda tomar decisiones de operación sin depender directamente de los detalles internos de lectura de cada sensor.

Si no se detecta línea, el firmware no calcula una acción de control normal. En este caso, el sistema entra en una lógica de recuperación temporal o, si la pérdida de línea se mantiene por más tiempo del permitido, ejecuta una parada segura.

### 5.3 Cálculo de posición y error

Una vez obtenidas y normalizadas las lecturas de los sensores, el firmware calcula una posición estimada de la línea. Esta posición se determina a partir de los sensores activos y de la ubicación relativa de cada sensor dentro del arreglo.

El valor de posición se interpreta dentro de un rango en el que los extremos representan la detección de la línea hacia los lados del robot y el valor central representa una línea alineada con el centro del chasis. En la configuración actual, el centro de referencia se define mediante el parámetro `LINE_CENTER_POSITION`.

El error de seguimiento se obtiene comparando la posición calculada de la línea con el centro esperado. Si la línea se encuentra centrada, el error tiende a cero. Si la línea se desplaza hacia un lado, el error adquiere un valor positivo o negativo, dependiendo de la dirección del desplazamiento.

Este error es la variable principal de entrada para el controlador de línea. Su función es representar cuánto y hacia dónde debe corregir el robot su trayectoria. Un error pequeño implica una corrección leve, mientras que un error mayor implica una diferencia más significativa entre la velocidad del motor izquierdo y la del motor derecho.

Cuando la línea se pierde temporalmente, el firmware conserva durante un tiempo limitado una estrategia de recuperación basada en el error disponible. Esta lógica permite que el robot intente recuperar la trayectoria antes de detenerse completamente. Si la línea no se detecta nuevamente dentro del tiempo configurado, el sistema ejecuta una parada segura.

### 5.4 Control PID y actuación sobre motores

El control de trayectoria se realiza mediante el módulo `line_control`, el cual implementa una estrategia basada en PID. Este controlador recibe como entrada el error de seguimiento calculado por el módulo de sensores y genera una corrección de control.

La acción proporcional responde directamente al error actual. Si el robot se encuentra desviado de la línea, esta componente genera una corrección proporcional a la magnitud de dicha desviación. La acción integral acumula el error a lo largo del tiempo, permitiendo compensar desviaciones persistentes. La acción derivativa responde a la variación del error, ayudando a suavizar la respuesta ante cambios rápidos en la posición de la línea.

La salida del controlador se limita dentro de un rango definido en la configuración del sistema. Esto evita que la corrección produzca velocidades excesivas o valores que no puedan ser aplicados físicamente por los motores.

La corrección obtenida se combina con una velocidad base. A partir de esta combinación se calculan dos velocidades: una para el motor izquierdo y otra para el motor derecho. Si el robot necesita girar hacia un lado, una de las velocidades aumenta y la otra disminuye, generando un movimiento diferencial.

El módulo `motor_driver` recibe estas velocidades y las convierte en señales físicas hacia el driver TB6612FNG. Este configura la dirección de giro y el ciclo de trabajo PWM correspondiente a cada motor. De esta forma, el firmware transforma la información de sensores en una acción física que corrige la trayectoria del robot.

Cuando el firmware determina que el robot no debe moverse, se ejecuta una función de parada segura. Esta función envía velocidades nulas al driver de motores y reinicia el controlador, garantizando que el robot permanezca detenido hasta que existan condiciones válidas de operación.

### 5.5 Comunicación, monitoreo y actualización de parámetros

Durante el ciclo de operación, el firmware no solo controla sensores y motores, sino que también actualiza la información visible para el usuario. La pantalla OLED muestra estados de calibración, espera y operación, mientras que el logger serial registra información útil para depuración.

La comunicación inalámbrica se gestiona desde `wifi_monitor`. Este bloque permite publicar telemetría del robot mediante MQTT y recibir parámetros enviados desde la página web. La telemetría incluye variables como error, posición, velocidad izquierda, velocidad derecha, corrección, estado de línea, estado de START, estado de motores y valores de sensores.

Cuando la página web envía nuevos valores de `KP`, `KI`, `KD` o velocidad base, el firmware detecta el cambio durante el ciclo principal. En ese momento, actualiza los parámetros del controlador y continúa operando con la nueva configuración. Esto permite ajustar el comportamiento del robot durante las pruebas sin modificar el código fuente ni recompilar el proyecto.

Los parámetros recibidos por comunicación se conservan mediante memoria no volátil. Gracias a esto, los ajustes realizados desde la interfaz web pueden mantenerse después de reiniciar la ESP32, siempre que hayan sido guardados correctamente por el módulo correspondiente.

### 5.6 Temporización y modelo de ejecución

El firmware opera mediante un ciclo principal temporizado. Después de la inicialización y calibración, `main.c` ejecuta de forma repetitiva la lectura de entradas, el procesamiento de sensores, la evaluación de seguridad, el cálculo de control, la actuación sobre motores, la actualización de datos para pantalla, el envío de telemetría y la generación de mensajes de depuración.

El periodo de ejecución del ciclo se define mediante un parámetro de configuración. En la implementación actual, el ciclo utiliza una espera basada en FreeRTOS mediante `vTaskDelayUntil`, lo cual permite controlar la frecuencia aproximada de actualización del sistema de forma más estable que una espera simple.

Aunque el control principal se mantiene dentro de un ciclo temporizado, algunas funciones auxiliares se ejecutan mediante tareas separadas. La pantalla OLED se actualiza en una tarea de baja prioridad para evitar que su refresco bloquee el control del robot. La telemetría también se publica desde una tarea asociada al módulo WiFi/MQTT, con una frecuencia definida para el envío periódico de datos.

El modelo actual evalúa primero las condiciones necesarias para permitir el movimiento. Si la señal `START` no está activa, el robot permanece detenido. Si la señal `START` está activa y la línea es detectada, se ejecuta el cálculo de control y se actualizan las velocidades de los motores. Si la línea se pierde, el sistema intenta recuperarla durante un tiempo limitado y, si no lo logra, entra en parada segura.

Este modelo de ejecución permite que el robot responda de forma periódica a los cambios detectados en la pista, manteniendo una lógica clara de operación, seguridad, visualización y comunicación remota.

## 6. Diagrama de estados del firmware

### 6.1 Estados principales

El firmware del robot puede representarse mediante una máquina de estados lógica que describe las condiciones principales de operación del sistema. Aunque la implementación se ejecuta dentro de un ciclo principal en `src/main.c`, el comportamiento del robot puede organizarse en estados funcionales para facilitar su comprensión, validación y trazabilidad.

El primer estado corresponde a la inicialización del sistema. En este estado se configuran los módulos principales del firmware, como logger, pantalla OLED, comunicación WiFi/MQTT, sensores de línea, entrada física START y driver de motores. También se preparan las estructuras necesarias para el controlador, la calibración y el manejo de datos del sistema.

Después de la inicialización, el firmware ejecuta una fase de calibración de sensores. Durante esta etapa, el robot toma muestras del arreglo QTR, actualiza valores de referencia y normaliza las lecturas. Esta fase permite que el sistema interprete con mayor precisión el contraste entre la línea y el fondo antes de iniciar el recorrido.

Una vez finalizada la calibración, el robot entra en un estado de espera segura. En este estado los motores permanecen detenidos mientras la señal física START no se encuentre activa. Esta condición evita que el robot inicie movimiento únicamente por estar energizado.

Cuando la señal START está activa, el firmware evalúa la lectura de los sensores de línea. Si la línea es detectada correctamente, el sistema entra en estado de seguimiento activo. En este estado se calcula la posición de línea, el error de seguimiento, la corrección PID y las velocidades diferenciales para los motores.

Si durante la operación la línea deja de detectarse, el sistema no pasa inmediatamente a una parada definitiva. Primero entra en una condición de recuperación temporal, en la cual intenta conservar el control durante un tiempo limitado utilizando la información disponible del error. Si la línea se recupera dentro del tiempo permitido, el robot continúa el seguimiento. Si la pérdida se mantiene más allá del límite configurado, el firmware detiene los motores y reinicia el controlador.

También existe una condición de parada por desactivación de START. Si la señal física de inicio se desactiva durante la operación, el firmware detiene inmediatamente los motores, reinicia el controlador y retorna a una condición segura.

Los estados principales del firmware son:

- INIT
- CALIBRATION
- WAIT_START
- RUNNING
- LINE_RECOVERY
- SAFE_STOP

El estado INIT representa la inicialización del sistema. CALIBRATION corresponde a la toma de muestras y normalización inicial de sensores. WAIT_START representa la espera de la señal física de inicio. RUNNING describe la operación normal de seguimiento de línea. LINE_RECOVERY representa el intento temporal de recuperar la trayectoria cuando la línea se pierde, y SAFE_STOP corresponde a la condición en la que los motores se mantienen detenidos por seguridad.

### 6.2 Transiciones y condiciones

La transición inicial ocurre desde el encendido o reinicio del microcontrolador hacia el estado INIT. Durante esta etapa se inicializan los módulos principales del firmware y se preparan las variables necesarias para la operación.

Una vez completada la inicialización, el sistema pasa al estado CALIBRATION. En esta fase se leen los sensores QTR durante un número definido de muestras, se actualizan los valores de referencia y se normalizan las lecturas. Cuando la calibración termina, el firmware registra que el sistema está listo y pasa a WAIT_START.

Desde WAIT_START, el robot permanece detenido mientras la señal física START no esté activa. Cuando el firmware detecta una activación válida de START, se habilita la evaluación de sensores para iniciar el recorrido. Si la línea es detectada, el sistema pasa al estado RUNNING.

Durante RUNNING, el robot ejecuta el ciclo de seguimiento de línea. En este estado se leen los sensores, se calcula la posición de la línea, se obtiene el error, se ejecuta el controlador PID y se actualizan las velocidades de los motores. También se actualiza la pantalla OLED, se registra información por salida serial y se envía telemetría hacia la interfaz web.

Si en RUNNING la línea deja de detectarse, el sistema pasa al estado LINE_RECOVERY. Esta condición ocurre cuando los sensores no entregan una lectura válida de la trayectoria. Durante este estado, el firmware intenta mantener una acción de recuperación durante un tiempo limitado. La duración máxima de esta recuperación se define desde la configuración del sistema.

Si la línea vuelve a detectarse mientras START continúa activo, el sistema regresa a RUNNING. Si la línea no se recupera dentro del tiempo permitido, el firmware pasa a SAFE_STOP. En este estado se detienen los motores, se reinicia el controlador y se mantiene el sistema en condición segura.

Si en cualquier momento la señal START se desactiva, el sistema pasa directamente a SAFE_STOP o retorna a WAIT_START, manteniendo los motores detenidos. Esta transición tiene prioridad sobre la operación normal porque START actúa como una habilitación física de movimiento.

### 6.3 Diagrama de estados

El diagrama de estados del firmware representa las condiciones principales de operación y las transiciones entre ellas. Este diagrama permite visualizar cómo el sistema pasa desde la inicialización hasta la operación normal, y cómo responde ante condiciones de espera, calibración, pérdida temporal de línea o parada segura.

Espacio para imagen del diagrama de estados del firmware:
![Diagrama de Estados](DocsImages/EstadosFirmware.png)

## 7. Manejo de errores y seguridad

### 7.1 Condiciones de error detectadas

El firmware incorpora una lógica de seguridad orientada a evitar que el robot continúe en movimiento cuando no existen condiciones confiables de operación. Esta lógica se encuentra principalmente coordinada desde `src/main.c`, aunque depende de la información entregada por los módulos `start_input`, `line_sensors`, `qtr_calibration`, `motor_driver`, `line_control`, `oled_display`, `logger` y `wifi_monitor`.

La primera condición de seguridad corresponde a la señal física `START`. Esta entrada actúa como habilitación general de movimiento. Si la señal `START` no está activa, el firmware mantiene los motores detenidos, independientemente de que los sensores detecten la línea o de que el resto de módulos se encuentren inicializados correctamente.

La segunda condición de seguridad corresponde a la pérdida de línea. Si los sensores no detectan una trayectoria válida, el firmware no debe continuar ejecutando el seguimiento normal. En la implementación actual, antes de detenerse completamente, el sistema intenta una recuperación temporal durante un número limitado de ciclos. Esta recuperación permite que el robot intente reencontrar la línea cuando la pérdida es breve. Si la línea no se recupera dentro del tiempo configurado, el sistema ejecuta una parada segura.

Otra condición importante está asociada con la calibración de sensores. Si la calibración no se ejecuta correctamente, las lecturas normalizadas pueden ser poco confiables y afectar el cálculo de posición y error. Por esta razón, la fase de calibración debe completarse antes de permitir el inicio del recorrido.

También se contempla como condición anómala la lectura no confiable de sensores. Esto puede ocurrir si los valores normalizados no permiten identificar una línea válida, si todos los sensores permanecen fuera del rango esperado o si la posición calculada no representa una trayectoria útil para el controlador. En estos casos, el sistema debe priorizar la seguridad sobre la continuidad del movimiento.

Las fallas durante la inicialización de módulos también deben considerarse dentro del manejo de errores. Si no se inicializan correctamente elementos como el driver de motores, los sensores o la entrada START, el robot no debería continuar hacia una operación normal. Cuando la falla se relaciona con interfaces de visualización o comunicación, como OLED o WiFi/MQTT, el comportamiento puede depender de la criticidad del módulo afectado.

Una falla en la pantalla OLED afecta la visualización local del sistema, pero no necesariamente impide el seguimiento de línea. De manera similar, una desconexión WiFi o MQTT afecta el monitoreo remoto y la actualización de parámetros desde la página web, pero no debería detener automáticamente el robot si las condiciones físicas de operación siguen siendo válidas. En estos casos, la condición debe registrarse como advertencia para diagnóstico.

En la implementación actual, la seguridad se maneja mediante validaciones dentro del ciclo principal, recuperación temporal ante pérdida de línea y una función de parada segura. Esta estrategia permite detener los motores ante condiciones no válidas y mantener el robot en un estado controlado.

### 7.2 Códigos de error y advertencia

Para efectos de documentación, trazabilidad y pruebas, las condiciones de error y advertencia del firmware se identifican mediante códigos estructurados. Estos códigos permiten relacionar cada condición anómala con una causa, una acción correctiva y una evidencia observable durante la validación.

Los códigos propuestos para el sistema son los siguientes:

- SYS_INIT_OK
- SYS_READY
- CALIBRATION_START
- CALIBRATION_DONE
- WARN_START_INACTIVE
- WARN_LINE_LOST
- WARN_LINE_RECOVERY
- ERR_LINE_LOST_TIMEOUT
- ERR_SENSOR_READ
- ERR_LINE_POSITION_INVALID
- ERR_CALIBRATION_INVALID
- ERR_MOTOR_DRIVER_INIT
- ERR_OLED_INIT
- WARN_WIFI_DISCONNECTED
- WARN_MQTT_DISCONNECTED
- WARN_MQTT_PARAM_INVALID
- SAFE_STOP_ENTER
- CONTROL_RESET
- PARAM_UPDATE_OK

`SYS_INIT_OK` indica que la inicialización general del sistema fue completada correctamente. `SYS_READY` indica que el robot se encuentra listo para operar y en espera de condiciones válidas de movimiento.

`CALIBRATION_START` y `CALIBRATION_DONE` se asocian con el inicio y finalización de la fase de calibración de sensores. Estos eventos permiten verificar que el robot no inicia el recorrido sin haber preparado previamente la interpretación de lecturas.

`WARN_START_INACTIVE` se utiliza cuando la entrada física START no está activa. Esta condición no se considera un error crítico, ya que puede corresponder al estado normal de espera antes de iniciar la prueba. Sin embargo, debe registrarse como advertencia o estado de espera porque mantiene los motores detenidos.

`WARN_LINE_LOST` se utiliza cuando los sensores no detectan una línea válida durante la operación. Esta condición indica que el robot perdió temporalmente su referencia de trayectoria. En la implementación actual, esta advertencia puede ir acompañada de una fase de recuperación antes de detener completamente los motores.

`WARN_LINE_RECOVERY` indica que el firmware se encuentra intentando recuperar la línea durante el tiempo permitido. Si la línea vuelve a detectarse, el robot puede regresar al seguimiento normal. Si no se recupera dentro del límite configurado, se genera `ERR_LINE_LOST_TIMEOUT` y se ejecuta la parada segura.

`ERR_SENSOR_READ` se asocia con errores en la lectura de sensores. Puede utilizarse cuando el módulo de sensores no logra entregar datos válidos al ciclo principal. `ERR_LINE_POSITION_INVALID` se relaciona con una posición de línea inválida o incoherente, mientras que `ERR_CALIBRATION_INVALID` corresponde a una calibración incompleta, fallida o insuficiente para normalizar correctamente las lecturas.

`ERR_MOTOR_DRIVER_INIT` indica una falla durante la inicialización del driver de motores. Esta condición es crítica porque el sistema no puede garantizar una actuación segura. `ERR_OLED_INIT` indica una falla durante la inicialización de la pantalla OLED. Esta condición afecta la interfaz local, pero no necesariamente impide el movimiento del robot.

`WARN_WIFI_DISCONNECTED` y `WARN_MQTT_DISCONNECTED` se utilizan cuando se pierde la comunicación inalámbrica o la conexión con el broker MQTT. Estas condiciones afectan la telemetría y la configuración remota desde la página web, pero no son necesariamente críticas para el control físico del robot. `WARN_MQTT_PARAM_INVALID` puede utilizarse cuando se recibe un parámetro no válido o fuera del rango esperado desde la interfaz web.

`PARAM_UPDATE_OK` indica que los parámetros enviados desde la página web fueron recibidos y aplicados correctamente. `SAFE_STOP_ENTER` registra que el sistema ejecutó una parada segura, y `CONTROL_RESET` indica que el controlador de línea fue reiniciado para evitar acumulaciones o respuestas incorrectas después de una condición de seguridad.

### 7.3 Acciones correctivas

Las acciones correctivas del firmware buscan llevar el sistema a una condición segura cuando se detecta una situación anómala. La acción principal es la detención de ambos motores mediante `motor_driver`.

Cuando la señal START no está activa, el firmware mantiene los motores detenidos y no ejecuta el cálculo normal de control. En esta condición también se reinicia el controlador para evitar que el error anterior o el término integral afecten el comportamiento cuando el robot vuelva a iniciar.

Cuando la línea no es detectada, el sistema entra primero en una fase de recuperación temporal. Durante esta etapa, el firmware intenta mantener una acción de corrección limitada para recuperar la trayectoria. Si la línea vuelve a detectarse, el sistema retorna al seguimiento normal. Si la pérdida se mantiene más allá del tiempo configurado, se detienen los motores y se reinicia el controlador.

Si ocurre una falla asociada con lectura inválida de sensores o posición incoherente, el firmware debe evitar calcular una corrección PID con datos no confiables. En este caso, la acción correctiva consiste en detener los motores, registrar el evento y esperar una nueva condición válida.

Ante una falla crítica del driver de motores, el sistema no debe continuar la operación normal porque no puede asegurar la actuación física. En este caso, la respuesta esperada es impedir el movimiento, registrar el error y mantener el sistema en un estado seguro hasta que se corrija la condición.

Si se detecta un problema de OLED, WiFi o MQTT, la acción correctiva principal es registrar la advertencia y continuar la operación física del robot si las condiciones de START, sensores y motores son válidas. Estas interfaces apoyan la visualización, monitoreo y configuración, pero no deben detener automáticamente el robot a menos que la falla comprometa directamente la seguridad del sistema.

Cuando se reciben parámetros desde la página web, el firmware debe aplicarlos únicamente si son válidos para el rango de operación del robot. Si un valor recibido no es adecuado, debe descartarse o limitarse para evitar una respuesta inestable del controlador. Esta validación es importante porque parámetros PID o velocidades mal configuradas pueden producir oscilaciones, movimientos bruscos o pérdida de línea.

Finalmente, cuando se ejecuta una parada segura, el controlador se reinicia. Esta acción evita que el término integral o el error anterior del PID generen una corrección acumulada cuando el robot vuelva a detectar la línea o cuando START vuelva a estar activo.

### 7.4 Modo seguro

El modo seguro corresponde a la condición en la cual el robot mantiene sus motores detenidos para evitar movimiento no controlado. Este modo puede activarse por START inactivo, pérdida de línea sostenida, lectura inválida de sensores, calibración no confiable o fallas críticas durante la inicialización.

En modo seguro, el firmware no ejecuta la actuación normal sobre los motores. En su lugar, envía una orden de parada al módulo `motor_driver`, reinicia el controlador de línea y actualiza la información de estado mediante OLED, salida serial y telemetría cuando la comunicación WiFi/MQTT está disponible. Esto permite que el usuario observe que el robot se encuentra detenido por una condición de seguridad.

El modo seguro no representa necesariamente una falla permanente. Por ejemplo, si el robot está en espera porque START está inactivo, puede pasar a operación normal cuando la señal se active y los sensores detecten línea. De igual forma, si la línea se pierde temporalmente, el robot puede recuperarla dentro del tiempo permitido y continuar el recorrido.

La prioridad del modo seguro es proteger el sistema físico y evitar comportamientos impredecibles. Este modo también facilita la validación del sistema mediante pruebas. Durante los test cases, se puede verificar que los motores se detienen cuando START está inactivo, cuando la línea no se recupera dentro del tiempo configurado o cuando se simula una condición inválida.

Las fallas de comunicación remota no activan necesariamente el modo seguro. Si WiFi o MQTT se desconectan, el robot puede continuar operando siempre que las condiciones físicas de seguimiento sean válidas. No obstante, el sistema pierde temporalmente la capacidad de enviar telemetría y recibir ajustes desde la interfaz web, por lo que la condición debe registrarse para diagnóstico y posterior corrección.

## 8. Logging y comunicaciones

### 8.1 Estrategia de logging

El firmware incorpora una estrategia de logging orientada a registrar información relevante del sistema durante la ejecución. En la implementación actual, el registro de eventos y datos de depuración se realiza mediante el módulo `logger`, utilizando salida serial generada por el firmware.

El propósito del logging es permitir la observación del comportamiento interno del robot durante pruebas, integración y demostración. A través de estos mensajes es posible verificar si el sistema fue inicializado correctamente, si la señal START está activa, si la línea está siendo detectada, cuál es la posición calculada, cuál es el error de seguimiento, qué corrección entrega el controlador y qué velocidades se aplican a los motores.

El logging también permite obtener información útil durante la validación del sistema. Los mensajes generados por salida serial pueden capturarse desde el monitor serial de PlatformIO y utilizarse como apoyo para revisar el comportamiento del robot durante pruebas de inicialización, sensores, control PID, actuación de motores, calibración y condiciones de seguridad.

En la arquitectura actual, `logger` funciona como una herramienta de diagnóstico y trazabilidad. Aunque no reemplaza las pruebas físicas del robot ni la telemetría enviada a la página web, sí permite observar variables internas directamente desde el entorno de desarrollo.

### 8.2 Formato de mensajes

El formato general para los mensajes de logging es el siguiente:
[TIME_MS] [LEVEL] [CODE] MESSAGE. Donde TIME_MS corresponde al tiempo de ejecución del sistema en milisegundos, LEVEL indica la severidad del evento, CODE identifica el tipo de evento o condición registrada, y MESSAGE contiene una descripción legible para el usuario o desarrollador.

Los niveles de severidad utilizados son:
- INFO
- WARN
- ERROR
- DEBUG

El nivel INFO se utiliza para eventos normales del sistema, como inicialización correcta, robot listo o inicio del ciclo de operación. El nivel WARN se utiliza para condiciones anómalas no necesariamente críticas, como START inactivo o línea no detectada. El nivel ERROR se reserva para fallas que impiden el funcionamiento normal, como errores de inicialización de módulos o fallas críticas de periféricos. El nivel DEBUG se utiliza para imprimir información detallada del ciclo de control, como lecturas de sensores, error, corrección y velocidades.

Ejemplos de mensajes esperados:
[0000120] [INFO] [SYS_INIT_OK] System initialized successfully
[0000450] [INFO] [SYS_READY] Robot ready, waiting for START
[0000800] [WARN] [WARN_START_INACTIVE] Start signal inactive, motors stopped
[0001200] [WARN] [WARN_LINE_LOST] Line not detected, entering safe stop
[0001500] [DEBUG] [CONTROL_DATA] pos=3500 err=0 corr=0 left=120 right=120
[0001800] [ERROR] [ERR_OLED_INIT] OLED initialization failed

### 8.3 Comunicación serial

La salida serial se utiliza como medio para visualizar mensajes de logging y depuración durante la ejecución del firmware. Esta comunicación permite observar desde el monitor serial el estado interno del robot mientras el programa se ejecuta en la ESP32.

La comunicación serial no se utiliza como canal principal de configuración en la versión actual del sistema. Su función principal es apoyar la depuración, el seguimiento de eventos y la revisión de variables internas durante las pruebas.

A través de la salida serial se puede observar información como:
- Estado de inicialización
- Estado de calibración
- Estado de START
- Detección de línea
- Lecturas de sensores
- Posición calculada
- Error de seguimiento
- Corrección del controlador
- Velocidad del motor izquierdo
- Velocidad del motor derecho
- Condiciones de recuperación o parada segura
- Actualización de parámetros recibidos desde la interfaz web

Permitiendo verificar el comportamiento del firmware incluso cuando la pantalla OLED o la comunicación WiFi/MQTT no están disponibles. 

### 8.4 Comunicación I2C para pantalla OLED

El segundo protocolo de comunicación implementado en el sistema es I2C, utilizado para la pantalla OLED. Esta pantalla funciona como interfaz visual local y permite mostrar información relevante del estado del robot sin depender exclusivamente del monitor serial.

La pantalla OLED se gestiona mediante el módulo oled_display. Este componente inicializa la comunicación I2C y actualiza la información visible durante la operación del sistema. La comunicación I2C permite conectar la pantalla a la ESP32 utilizando únicamente dos líneas principales: SDA y SCL.

El uso de I2C se justifica porque permite conectar periféricos de visualización con bajo consumo de pines, lo cual es conveniente en un robot que también requiere múltiples entradas digitales para sensores, señales de control para motores y entrada física START.

La pantalla OLED permite mostrar información como:
- Estado del robot
- START activo o inactivo
- Línea detectada o perdida
- Posición calculada
- Error de seguimiento
- Velocidades de motores
- Mensajes de parada segura

### 8.5 Comunicación WiFi/MQTT para telemetría y configuración

Además de la comunicación local mediante OLED y salida serial, el firmware implementa comunicación inalámbrica mediante WiFi y MQTT. Esta comunicación permite conectar la ESP32 con una página web de monitoreo, enviar telemetría del robot y recibir parámetros de configuración durante la ejecución.

La comunicación WiFi/MQTT se gestiona desde wifi_monitor. Allí se realiza la conexión a la red WiFi, la comunicación con el broker MQTT, la publicación de datos del robot y la recepción de valores enviados desde la interfaz web. Los datos de conexión, broker y tópicos se definen desde robot_config.

La telemetría enviada por el robot incluye variables importantes para observar el comportamiento del sistema en tiempo real. Entre ellas se encuentran:
- Error de seguimiento
- Posición de la línea
- Velocidad del motor izquierdo
- Velocidad del motor derecho
- Corrección calculada por el controlador
- Estado de detección de línea
- Estado de START
- Estado de motores
- Valores de sensores

Esta información permite que la página web funcione como una interfaz remota de monitoreo. A diferencia de la pantalla OLED, que muestra información limitada por el tamaño físico de la pantalla, la página web puede organizar más variables de operación y presentarlas de forma más amplia.

La comunicación MQTT también permite recibir parámetros desde la interfaz web. En la implementación actual, los valores de KP, KI, KD y velocidad base pueden actualizarse durante la ejecución. Cuando el firmware detecta nuevos parámetros, actualiza la configuración del controlador y continúa operando con los valores recibidos.

Esta funcionalidad mejora el proceso de pruebas porque permite ajustar el comportamiento del robot sin modificar el código fuente, recompilar o cargar nuevamente el firmware en la ESP32. De esta forma, el equipo puede realizar ajustes de control de manera más rápida durante la etapa de calibración y validación en pista.

### 8.6 Interfaz web de usuario

La interfaz de usuario del sistema se compone de elementos locales y remotos. A nivel local, el usuario cuenta con la entrada física START y la pantalla OLED. A nivel remoto, el sistema incorpora una página web conectada mediante WiFi/MQTT.

La entrada física START permite habilitar o detener la operación del robot. Esta señal cumple una función importante dentro de la seguridad operacional, ya que evita que el robot se mueva automáticamente al encenderse. Si START está inactivo, el firmware mantiene los motores detenidos.

La pantalla OLED complementa esta interacción mostrando información del estado actual. Esto permite identificar si el robot está calibrando, esperando START, ejecutando seguimiento de línea, intentando recuperar la trayectoria o detenido por seguridad.

La página web amplía las capacidades de interacción con el sistema. Desde esta interfaz se puede visualizar telemetría del robot y modificar parámetros de control durante la ejecución. En lugar de depender únicamente de valores estáticos definidos en el código, el usuario puede ajustar parámetros como KP, KI, KD y velocidad base desde la interfaz remota.

La salida serial mediante logger funciona como una interfaz de depuración para el equipo de desarrollo. Aunque no es una interfaz gráfica, permite visualizar información detallada del funcionamiento interno del firmware y revisar eventos durante la validación.

En conjunto, START, OLED, salida serial y página web permiten cumplir funciones complementarias de interacción, visualización, diagnóstico y configuración. START controla la habilitación física del movimiento; OLED muestra el estado local del robot; la salida serial permite depurar desde PlatformIO; y la página web permite monitorear variables del sistema y ajustar parámetros de control mediante WiFi/MQTT.

## 9. Configuración de parámetros

### 9.1 Parámetros de control

La configuración de parámetros del sistema se encuentra centralizada principalmente en `robot_config`, específicamente en los archivos `robot_config.c` y `robot_config.h`. Este bloque permite agrupar los valores principales del firmware en un solo lugar, evitando que las constantes de operación queden dispersas en diferentes archivos del proyecto.

Los parámetros de control definen el comportamiento del algoritmo de seguimiento de línea. Estos valores incluyen las ganancias proporcional, integral y derivativa del controlador PID, la velocidad base del robot y los límites de operación permitidos para la corrección y los motores.

La ganancia proporcional determina qué tan fuerte responde el robot ante el error actual de seguimiento. Una ganancia proporcional mayor genera una respuesta más rápida frente a desviaciones de la línea, pero si es demasiado alta puede producir oscilaciones o movimientos bruscos.

La ganancia integral permite corregir errores acumulados a lo largo del tiempo. Esta componente puede ser útil cuando el robot presenta una desviación persistente, pero debe utilizarse con cuidado porque un valor excesivo puede generar acumulación indeseada y afectar la estabilidad del movimiento.

La ganancia derivativa responde a la variación del error. Su función es suavizar la respuesta del sistema y reducir cambios bruscos en la corrección. En un robot seguidor de línea tipo velocista, esta componente puede ayudar a mejorar la estabilidad en curvas o cambios rápidos de trayectoria.

La velocidad base define el valor nominal con el que el robot avanza cuando se encuentra siguiendo la línea. A partir de esta velocidad, el firmware suma o resta la corrección calculada por el controlador para obtener la velocidad del motor izquierdo y del motor derecho.

Además de las ganancias PID y la velocidad base, el sistema define límites mínimos y máximos para la salida del controlador y para las velocidades de los motores. Estos límites evitan que la corrección calculada genere valores fuera del rango permitido por el sistema físico.

Los parámetros de control principales son:

- `KP`
- `KI`
- `KD`
- `BASE_SPEED`
- `MIN_SPEED`
- `MAX_SPEED`
- `PID_OUTPUT_MIN`
- `PID_OUTPUT_MAX`
- `LINE_CENTER_POSITION`
- `CONTROL_PERIOD_MS`

El parámetro `LINE_CENTER_POSITION` define el valor de referencia correspondiente a la línea centrada respecto al arreglo de sensores. El error de seguimiento se calcula comparando la posición estimada de la línea con este valor central.

El parámetro `CONTROL_PERIOD_MS` define el periodo de ejecución del ciclo de control. Este valor influye en la frecuencia con la que el firmware actualiza sensores, calcula el PID, ajusta motores, actualiza datos de visualización y envía información de telemetría.

### 9.2 Parámetros de sensores, calibración y seguridad

Los parámetros de sensores definen cómo el firmware interpreta las lecturas provenientes del arreglo QTR. En la implementación actual, el sistema utiliza sensores que requieren una fase de calibración para obtener valores de referencia antes de iniciar el seguimiento de línea. El parámetro `NUM_SENSORS` define la cantidad utilizada por el sistema, este valor permite que los módulos de calibración y lectura conozcan el tamaño del arreglo y puedan procesar correctamente las lecturas individuales.

Los pines asociados a los sensores también se definen desde la configuración del sistema. Esta decisión permite modificar la conexión física de un sensor sin cambiar directamente la lógica de lectura dentro de `line_sensors`.

La calibración permite identificar rangos de lectura para cada sensor. A partir de estos valores, el firmware puede normalizar las lecturas y mejorar la interpretación del contraste entre línea y fondo. Esta etapa es importante porque las condiciones físicas de la pista, la iluminación, la distancia al suelo y el tipo de superficie pueden afectar el comportamiento de los sensores.

La seguridad del sistema también depende de ciertos parámetros de configuración. Uno de los más importantes es el nivel activo de la señal START. Este valor define si la entrada física de inicio se considera activa en nivel alto o en nivel bajo.

La señal START funciona como habilitación física de movimiento. Si esta señal no se encuentra activa, el firmware mantiene los motores detenidos. Esta condición tiene prioridad sobre la operación normal, ya que evita movimientos inesperados durante encendido, reinicio o pruebas.

La detección de línea también se considera una condición de seguridad. Si el sistema deja de detectar una línea válida, el firmware intenta recuperar la trayectoria durante un tiempo limitado. Si la línea no se recupera dentro del límite configurado, el robot entra en parada segura.

Los parámetros asociados a sensores y seguridad incluyen:
- Pines de sensores
- Parámetros de calibración QTR
- `NUM_SENSORS`
- `START_PIN`
- `START_ACTIVE_LEVEL`
- `LOST_LINE_TIMEOUT_MS`
- `LOST_LINE_MAX_CYCLES`

El parámetro `LOST_LINE_TIMEOUT_MS` define el tiempo máximo permitido para intentar recuperar la línea antes de detener el robot. 

### 9.3 Parámetros de comunicación

El firmware también contiene parámetros asociados a las comunicaciones del sistema. Estos valores permiten configurar la pantalla OLED, la salida serial de depuración y la comunicación inalámbrica mediante WiFi/MQTT.

Para la pantalla OLED se definen parámetros como los pines de comunicación I2C, la dirección del dispositivo y las condiciones necesarias para inicializar la interfaz visual local. Esta configuración permite que el robot muestre información de estado durante la calibración, espera, operación y parada segura.

La salida serial se utiliza para logging y depuración. Aunque no es el canal principal de configuración en la implementación actual, permite observar eventos, variables internas y condiciones de seguridad desde el monitor serial de PlatformIO.

La comunicación WiFi/MQTT requiere parámetros adicionales como el nombre de la red, la contraseña, la dirección del broker MQTT y los tópicos utilizados para publicar telemetría o recibir comandos. Estos valores permiten que la ESP32 se conecte con la página web y mantenga intercambio de información durante la ejecución.

Los parámetros de comunicación principales son:

- Pines I2C de la pantalla OLED
- Dirección I2C de la OLED
- Configuración de salida serial
- SSID de la red WiFi
- Contraseña de la red WiFi
- Broker MQTT
- Tópico de telemetría
- Tópico de comandos

La correcta configuración de estos parámetros es necesaria para que la pantalla OLED, el monitor serial y la página web funcionen como mecanismos complementarios de observación e interacción con el robot.

### 9.4 Parámetros modificables desde la interfaz web

En versiones anteriores del firmware, cualquier ajuste de ganancias PID o velocidad base debía realizarse directamente en el código fuente y posteriormente recompilar el proyecto. En la implementación actual, algunos parámetros pueden modificarse desde la página web mediante comunicación WiFi/MQTT.

Los parámetros modificables desde la interfaz web son:

- `KP`
- `KI`
- `KD`
- `BASE_SPEED`

Cuando el usuario modifica alguno de estos valores desde la página web, la información se envía hacia la ESP32 mediante MQTT. El módulo `wifi_monitor` recibe los nuevos parámetros y notifica al ciclo principal que existe una actualización pendiente.

Una vez detectado el cambio, `main.c` actualiza los valores utilizados por el controlador de línea. De esta forma, el robot puede modificar su comportamiento durante las pruebas físicas sin necesidad de editar el código fuente, recompilar o cargar nuevamente el firmware.

Esta funcionalidad es especialmente útil durante la calibración del robot en pista. Las ganancias PID y la velocidad base suelen requerir ajustes progresivos para lograr un seguimiento estable. Al permitir cambios desde la interfaz web, el equipo puede probar diferentes combinaciones de parámetros de forma más rápida.

Los parámetros recibidos desde la página web deben mantenerse dentro de rangos seguros. Un valor excesivo de `KP`, `KI`, `KD` o velocidad base puede producir oscilaciones, giros bruscos, pérdida de línea o inestabilidad en el movimiento. Por esta razón, la configuración remota debe tratarse como una herramienta de ajuste controlado y no como una entrada libre sin validación.

### 9.5 Persistencia de parámetros

El firmware incorpora persistencia de ciertos parámetros mediante memoria no volátil. Esta función permite conservar ajustes recibidos desde la página web incluso después de reiniciar la ESP32.

Los parámetros de control modificados remotamente pueden almacenarse en NVS. Cuando el sistema vuelve a iniciar, el firmware intenta cargar los valores guardados. Si existen parámetros almacenados correctamente, estos se utilizan como configuración inicial del controlador. Si no se encuentran valores válidos, el sistema usa los parámetros definidos por defecto en `robot_config`.

Esta estrategia combina dos niveles de configuración. Por un lado, `robot_config` conserva los valores base del sistema y permite definir una configuración inicial estable. Por otro lado, NVS permite guardar los ajustes realizados durante pruebas, evitando que se pierdan después de un reinicio.

La persistencia de parámetros mejora el proceso de validación porque permite conservar una configuración de control que haya mostrado buen desempeño en pista. También facilita comparar el comportamiento del robot entre diferentes sesiones de prueba sin tener que ingresar manualmente los mismos valores cada vez.

En conjunto, la configuración del sistema se organiza en tres niveles: parámetros estáticos definidos en `robot_config`, parámetros ajustables en tiempo real desde la página web y parámetros persistidos en NVS. Esta organización permite mantener una base de configuración clara, pero al mismo tiempo ofrece flexibilidad para ajustar el comportamiento del robot durante la operación.

## 10. SRTM

### 10.1 Criterios de trazabilidad

La matriz de trazabilidad de requisitos del sistema, o SRTM, permite relacionar cada requisito funcional y no funcional con el diseño del firmware, los módulos de código que lo implementan, los casos de prueba asociados y las evidencias esperadas.

El propósito de esta trazabilidad es demostrar que cada requisito del proyecto tiene una correspondencia clara con una parte del diseño y con una implementación verificable. De esta manera, el cumplimiento del sistema no depende únicamente de que el robot funcione físicamente, sino de que exista una relación documentada entre requisito, diseño, código y prueba.

La trazabilidad también contempla los elementos asociados a comunicación e interfaz remota. Por ejmeplo la conexión WiFi, MQTT, la publicación de telemetría, la recepción de parámetros desde la página web y la persistencia de configuración mediante NVS.

Para este proyecto, la trazabilidad se organiza usando los siguientes criterios:

- Req ID: Identificador del requisito funcional o no funcional.
- Descripción: Resumen del comportamiento esperado del sistema.
- Módulo de firmware asociado: Componente o archivo encargado de implementar la funcionalidad.
- Archivo de código: Archivo específico donde se encuentra la implementación principal.
- Caso de prueba: Prueba utilizada para verificar el cumplimiento del requisito.
- Evidencia esperada: Elemento objetivo que permite demostrar el resultado de la prueba.

### 10.2 Matriz de trazabilidad completa

INSERTAR MATRIZ DE TRAZABILIDAD

## 11. Casos de prueba y evidencias

### 11.1 Estrategia general de pruebas

La estrategia de pruebas del firmware tiene como propósito verificar que el sistema embebido cumple con los requisitos funcionales y no funcionales definidos para el robot seguidor de línea. Las pruebas se diseñan para validar que cada módulo del firmware cumple su responsabilidad dentro del sistema y que la integración entre sensores, control, motores, entrada START, pantalla OLED y logging funciona de manera coherente.

Los casos de prueba se estructuran a partir de la matriz de trazabilidad SRTM. Esto permite que cada requisito tenga al menos una prueba asociada y que cada prueba esté relacionada con una parte específica del diseño o del código. De esta manera, la validación del sistema no se basa únicamente en observar que el robot se mueve, sino en comprobar que cada función relevante fue diseñada, implementada y evaluada.

La estrategia de pruebas contempla pruebas de inicialización, lectura de sensores, detección de línea, cálculo de posición, cálculo de error, control PID, actuación sobre motores, habilitación mediante START, parada segura, visualización en OLED, logging serial, configuración de parámetros y compilación del firmware.

Las pruebas se plantean en diferentes niveles. Algunas pruebas son de módulo, porque verifican el comportamiento de un componente específico, como `line_sensors`, `line_control`, `motor_driver`, `start_input`, `oled_display` o `logger`. Otras pruebas son de integración, porque validan la interacción entre varios módulos. Finalmente, se contempla una prueba funcional del sistema completo, en la cual el robot debe seguir una línea sobre una pista y responder correctamente ante condiciones normales y condiciones de seguridad.

## 12. Conclusiones técnicas del diseño de firmware

El firmware desarrollado para el robot seguidor de línea integra las funciones principales de un sistema embebido: sensado, calibración, procesamiento, actuación, visualización, logging, comunicación remota y respuesta segura ante condiciones no válidas de operación. La implementación sobre ESP32, utilizando ESP-IDF en lenguaje C dentro de PlatformIO, permite mantener una estructura organizada y cercana a un desarrollo embebido profesional.

La arquitectura modular facilita la separación de responsabilidades. `src/main.c` coordina la operación general del sistema, mientras que los componentes en `components/` se encargan de funciones específicas como configuración, calibración de sensores, lectura de línea, control PID, manejo de motores, entrada START, pantalla OLED, logging y comunicación WiFi/MQTT. Esta organización mejora la mantenibilidad del código y permite relacionar cada módulo con los requisitos del proyecto.

El flujo principal del firmware transforma las lecturas del arreglo QTR en acciones de control sobre los motores. Para ello, los sensores se calibran y normalizan, se calcula la posición estimada de la línea, se obtiene el error de seguimiento y se ejecuta el controlador PID. La corrección resultante se combina con la velocidad base para generar velocidades diferenciales en los motores izquierdo y derecho.

La seguridad operacional se apoya principalmente en la entrada física START y en la respuesta ante pérdida de línea. El robot no inicia movimiento hasta recibir una habilitación física, y si START se desactiva, los motores se detienen. Además, ante una pérdida temporal de línea, el firmware intenta recuperar la trayectoria durante un número limitado de ciclos; si no lo logra, ejecuta una parada segura.

Las interfaces de observación permiten supervisar el comportamiento del sistema desde diferentes medios. La pantalla OLED conectada por I2C muestra información local del estado del robot; el logger por salida serial permite depurar y revisar eventos durante las pruebas y la comunicación WiFi/MQTT permite enviar telemetría hacia una página web.

La interfaz web amplía la capacidad de monitoreo y ajuste del robot. Desde ella se pueden observar variables como posición, error, velocidades, corrección, estado de línea, START, motores y sensores. También permite modificar parámetros como `KP`, `KI`, `KD` y velocidad base durante la ejecución, evitando recompilar el firmware para cada ajuste.

La persistencia de parámetros mediante NVS permite conservar los valores recibidos desde la interfaz web después de reiniciar la ESP32. De esta manera, el sistema combina una configuración base definida en `robot_config` con ajustes dinámicos realizados durante las pruebas.

En conclusión, el firmware constituye una base funcional, modular y trazable para el robot seguidor de línea. Su diseño permite relacionar requisitos del sistema con módulos de código, pruebas y evidencias, cumpliendo con el propósito del Embedded Firmware Design Document y proporcionando una estructura clara para la validación final del proyecto.

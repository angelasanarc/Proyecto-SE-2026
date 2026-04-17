# Requisitos del sistema

**Proyecto:** Robot seguidor de línea tipo velocista con ESP32  
**Versión:** 1.0  
**Fecha:** 25/03/2026

---

## 1. Descripción general

El presente documento define los requisitos funcionales y no funcionales del sistema para el proyecto **Robot seguidor de línea tipo velocista con ESP32**. El objetivo del sistema es detectar y seguir una trayectoria de manera autónoma, estable y repetible, incorporando calibración de sensores, ajuste del controlador, monitoreo del estado del sistema, manejo de errores, registro de eventos y comunicación con interfaz de usuario.

Este documento servirá como base para el diseño, implementación, pruebas y trazabilidad del proyecto.

---

## 2. Alcance

El sistema deberá:

- Detectar una línea mediante sensores infrarrojos
- Calcular la posición relativa de la línea
- Controlar diferencialmente los motores para seguir la trayectoria
- Permitir la calibración de sensores y parámetros de control
- Registrar eventos y errores
- Supervisar condiciones de operación
- Mantener un comportamiento repetible ante distintas corridas de prueba
- Reducir el impacto de variaciones eléctricas debidas al nivel de batería

---

## 3. Convenciones

- **RF-XXX:** Requisito funcional
- **RNF-XXX:** Requisito no funcional

Cada requisito incluye:
- Identificador
- Descripción
- Prioridad
- Criterio de aceptación

---

## 4. Requisitos funcionales

| ID | Requisito | Prioridad | Criterio de aceptación |
|---|---|---:|---|
| RF-001 | El sistema deberá ejecutar una secuencia de inicialización al encenderse, verificando sensores, actuadores, comunicaciones e interfaz antes de habilitar la operación. | Alta | Al energizar el robot, el sistema muestra estado de listo o reporta un error de inicialización. |
| RF-002 | El sistema deberá ejecutar una rutina de calibración inicial del arreglo de sensores para obtener los valores mínimos y máximos requeridos para la detección confiable de la línea. | Alta | Después de la calibración se almacenan valores mínimos y máximos válidos para cada sensor. |
| RF-003 | El sistema deberá validar la calibración realizada y bloquear el inicio del seguimiento si la calibración no es válida. | Alta | Si la calibración falla, el robot no entra en modo de seguimiento y reporta el error. |
| RF-004 | El sistema deberá adquirir de forma continua las lecturas del arreglo de sensores infrarrojos durante la operación. | Alta | El firmware registra lecturas actualizadas de todos los sensores definidos en el sistema. |
| RF-005 | El sistema deberá calcular en tiempo real una variable de error asociada a la posición relativa de la línea respecto al eje del robot. | Alta | El valor del error cambia de forma coherente al desplazar la línea hacia la izquierda o derecha. |
| RF-006 | El sistema deberá controlar diferencialmente la velocidad de los motores a partir de la señal de error y de un controlador PID implementado en firmware. | Alta | El robot corrige su trayectoria al detectar desviaciones laterales respecto a la línea. |
| RF-007 | El sistema deberá permitir la configuración de los parámetros del controlador PID y de parámetros de operación del robot mediante la interfaz de usuario. | Alta | Es posible modificar al menos Kp, Ki, Kd y velocidad base sin recompilar el firmware. |
| RF-008 | El sistema deberá permitir registrar variables de prueba para análisis y ajuste del sistema, incluyendo lecturas de sensores, error de seguimiento, salida de control y estado de motores. | Media | Durante una prueba se exportan o visualizan los datos definidos para análisis posterior. |
| RF-009 | El sistema deberá seguir de manera autónoma la línea una vez finalizadas correctamente la inicialización y la calibración. | Alta | El robot recorre un tramo de pista de prueba sin intervención manual continua. |
| RF-010 | El sistema deberá detectar la condición de pérdida de línea cuando las lecturas de los sensores no permitan determinar una posición válida de la trayectoria. | Alta | Al retirar el robot de la línea o perder referencia, el sistema detecta el evento. |
| RF-011 | El sistema deberá ejecutar una acción segura ante pérdida de línea prolongada o error crítico, como detenerse o entrar en modo seguro. | Alta | Ante un evento crítico, el robot detiene motores o cambia a un modo seguro definido. |
| RF-012 | El sistema deberá monitorear el estado de alimentación del robot para identificar condiciones que puedan afectar el desempeño del seguimiento. | Alta | El sistema detecta una condición anómala de alimentación y la reporta. |
| RF-013 | El sistema deberá incorporar una estrategia para mantener condiciones de operación consistentes frente a variaciones de la batería, mediante regulación o compensación equivalente. | Alta | El comportamiento del robot no presenta degradación severa dentro del rango operativo definido. |
| RF-014 | El sistema deberá implementar un módulo de logging que registre eventos relevantes con timestamp, severidad, código y mensaje. | Alta | Los eventos generados siguen un formato consistente y verificable. |
| RF-015 | El sistema deberá implementar comunicación UART para depuración, visualización de variables o salida de logs. | Alta | Se reciben correctamente logs o variables del sistema por puerto serial con la configuración definida. |
| RF-016 | El sistema deberá implementar un segundo protocolo de comunicación, I2C o SPI, para la interacción con al menos un periférico del sistema. | Alta | El periférico conectado responde correctamente y evidencia intercambio de datos válido. |
| RF-017 | El sistema deberá contar con una interfaz gráfica de usuario para visualizar estado, errores y variables relevantes del sistema. | Alta | La interfaz muestra al menos estado actual, una variable de operación y errores detectados. |
| RF-018 | El sistema deberá permitir la evaluación de repetibilidad mediante la ejecución de pruebas repetidas bajo condiciones controladas. | Media | El sistema puede ser probado varias veces bajo el mismo protocolo y generar resultados comparables. |

---

## 5. Requisitos no funcionales

| ID | Requisito | Prioridad | Criterio de aceptación |
|---|---|---:|---|
| RNF-001 | El sistema deberá mantener un comportamiento repetible entre corridas equivalentes de prueba. | Alta | La variación entre pruebas se mantiene dentro del rango definido por el equipo para tiempo de recorrido, error o pérdidas de línea. |
| RNF-002 | El sistema deberá conservar un desempeño funcional estable dentro del rango operativo de la batería especificado por el equipo. | Alta | El robot mantiene seguimiento aceptable durante el rango de carga definido sin descontrol severo. |
| RNF-003 | El ciclo de lectura de sensores, cálculo del error y actualización del control deberá ejecutarse dentro de un tiempo máximo definido por el equipo. | Alta | El tiempo de ejecución del ciclo de control cumple el límite establecido durante pruebas. |
| RNF-004 | El firmware deberá estar organizado de forma modular, separando al menos sensado, control, comunicaciones, interfaz y logging. | Media | La estructura del código permite identificar claramente los módulos funcionales del sistema. |
| RNF-005 | Cada requisito deberá tener al menos un caso de prueba asociado y evidencia verificable en la matriz de trazabilidad del proyecto. | Alta | Todos los requisitos quedan relacionados con pruebas y evidencias documentadas. |
| RNF-006 | El sistema deberá responder de forma segura ante pérdida de línea, fallo de sensor o condición anómala de alimentación. | Alta | Ante estas condiciones el robot no entra en estados inseguros y ejecuta la acción definida. |

---

## 6. Consideraciones de ingeniería derivadas de la experiencia previa

Durante pruebas previas con robots seguidores de línea se observó que el desempeño del sistema depende fuertemente de:

1. La correcta calibración inicial del arreglo de sensores
2. La sintonización adecuada del controlador PID
3. La repetibilidad del comportamiento en distintas corridas
4. La estabilidad de la alimentación eléctrica

Se identificó además que la ausencia de regulación o compensación de voltaje puede alterar la velocidad de los motores a medida que la batería se descarga, afectando el ajuste efectivo del controlador y provocando pérdida de estabilidad. Por esta razón, el monitoreo de alimentación y la estabilidad operativa se consideran aspectos críticos del sistema.

---

## 7. Supuestos y restricciones

- El robot empleará una ESP32 como unidad principal de procesamiento
- El sensado de línea se realizará con un arreglo de sensores infrarrojos
- El sistema contará con dos motores para tracción diferencial
- El proyecto deberá documentar y justificar los protocolos de comunicación utilizados
- Los parámetros configurables deberán poder ajustarse sin necesidad de modificar el código fuente manualmente antes de cada prueba
- Las métricas exactas de repetibilidad, tiempos máximos de ciclo y rango de batería serán definidas por el equipo durante la etapa de pruebas

---

## 8. Trazabilidad futura

Este documento se complementará con:

- Plan de pruebas
- Plantilla de test cases
- Matriz SRTM de trazabilidad
- Evidencias experimentales de validación

---

# Proyecto de Aula – Seguidor de Línea Tipo Velocista con ESP32

<p align="center">
  <img src="assets/img/robot_portada.png" alt="Robot seguidor de línea tipo velocista con ESP32" width="850">
</p>

<p align="center"><em>Propuesta visual del robot seguidor de línea tipo velocista desarrollado con ESP32.</em></p>

## Introducción

En el auge de la cuarta revolución industrial, los sistemas embebidos forman parte fundamental de numerosos dispositivos tecnológicos que facilitan el día a día, salvan vidas y aportan al desarrollo global. Un sistema embebido es un dispositivo diseñado para realizar una o pocas funciones específicas, generalmente dentro de un sistema electrónico o mecánico de mayor tamaño. Los microcontroladores se utilizan como el componente central de estos sistemas, ya que integran en un solo circuito las diferentes partes que lo componen. Éstos son capaces de recibir información, procesarla mediante algoritmos programados y generar señales de control para actuadores como luces o motores.

Por la capacidad de percibir su entorno, tomar decisiones basadas en esa información y ejecutar acciones sin intervención humana, los microcontroladores son ideales para controlar sistemas autónomos.

Un ejemplo de este tipo de aplicación es el carrito seguidor de línea, objeto de este proyecto, en el cual un microcontrolador procesa la información proveniente de sensores que detectan una línea marcada en el suelo, y a partir de esta entrada, controla la velocidad y dirección de los motores para mantener la trayectoria establecida. De esta forma, el sistema embebido permite que el carrito se desplace de manera autónoma al mismo tiempo que es capaz de comunicarse con el usuario para brindar información sobre su estado. Los robots de navegación autónoma tienen diversas aplicaciones en diferentes campos, por lo que resulta pertinente diseñar un sistema embebido encargado de la integración entre sensores, procesamiento y actuadores sin intervención humana.

## Objetivos

### Objetivo general
Diseñar e implementar un sistema embebido basado en una ESP32 para controlar un robot seguidor de línea tipo velocista, capaz de detectar y seguir de manera autónoma una trayectoria marcada sobre una pista, manteniendo estabilidad, rapidez de respuesta y precisión en el movimiento.

### Objetivos específicos
- Diseñar la arquitectura general del sistema, integrando sensado, procesamiento, actuación, comunicación y visualización.
- Seleccionar e integrar los componentes electrónicos y mecánicos necesarios para el funcionamiento del robot.
- Implementar el sistema de detección de línea mediante sensores reflectivos QTR-8RC.
- Desarrollar el firmware en la ESP32 para la lectura de sensores, el procesamiento de datos y el control del robot en tiempo real.
- Controlar los motores mediante el driver TB6612 para corregir la trayectoria de forma rápida y estable.
- Incorporar un sistema de encendido e inicio de operación mediante control láser.
- Implementar una pantalla OLED para la visualización del estado del sistema y variables relevantes.
- Incorporar mecanismos de manejo de errores ante eventos como pérdida de línea, lecturas inconsistentes o fallas de operación.
- Alimentar el sistema con una batería LiPo de 300 mAh y 7.4 V que permita la operación autónoma del prototipo.
- Validar experimentalmente el desempeño del robot sobre una pista de prueba.
- Documentar el diseño, integración y resultados del proyecto en el repositorio.

## Asignación de roles

### Líder técnico (Technical Lead)
**Responsable:** Angela Sanchez  
Encargada de liderar el desarrollo técnico del proyecto, definir la arquitectura general del sistema, coordinar al equipo de trabajo y supervisar la integración final entre hardware y firmware.

### Ingeniero de integración de hardware (Hardware Integration Engineer)
**Responsable:** Simon Patiño  
Encargado del diseño e integración física del sistema, interconexión de componentes electrónicos, ensamblaje del hardware, montaje del prototipo y validación del funcionamiento eléctrico y mecánico.

### Ingeniera de firmware (Firmware Engineer)
**Responsable:** Laura Maya  
Encargada del desarrollo del firmware en la ESP32, implementación de la lógica de control, adquisición y procesamiento de señales de sensores, control de actuadores, lectura del sistema de inicio por láser y organización del software embebido.

### Ingeniero de verificación y pruebas (Verification & Testing Engineer)
**Responsable:** Jeronimo Zapata  
Encargado del diseño y ejecución del plan de pruebas, verificación del cumplimiento de requisitos, recolección de evidencias experimentales y apoyo en la construcción de la matriz de trazabilidad y validación.

## Descripción detallada del problema que se desea trabajar

El problema central que se desea abordar en este proyecto es el desarrollo de un robot seguidor de línea tipo velocista controlado por una ESP32, capaz de desplazarse de forma autónoma sobre una pista marcada, manteniendo una trayectoria estable y corrigiendo su movimiento en tiempo real. A diferencia de un seguidor de línea convencional, este sistema no solo debe detectar la línea y permanecer sobre ella, sino hacerlo con mayor velocidad, precisión y capacidad de respuesta frente a curvas, cambios de dirección y perturbaciones durante el recorrido.

El sistema integrará una **ESP32** como unidad principal de procesamiento, un arreglo de sensores **QTR-8RC** para la detección de la línea, un driver de motores **TB6612** para el accionamiento de los actuadores, una batería **LiPo de 300 mAh, 7.4 V, 2S, 2.22 Wh** como fuente de alimentación, **motorreductores Pololu 10:1** como sistema de tracción, un **control láser de inicio/parada** para habilitar la operación del robot y una **pantalla OLED I2C** para la visualización del estado del sistema.

El desafío técnico radica en que el robot debe interpretar continuamente la información captada por los sensores QTR-8RC para estimar la posición relativa de la línea respecto al chasis. Con base en esta información, la ESP32 debe procesar los datos y generar señales de control hacia el TB6612, el cual accionará los motorreductores Pololu para corregir la trayectoria. Este proceso debe ejecutarse en tiempos muy cortos, ya que cualquier retraso en la lectura, procesamiento o actuación puede provocar oscilaciones, pérdida de estabilidad o salida de la pista.

Adicionalmente, el sistema incorporará un mecanismo de control externo mediante láser para habilitar o detener la operación del robot de forma controlada antes de la carrera o durante pruebas. Esta característica permite mejorar la seguridad, facilitar el arranque sincronizado y adaptar el sistema a dinámicas de competencia o validación experimental.

Por otra parte, la pantalla OLED permitirá visualizar información relevante del robot en tiempo real, como el estado general del sistema, modo de operación, confirmación de inicio, errores detectados, nivel de batería o variables básicas de depuración. Esto mejora la interacción con el usuario y facilita tanto la fase de pruebas como la supervisión del funcionamiento del prototipo.

Este problema es pertinente dentro del contexto de sistemas embebidos porque exige la integración coordinada de varios elementos de hardware y software. En primer lugar, el proyecto involucra **sensado físico real**, dado que los sensores QTR-8RC permiten detectar el contraste entre la línea y la superficie de la pista. En segundo lugar, requiere **actuación electrónica real**, ya que la ESP32 debe controlar de forma dinámica la velocidad y dirección de los motores a través del TB6612. Finalmente, el sistema necesita una **lógica de control en tiempo real**, capaz de traducir señales de entrada en acciones correctivas inmediatas.

Adicionalmente, el desarrollo del proyecto exige una estructura de firmware organizada en módulos funcionales, tales como lectura de sensores, cálculo de posición, generación de acciones de control, lectura del módulo de inicio, visualización en pantalla, manejo de errores, monitoreo y comunicación. De esta manera, el robot no se limita a ejecutar una secuencia simple de instrucciones, sino que constituye una solución embebida completa y técnicamente justificada.

Otro aspecto importante del problema es la necesidad de contemplar condiciones de falla. Durante la operación del robot pueden presentarse eventos como pérdida de línea, lecturas inconsistentes, ruido en los sensores, variaciones en la tensión de la batería o respuestas inadecuadas de los motores. Por esta razón, el sistema debe incluir mecanismos que permitan detectar estas anomalías y responder de manera segura, por ejemplo reduciendo la velocidad, corrigiendo de manera más agresiva la trayectoria o deteniendo el robot si es necesario.

Asimismo, el proyecto permite incorporar monitoreo y registro de eventos, ya que durante las pruebas será fundamental observar variables relevantes del sistema, como lecturas de sensores, estados de operación, comportamiento de los motores y posibles errores. Esto facilitará la depuración, el ajuste del control y la validación experimental del prototipo.

En conjunto, el problema propuesto representa una aplicación completa de sistemas embebidos, al integrar percepción del entorno, procesamiento local, toma de decisiones, visualización de información y actuación autónoma. Además, su desarrollo tiene valor académico y práctico, pues reproduce un escenario real de automatización móvil donde un sistema debe reaccionar en tiempo real ante información captada del entorno.

## Justificación del cumplimiento de los requisitos del proyecto

El desarrollo de este robot seguidor de línea permite satisfacer de manera clara los requerimientos fundamentales del proyecto:

- **Uso de sistema embebido:** la ESP32 actúa como unidad central de procesamiento, ejecutando la lógica de control y coordinando todas las funciones del sistema.
- **Integración de sensor físico:** el robot emplea un arreglo de sensores QTR-8RC para detectar la línea y obtener información del entorno en tiempo real.
- **Integración de actuador:** los motorreductores Pololu 10:1 son controlados electrónicamente mediante el driver TB6612, de acuerdo con las decisiones generadas por el firmware.
- **Procesamiento en firmware:** la lógica del sistema se implementa en la ESP32 para leer sensores, procesar datos y generar acciones correctivas sobre los motores.
- **Manejo de errores:** el sistema contempla situaciones como pérdida de línea, lecturas inválidas, ruido en sensores o caídas en la alimentación.
- **Comunicación y monitoreo:** se podrán implementar mecanismos de comunicación serial para depuración, monitoreo y visualización de variables del sistema.
- **Interfaz de visualización:** la pantalla OLED permitirá mostrar estados del robot, errores y variables importantes durante la operación.
- **Inicio y parada controlados:** el uso del control láser permitirá activar o desactivar la operación del robot de forma controlada y segura.
- **Registro de eventos:** se podrán registrar estados relevantes del robot durante su operación, como inicio, error, pérdida de línea y condiciones anómalas.
- **Validación experimental:** el prototipo será probado sobre una pista real para verificar su desempeño y cumplimiento de los objetivos planteados.
- **Documentación técnica:** el proyecto incluye organización del repositorio, descripción de arquitectura, pruebas y evidencias del desarrollo.

## Alcance del proyecto

El alcance de este proyecto comprende el diseño, desarrollo, integración y validación de un robot seguidor de línea tipo velocista basado en una ESP32, orientado al seguimiento autónomo de una trayectoria marcada sobre una pista de prueba. El sistema será concebido como un prototipo funcional capaz de captar información del entorno, procesarla en tiempo real y ejecutar acciones correctivas sobre sus actuadores para mantener un desplazamiento estable y eficiente.

Dentro del alcance del proyecto se incluye la integración de un arreglo de sensores **QTR-8RC** para la detección de línea, el uso de un driver **TB6612** para el control de potencia de los motores, la implementación de **motorreductores Pololu 10:1** como sistema de tracción, el uso de una batería **LiPo de 300 mAh, 7.4 V, 2S, 2.22 Wh** como fuente de alimentación, la incorporación de un **control láser** para el encendido o habilitación del robot y la integración de una **pantalla OLED I2C** para la visualización de estados y variables del sistema.

También se incluye el desarrollo del firmware en la ESP32, la definición de la lógica de navegación sobre pista, el manejo de errores, el monitoreo del estado del sistema y la validación del funcionamiento mediante pruebas experimentales.

Desde el punto de vista físico, el alcance incluye la construcción del prototipo, el ensamblaje del sistema electrónico, la integración sobre el chasis y la verificación de su funcionamiento global en condiciones controladas. Igualmente, se contempla la elaboración de la documentación técnica necesaria para describir el diseño, el desarrollo y los resultados obtenidos.

El prototipo final deberá ser capaz de seguir una línea de forma autónoma, responder ante desviaciones, mantener estabilidad en trayectorias curvas y operar con una lógica de control acorde con el enfoque de robot velocista. Además, deberá contar con un sistema de visualización básica y un mecanismo externo de activación que apoye las pruebas y la operación controlada del robot.

Sin embargo, el alcance del proyecto se limita a un entorno académico y controlado, por lo que no se incluyen funciones avanzadas como visión artificial, navegación en entornos no estructurados, mapeo del entorno, localización global o inteligencia artificial compleja.

Tampoco hace parte del alcance la fabricación comercial del sistema ni su implementación en aplicaciones industriales de gran escala. El enfoque está centrado en desarrollar una solución embebida funcional, justificable y bien documentada, que permita demostrar la integración efectiva entre sensores, procesamiento, visualización y actuadores en un sistema autónomo.

## Tecnologías y componentes principales

- **ESP32:** microcontrolador principal encargado del procesamiento, control y ejecución del firmware.
- **TB6612:** driver de motores utilizado para controlar el sentido de giro y la velocidad de los motores DC.
- **Batería LiPo 300 mAh, 7.4 V, 2S, 2.22 Wh:** fuente de alimentación principal del sistema.
- **Sensores QTR-8RC:** arreglo de sensores reflectivos empleado para detectar la línea y estimar la posición relativa del robot respecto a la trayectoria.
- **Motorreductores Pololu 10:1:** actuadores principales del sistema de tracción, seleccionados por su relación entre velocidad y torque para un robot tipo velocista.
- **Control láser Ja-Bots:** sistema de activación y parada externa del robot para pruebas y operación controlada.
- **Pantalla OLED I2C:** interfaz de visualización para mostrar estados, errores y variables relevantes del sistema.

## Funcionalidades principales esperadas

- Detección de línea mediante sensores reflectivos.
- Cálculo de la posición relativa de la línea respecto al robot.
- Corrección de trayectoria en tiempo real.
- Control diferencial de motores para giros y ajuste de rumbo.
- Activación o habilitación del robot mediante control láser.
- Visualización del estado del sistema en pantalla OLED.
- Monitoreo de errores y eventos relevantes.
- Operación autónoma sobre una pista de prueba.


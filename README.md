# Proyecto de Aula – Seguidor de Línea Tipo Velocista con ESP32

## Introducción
En el auge de la cuarta revolución industrial, los sistemas embebidos forman parte fundamental de numerosos dispositivos tecnológicos que facilitan el día a día, salvan vidas y aportan al desarrollo global. Un sistema embebido es un dispositivo diseñado para realizar una o pocas funciones específicas, generalmente dentro de un sistema electrónico o mecánico de mayor tamaño. Los microcontroladores se utilizan como el componente central de estos sistemas, ya que integran en un solo circuito las diferentes partes que lo componen. Éstos son capaces de recibir información, procesarla mediante algoritmos programados y generar señales de control para actuadores como luces o motores. Por la capacidad de percibir su entorno, tomar decisiones basadas en esa información y ejecutar acciones sin intervención humana, los microcontroladores son ideales para controlar sistemas autónomos. 

Un ejemplo de este tipo de aplicación es el carrito seguidor de línea, objeto de este proyecto, en el cual un microcontrolador procesa la información proveniente de sensores que detectan una línea marcada en el suelo, y a partir de esta entrada, controla la velocidad y dirección de los motores para mantener la trayectoria establecida. De esta forma, el sistema embebido permite que el carrito se desplace de manera autónoma al mismo tiempo que es capaz de comunicarse con el usuario para brindar información sobre su estado. Los robots de navegación autónoma tienen diversas aplicaciones en diferentes campos, por lo que resulta pertinente diseñar un sistema embebido encargado de la integración entre sensores, procesamiento y actuadores sin intervención humana. 


## Objetivos

### Objetivo general
Diseñar e implementar un sistema embebido basado en una ESP32 para controlar un robot seguidor de línea tipo velocista, capaz de detectar una trayectoria marcada sobre una pista y seguirla de manera autónoma, rápida y estable mediante el uso de sensores, procesamiento en tiempo real y control de actuadores.

### Objetivos específicos
- Diseñar la arquitectura general del sistema, integrando sensado, procesamiento, actuación, comunicación y monitoreo.
- Seleccionar e implementar los sensores necesarios para detectar la línea y estimar la posición relativa del robot respecto a la trayectoria.
- Desarrollar el firmware en la ESP32 para la adquisición de datos, procesamiento de señales y control del movimiento del robot.
- Implementar el control de los motores mediante una lógica que permita corregir la trayectoria con rapidez y estabilidad.
- Incorporar mecanismos de manejo de errores y eventos críticos, como pérdida de línea, lecturas inválidas o fallas en la operación.
- Diseñar un sistema de comunicación y visualización del estado del robot para facilitar la depuración, configuración y seguimiento de variables del sistema.
- Construir e integrar el prototipo físico, validando su funcionamiento sobre una pista de prueba.
- Verificar el cumplimiento de los requisitos del proyecto mediante pruebas funcionales, documentación técnica y trazabilidad.

## Asignación de roles

### Technical Lead
**Responsable:** Angela Sanchez  
Encargada de liderar el desarrollo técnico del proyecto, definir la arquitectura general del sistema, coordinar la integración entre hardware y firmware, tomar decisiones de diseño y supervisar el cumplimiento de los objetivos planteados.

### Hardware Integration Engineer
**Responsable:** Simon Patiño  
Encargado del diseño e integración física del sistema, selección e interconexión de componentes electrónicos, montaje del circuito, ensamblaje del chasis y validación del funcionamiento del hardware del prototipo.

### Firmware Engineer
**Responsable:** Laura Maya  
Encargada del desarrollo del firmware en la ESP32, implementación de la lógica de control, adquisición y procesamiento de datos de sensores, manejo de actuadores, comunicaciones y estructura del software embebido.

### Verification & Testing Engineer
**Responsable:** Jeronimo Zapata  
Encargado de diseñar y ejecutar el plan de pruebas, verificar el cumplimiento de los requisitos funcionales y no funcionales, documentar resultados, gestionar evidencias de validación y apoyar la construcción de la matriz de trazabilidad.

## Descripción detallada del problema que se desea trabajar

El problema central que se desea abordar en este proyecto es el desarrollo de un robot seguidor de línea tipo velocista controlado por una ESP32, capaz de desplazarse de manera autónoma sobre una pista marcada, manteniendo una trayectoria estable aun cuando se presenten curvas, cambios bruscos de dirección o pequeñas perturbaciones durante el recorrido. A diferencia de un seguidor de línea convencional, el objetivo de un robot velocista no es únicamente detectar una línea y moverse sobre ella, sino hacerlo con un mayor nivel de rapidez, precisión y capacidad de respuesta, lo que convierte el problema en un reto de integración entre sensado, procesamiento y control en tiempo real.

El desafío principal radica en que el sistema debe interpretar continuamente la información proveniente de sensores ubicados en la parte inferior del robot para determinar la posición relativa de la línea respecto al chasis. A partir de esta información, la ESP32 debe ejecutar una lógica de control que ajuste de forma inmediata la velocidad de los motores y la dirección de avance, evitando que el robot se desvíe o abandone la pista. Este proceso debe realizarse en tiempos muy cortos, ya que cualquier retraso en la lectura, procesamiento o actuación puede traducirse en errores de trayectoria, oscilaciones excesivas o pérdida total de la línea.

Desde el punto de vista técnico, este problema permite desarrollar un sistema embebido completo y coherente con los lineamientos del curso. En primer lugar, el proyecto incorpora **sensado físico real**, ya que la detección de la línea dependerá de sensores ópticos o infrarrojos capaces de identificar contrastes entre la superficie de la pista y la trayectoria marcada. Esto garantiza que el sistema reciba información del entorno en tiempo real y tome decisiones basadas en variables físicas medibles.

En segundo lugar, el proyecto incluye **actuación electrónica real**, ya que el robot deberá controlar motores de corriente continua mediante señales de mando generadas por la ESP32 y acondicionadas por una etapa de potencia. La respuesta del sistema no será fija ni manual, sino dependiente de una lógica embebida que relacione directamente las lecturas de los sensores con la acción de los motores. En consecuencia, el proyecto no solo detecta información del entorno, sino que responde activamente a ella de manera autónoma.

Asimismo, el problema exige una **arquitectura de firmware organizada**, debido a que no basta con programar una secuencia simple de instrucciones. El sistema deberá estructurarse en módulos claramente definidos, como adquisición de sensores, procesamiento de señales, generación de acciones de control, comunicaciones, registro de eventos y manejo de errores. Esta organización es importante para asegurar mantenibilidad, escalabilidad y claridad en el diseño del software embebido.

Otro aspecto relevante es el **manejo de errores y condiciones de falla**. En un robot seguidor de línea pueden presentarse múltiples situaciones anómalas, como lecturas inconsistentes, pérdida de referencia de la línea, ruido en las señales, caída de tensión o fallos de comunicación. Por esta razón, el sistema debe ser capaz de detectar estas situaciones y responder de forma segura, por ejemplo, reduciendo velocidad, corrigiendo trayectoria, generando alertas o deteniendo el robot si es necesario. Esto fortalece el proyecto desde la perspectiva de robustez y confiabilidad.

Además, este trabajo justifica la implementación de un **sistema de logging o registro de eventos**, ya que durante el funcionamiento del robot resulta útil almacenar o transmitir información sobre estados relevantes, como inicio de operación, pérdida de línea, cambios de modo, errores de lectura, maniobras correctivas y variables críticas del sistema. Este registro será clave tanto para la depuración como para la validación experimental del prototipo.

El proyecto también permite integrar **protocolos de comunicación**, necesarios para depuración, monitoreo o interacción con periféricos. Estas comunicaciones pueden emplearse para transmitir datos de sensores, parámetros de control, estados internos o mensajes de error hacia una interfaz externa. Esto aporta valor técnico al proyecto al ampliar la visibilidad del sistema y facilitar su ajuste durante las etapas de prueba.

Adicionalmente, se contempla la posibilidad de implementar una **interfaz de usuario** que permita visualizar información del estado del robot y modificar ciertos parámetros de operación, como velocidad base, sensibilidad o modo de funcionamiento. Esto enriquece la experiencia de uso y aporta un componente de interacción que va más allá del simple desplazamiento autónomo.

En términos de ingeniería aplicada, este problema es significativo porque reúne elementos de electrónica, programación, control automático y diseño mecatrónico dentro de un solo sistema funcional. El desarrollo del robot implica seleccionar componentes adecuados, diseñar una estrategia de control eficiente, integrar correctamente las conexiones físicas y validar el desempeño del conjunto en un entorno real de operación.

En síntesis, el problema que se desea trabajar consiste en diseñar e implementar un robot seguidor de línea tipo velocista basado en ESP32, capaz de sensar una trayectoria, procesar información en tiempo real y controlar su movimiento de manera autónoma, estable y rápida. Al mismo tiempo, el proyecto permite justificar y materializar los principales requerimientos de un sistema embebido académico: sensado físico, actuación electrónica, lógica de control, manejo de errores, comunicaciones, monitoreo, validación y documentación técnica.

## Alcance del proyecto

El alcance de este proyecto comprende el diseño, desarrollo, integración y validación de un robot seguidor de línea tipo velocista controlado por una ESP32, orientado al seguimiento autónomo de una trayectoria marcada sobre una pista de prueba. El sistema será concebido como un prototipo funcional capaz de captar información del entorno, procesarla en tiempo real y ejecutar acciones correctivas sobre sus actuadores para mantener un desplazamiento estable y eficiente.

Dentro del alcance del proyecto se incluye el diseño de la arquitectura general del sistema embebido, la selección de los componentes principales, la integración de sensores para detección de línea, el acondicionamiento y control de motores, la implementación del firmware en la ESP32 y la definición de la lógica de navegación sobre pista. También forma parte del alcance la incorporación de mecanismos de manejo de errores, el registro de eventos relevantes del sistema, la comunicación para monitoreo y depuración, y el desarrollo de una forma básica de interacción con el usuario para consulta del estado del robot o ajuste de parámetros principales.

Desde el punto de vista físico, el alcance incluye la construcción del prototipo, el ensamblaje del sistema electrónico, la integración sobre el chasis y la validación del funcionamiento global mediante pruebas experimentales en una pista controlada. Igualmente, se contempla la elaboración de la documentación técnica del proyecto, incluyendo descripción del sistema, organización del desarrollo, evidencia de pruebas y trazabilidad de requisitos.

El prototipo final deberá ser capaz de seguir una línea de forma autónoma, responder ante desviaciones, mantener un comportamiento estable en trayectorias curvas y operar con una lógica de control coherente con el enfoque de robot velocista. Sin embargo, el alcance del proyecto se limita a un entorno controlado de laboratorio o demostración académica, por lo que no se incluyen funciones avanzadas como visión artificial, navegación en entornos no estructurados, localización global, mapeo del entorno o toma de decisiones basada en inteligencia artificial compleja.

Tampoco hace parte del alcance la industrialización del sistema, la fabricación comercial del producto ni el desarrollo de una plataforma de navegación multipropósito. El enfoque del proyecto está centrado en construir una solución embebida funcional, académicamente sólida y técnicamente justificable, que permita demostrar la correcta integración entre sensores, procesamiento, comunicaciones y actuadores en un sistema autónomo de pequeña escala.

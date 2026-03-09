# Proyecto de Aula – Seguidor de Línea Tipo Velocista con ESP32

## Introducción


## Objetivos


## Asignación de roles

### Technical Lead
**Responsable:** Angela Sanchez  
Encargada de definir la arquitectura general del sistema, liderar la toma de decisiones técnicas, coordinar al equipo de trabajo y supervisar la integración final del proyecto.

### Hardware Integration Engineer
**Responsable:** Simon Patiño  
Encargado del diseño e integración física del sistema, conexiones eléctricas, ensamblaje del hardware, validación de componentes y montaje del prototipo funcional.

### Firmware Engineer
**Responsable:** Laura Maya  
Encargada del desarrollo del firmware en la ESP32, implementación de la lógica de control, lectura de sensores, control de actuadores, manejo de errores y documentación del código.

### Verification & Testing Engineer
**Responsable:** Jeronimo Zapata  
Encargado de diseñar el plan de pruebas, verificar el cumplimiento de requisitos, recolectar evidencias experimentales y elaborar la matriz de trazabilidad y validación del sistema.

## Descripción detallada del problema que se desea trabajar

El problema que se desea abordar en este proyecto es el desarrollo de un sistema embebido capaz de controlar un robot seguidor de línea tipo velocista, utilizando una ESP32 como unidad principal de procesamiento. El propósito del sistema es lograr que un vehículo autónomo detecte y siga una trayectoria marcada sobre una pista de manera rápida, estable y precisa, corrigiendo su movimiento en tiempo real frente a curvas, desalineaciones y cambios en la trayectoria.

Este problema es relevante porque integra varios retos propios de los sistemas embebidos reales. En primer lugar, el robot debe sensar físicamente su entorno mediante sensores que detecten la línea sobre la superficie. En este caso, la variable a medir puede interpretarse como la presencia, ausencia o intensidad relativa de detección de la línea en distintos puntos bajo el robot. Además, el sistema deberá contemplar manejo de error ante lecturas inválidas, pérdida de línea, desconexión de sensor o valores fuera de comportamiento esperado.

En segundo lugar, el proyecto requiere actuación electrónica real, ya que el robot necesita controlar motores para desplazarse y corregir su trayectoria. El movimiento no será manual ni fijo, sino dependiente de una lógica de firmware que interprete la información capturada por los sensores y ajuste la velocidad o dirección de los motores según la posición relativa respecto a la línea.

Adicionalmente, el problema es apropiado para el curso porque exige una arquitectura completa de hardware y firmware, no solo una solución improvisada. El sistema debe diseñarse con una estructura organizada, con módulos claros para sensado, procesamiento, control de actuadores, manejo de errores, logging y comunicación.

Otro aspecto importante es que el robot tipo velocista no busca únicamente seguir una línea de forma básica, sino hacerlo con un desempeño superior al de un prototipo elemental. Esto significa que el sistema deberá responder con baja latencia, mantener estabilidad en curvas y reducir oscilaciones o pérdidas de trayectoria. Por tanto, el problema tiene un componente claro de optimización y control en tiempo real.

Asimismo, este proyecto permite incluir un sistema de logging para registrar eventos relevantes, tales como inicio de carrera, pérdida de línea, corrección brusca de trayectoria, condición de error de sensor, cambios de modo, parada de emergencia o estado general del sistema.

También se pueden implementar protocolos de comunicación para depuración, monitoreo e interacción con periféricos, además de una interfaz gráfica de usuario que permita configurar parámetros del robot, visualizar estados y supervisar errores o variables de operación.

Desde la perspectiva física, el proyecto también contempla el ensamblaje del sistema en una solución funcional, integrando la electrónica, el chasis, la alimentación y los elementos de sensado y actuación necesarios para el funcionamiento del robot.

En síntesis, el problema consiste en diseñar e implementar un robot seguidor de línea tipo velocista que, por medio de una ESP32, sensores físicos y actuadores electrónicos, sea capaz de seguir de forma autónoma una trayectoria marcada en pista, integrando sensado, control, manejo de errores, validación y documentación técnica.

## Alcance del proyecto

El alcance de este proyecto comprende el diseño, implementación, integración y validación de un robot seguidor de línea tipo velocista basado en ESP32. El sistema estará orientado a detectar una línea en una pista y seguirla de forma autónoma mediante el procesamiento de señales de sensores y el control electrónico de motores.

Dentro del alcance se incluye el diseño general del sistema embebido, la selección e integración de sensores de línea, la programación del firmware en ESP32, el control de actuadores para el desplazamiento del robot, el manejo de errores del sistema, la implementación de mecanismos de monitoreo, la construcción física del prototipo y la documentación técnica del proyecto.

El proyecto llegará hasta un prototipo funcional demostrable, capaz de operar sobre una pista controlada. No hace parte del alcance el desarrollo de navegación libre, visión artificial avanzada, mapeo del entorno, autonomía en terrenos no estructurados ni aplicaciones industriales a gran escala. El enfoque estará centrado en una solución embebida académica, funcional y bien documentada.
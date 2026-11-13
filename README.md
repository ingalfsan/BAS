# Prototipo BAS para ESP32-S3

Prototipo de investigación inspirado en el marco teórico **Biogeophysical Admittance Spectroscopy (BAS)** y su arquitectura de orquestación distribuida **BCO**, descritos en [`BAS3.md`](BAS3.md).

## Estado del proyecto

Prototipo de banco inicial para Arduino IDE: sketch ESP32-S3, adquisición propuesta con dos sensores TMP117, telemetría JSON por puerto serie y modo simulado. El sketch y el montaje físico todavía deben compilarse y verificarse en la placa concreta.

## Objetivo

Explorar una plataforma modular basada inicialmente en un **ESP32-S3** que pueda adquirir y procesar señales de sensores, y que permita evaluar más adelante una arquitectura distribuida de nodos. El documento BAS propone diez dominios de sensado y un orquestador central; este repositorio comenzará como un prototipo y no presupone que todos esos dominios puedan implementarse o validarse.

La arquitectura conceptual de BAS asigna al nodo central tareas de coordinación y fusión de datos, y a los nodos de sensado el procesamiento específico de cada dominio. También propone **BAS-Link**, basado en ESP-NOW, como mecanismo de comunicación entre nodos. Estas decisiones deberán comprobarse frente al hardware y las necesidades reales del prototipo antes de considerarse definitivas.

## Dominios contemplados en el marco BAS

El marco teórico describe los siguientes dominios:

1. Óptico (CKM)
2. Dieléctrico
3. Inductivo (TEM)
4. Electroquímico (Hydro-Logics)
5. Volátiles (VASP)
6. Fotónico (emisión ultra débil)
7. Termodinámico
8. Autofluorescencia
9. Reológico
10. Magnetohidrodinámico (MHD)

La inclusión en esta lista no implica que un dominio esté implementado, que sus mediciones sean viables con el hardware disponible ni que tenga utilidad clínica demostrada. La selección inicial de sensores y experimentos se documentará al definir el prototipo.

## Plataforma

- **Microcontrolador objetivo inicial:** ESP32-S3.
- **Firmware principal:** Arduino IDE 2.x con el core **esp32 by Espressif Systems** 3.x.
- **Sketch:** [`firmware/BASPrototype/BASPrototype.ino`](firmware/BASPrototype/BASPrototype.ino).
- **Sensores de banco propuestos:** dos TMP117 en un bus I2C, con direcciones distintas para medir objeto y ambiente.
- **Arquitectura actual:** una placa, adquisición térmica y telemetría local. No están implementados BAS-Link, trabajadores distribuidos, PINN ni fusión multimodal.
- El cableado y los GPIO deben confirmarse para la placa concreta en [`docs/hardware.md`](docs/hardware.md).

### Preparar Arduino IDE

1. Instala Arduino IDE 2.x desde https://www.arduino.cc/en/software
2. Abre **File > Preferences**. En **Additional boards manager URLs**, agrega:

   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`

3. Ve a **Tools > Board > Boards Manager**, busca `esp32` e instala **esp32 by Espressif Systems**, versión 3.x.
4. Abre el sketch `firmware/BASPrototype/BASPrototype.ino`.
5. En **Tools > Board**, selecciona **ESP32S3 Dev Module** o el perfil indicado por el fabricante de tu placa. Conecta la placa y selecciona su puerto en **Tools > Port**.
6. Para probar sin sensores, cambia `kSimulationMode` a `true` en el sketch. Pulsa **Verify** para compilar y **Upload** para cargar el firmware.
7. Abre **Tools > Serial Monitor** y selecciona **115200 baud**. La telemetría simulada indicará `"mode":"simulated"`.

En placas que usan el USB nativo del ESP32-S3, quizá debas habilitar **Tools > USB CDC On Boot**. Las placas con puente USB-serie pueden requerir otra configuración. Para lecturas reales, confirma que los GPIO I2C `8` y `9` coincidan con el pinout de tu placa y conecta los dos TMP117 según [`docs/hardware.md`](docs/hardware.md). No necesitas instalar una librería aparte para el sensor: el sketch se comunica usando `Wire`.

No hace falta instalar ESP-IDF para compilar el sketch Arduino.

Las pruebas del núcleo térmico independiente de Arduino requieren un compilador C++ de escritorio (GCC o Clang):

```sh
python tests/run_arduino_core_tests.py
```

El proyecto ESP-IDF previo se conserva en `firmware/main/` como alternativa; no es necesario instalar ESP-IDF para usar el sketch Arduino. Consultar [`Plan.md`](Plan.md) para los criterios de verificación pendientes.

## Seguridad, alcance y limitaciones

Este proyecto es experimental y **no es un dispositivo médico ni una herramienta de diagnóstico o triaje validada**. No se debe usar para tomar decisiones clínicas, sustituir pruebas de referencia ni orientar tratamientos.

Las hipótesis, ecuaciones, asociaciones biológicas y proyecciones de desempeño de `BAS3.md` son la base teórica proporcionada para explorar el prototipo; no constituyen por sí mismas evidencia de eficacia o validación independiente. En particular, las cifras de desempeño del documento se presentan como objetivos teóricos, no como resultados experimentales completados.

Cualquier experimento que involucre muestras biológicas, sustancias químicas, electrodos, radiación UV u otros riesgos requiere evaluación de seguridad, supervisión apropiada y cumplimiento de las normas aplicables. Las pruebas iniciales deberían limitarse a bancos de prueba y materiales no biológicos seguros.

## Documento de referencia

- [`BAS3.md`](BAS3.md): transcripción en Markdown del marco BAS/BCO proporcionado para este proyecto. Consúltese el documento original para sus ecuaciones, arquitectura propuesta, protocolo de validación y referencias.

## Próximos pasos

Confirmar el modelo de placa, compilar y probar primero el modo simulado; después verificar el cableado y las lecturas de los TMP117 en un banco no biológico.

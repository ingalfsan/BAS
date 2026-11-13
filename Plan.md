# Plan inicial de implementación: primer prototipo BAS

## Estado de implementación

Se añadió un sketch Arduino IDE en `firmware/BASPrototype/` con lectura TMP117, modo simulado y telemetría, además de pruebas host para el núcleo térmico. Se propone Arduino IDE 2.x con el core Espressif ESP32 3.x. Falta compilar con Arduino IDE/CLI y verificar en la placa física; en este entorno no están instalados Arduino CLI, Arduino IDE ni un compilador C++. El firmware ESP-IDF anterior se conserva como alternativa.

## 1. Propósito y criterio de alcance

Construir un **prototipo de banco con una sola placa ESP32-S3** que adquiera datos reproducibles de un dominio físico, registre su calidad y permita ampliar el sistema sin rehacer el flujo de adquisición. El punto de partida será el **dominio termodinámico** descrito en la sección 3.7 de [`BAS3.md`](BAS3.md): medir la temperatura de un objeto de prueba y la del entorno para observar diferencias y tendencias, **sin inferir metabolismo ni condiciones de salud**.

Esta elección reduce el número de circuitos y riesgos iniciales. El primer prototipo no usa muestras biológicas, no entrega puntajes de triaje y no pretende validar las hipótesis clínicas del documento. Las decisiones de sensores, pines y frecuencia se confirmarán con el hardware disponible antes de fijarlas en firmware.

## 2. Resultado mínimo esperado (MVP)

Una placa ESP32-S3 ejecuta firmware que:

1. Inicia, identifica su versión y expone un registro de estado por puerto serie.
2. Lee dos puntos de temperatura (objeto de prueba y ambiente) mediante sensores digitales; sin hardware, usa un modo completamente simulado y marcado como tal, sin mezclar canales simulados con lecturas reales.
3. Emite registros con tiempo monotónico, identificador de canal, valor en °C, estado de la lectura y número de secuencia; identifica datos ausentes o inválidos.
4. Calcula la diferencia de temperaturas y una tendencia temporal simple en una ventana definida, únicamente cuando ambas lecturas son válidas.
5. Puede registrar y reproducir lecturas simuladas en pruebas de desarrollo, sin confundirlas con mediciones reales.

**Salida:** telemetría técnica por serie, legible por una persona y procesable por un script. No se muestra una clasificación médica, una probabilidad de enfermedad ni un índice clínico.

## 3. Hardware y herramientas propuestos

| Elemento | Uso | Decisión pendiente |
| --- | --- | --- |
| Placa ESP32-S3 con USB | Ejecución, consola y alimentación de banco | Modelo, memoria disponible y GPIO accesibles |
| Dos sensores digitales TMP117 | Medición de objeto y ambiente por I2C en direcciones distintas | Confirmar breakouts, pull-ups y compatibilidad eléctrica antes de conectar |
| Montaje de prueba no biológico | Observar un cambio térmico controlado | Material, aislamiento y límites de temperatura seguros |
| Cable USB y PC | Alimentación, programación y captura de registros | Puerto serie, perfil de placa y drivers si se requieren |

Se usa **Arduino IDE 2.x** con el core **esp32 by Espressif Systems 3.x**. No se fija un modelo de placa ni se promete precisión del sistema. Los pines predeterminados están definidos como constantes en el sketch; confirmarlos y revisar las fichas técnicas y protecciones eléctricas antes de conectar.

## 4. Etapas de ejecución y criterios de salida

### Etapa A: base reproducible

- Inicializar el repositorio Git y agregar `.gitignore`, sketch Arduino y documentación de instalación/compilación.
- Registrar modelo de placa, versión del core Arduino-ESP32, esquema de conexiones, tensión de alimentación y selección de sensores.
- Compilar, cargar y ejecutar una aplicación de arranque que imprima versión, identificador de placa y estado por serie.

**Salida:** un tercero puede compilar y cargar el firmware siguiendo instrucciones documentadas; no se incluyen credenciales ni datos personales en el repositorio.

### Etapa B: adquisición y calidad de datos

- Implementar una interfaz de canal de medición separada del controlador concreto del sensor.
- Leer los sensores a una frecuencia inicial de **1 muestra por segundo** (ajustable tras revisar sus fichas técnicas); verificar errores de bus, valores fuera de rango y lecturas faltantes.
- Separar los modos `real` y `simulado` en cada registro; usar tiempo monotónico para intervalos y número de secuencia para detectar huecos.
- Definir un formato estable de telemetría (por ejemplo, una línea JSON por ciclo) y documentar unidades y significado de los campos.

**Salida:** un registro de banco muestra ambas temperaturas y estados; desconectar un sensor produce un error explícito y no reutiliza una lectura antigua como actual.

### Etapa C: procesamiento y verificación de banco

- Calcular `delta_T = T_objeto - T_ambiente` y estimar `dT/dt` con una ventana temporal documentada; marcar la salida como no válida si faltan datos suficientes.
- Comparar las lecturas con un termómetro de referencia disponible, sin asumir que el prototipo cumple especificaciones clínicas; registrar método, condiciones y desviaciones observadas.
- Ejecutar al menos **30 minutos** de adquisición continua y conservar registro, tasa de lecturas válidas, reinicios y errores de comunicación.
- Automatizar pruebas de la lógica independiente del hardware con secuencias normales, faltantes, fuera de rango y discontinuidades de tiempo.

**Salida:** informe breve de reproducibilidad y limitaciones. El ensayo pasa si el sistema no se reinicia durante la captura, no presenta valores silenciosamente inventados y documenta la tasa real de lecturas válidas; cualquier umbral de precisión se fija después de medir el hardware.

### Etapa D: decisión sobre la siguiente iteración

- Evaluar ruido, deriva, consumo, memoria y complejidad real del firmware.
- Decidir si conviene añadir otro dominio físico, mejorar el montaje térmico o pasar a dos placas.
- Si se justifica un segundo nodo, diseñar primero el contrato de mensajes y probar ESP-NOW con secuencia, caducidad de datos, autenticación/cifrado y fallos de enlace. BAS-Link es una propuesta del documento, no una prestación implementada en el MVP.

**Salida:** decisión escrita con evidencia del banco y alcance de la siguiente versión.

## 5. Estructura prevista (se creará al implementar)

```text
README.md             Descripción y estado del proyecto
Plan.md               Este plan
BAS3.md               Base teórica aportada
firmware/BASPrototype/ Sketch Arduino IDE y controladores
firmware/main/        Versión ESP-IDF anterior, conservada como alternativa
tests/                Pruebas de lógica y datos simulados
docs/                 Conexiones, protocolos e informes de banco
```

No crear módulos vacíos ni afirmar instrucciones de compilación antes de escoger la placa y comprobar el entorno.

## 6. Trazabilidad y límites

| Referencia teórica | Qué se explorará en el MVP | Qué queda fuera |
| --- | --- | --- |
| `BAS3.md` §3.7, balance térmico | Captura de temperatura, diferencia con ambiente y tendencia | Estimar calor metabólico o carga biológica sin caracterizar masa, capacidad térmica y pérdidas |
| `BAS3.md` §4, arquitectura BCO | Interfaz de datos modular en una placa | Clúster, autodescubrimiento y fusión de diez dominios |
| `BAS3.md` §6.1, validación | Pruebas unitarias, integración y comparación instrumental de banco | Validación biológica, ensayos de campo, sensibilidad/especificidad |

PINN, puntajes de triaje, cifrado de una red inexistente y metas de costo o rendimiento del artículo **no forman parte del MVP**. Para cualquier fase futura con uso biológico o sanitario se requerirán evaluación científica independiente, protocolo de bioseguridad, revisión ética y cumplimiento regulatorio aplicable antes de probar con personas o muestras humanas.

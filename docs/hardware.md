# Hardware del prototipo térmico

## Componentes para el banco

- Una placa ESP32-S3 con USB.
- Dos sensores digitales TMP117 (uno para el objeto de prueba y otro para el ambiente), en breakout compatible con 3.3 V. El firmware reconoce el dispositivo mediante su registro de identidad antes de leer medidas.
- Cableado corto y conexión USB. No conectar muestras biológicas ni actuadores de calentamiento en esta etapa.

## Conexión I2C

| Señal | ESP32-S3 | TMP117 objeto | TMP117 ambiente |
| --- | --- | --- | --- |
| Alimentación | 3V3 | V+ | V+ |
| Tierra | GND | GND | GND |
| SDA | GPIO configurado | SDA | SDA |
| SCL | GPIO configurado | SCL | SCL |
| ADD0 | — | GND (0x48) | V+ (0x49) |

Las direcciones iniciales son `0x48` y `0x49`; el TMP117 admite direcciones de `0x48` a `0x4b`, siempre distintas. Se configuran en las constantes `kObjectAddress` y `kAmbientAddress` del sketch. Verificar el pinout de la placa y la serigrafía/documentación de cada breakout antes de alimentar. La configuración de ejemplo usa GPIO 8 para SDA y GPIO 9 para SCL; se cambian en `kSdaPin` y `kSclPin` si es necesario. Comprobar que el bus tiene pull-ups I2C adecuados a 3.3 V y evitar duplicar resistencias de muchos breakouts en paralelo.

El sketch muestrea por defecto cada segundo; el periodo se configura en `kSamplePeriodMs`. Un sensor que no responda durante el inicio se informa como `sensor_unavailable`; un fallo durante una lectura se marca como `read_error`, sin reutilizar el último valor.

## Límites

La precisión del sistema completo depende del sensor, montaje, acoplamiento térmico, ubicación, cableado y calibración. Una medición de temperatura no demuestra actividad metabólica. Emplear únicamente objetos de prueba no biológicos y temperaturas ambientalmente seguras.

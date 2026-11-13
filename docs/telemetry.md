# Telemetría serie

El firmware emite una línea JSON por ciclo de muestreo. Cada línea es independiente y usa `schema_version` para identificar el formato.

```json
{
  "schema_version": "1.0",
  "seq": 12,
  "uptime_ms": 12034,
  "mode": "real",
  "object": {"id": "object", "celsius": 24.5, "valid": true, "status": "ok"},
  "ambient": {"id": "ambient", "celsius": 22.0, "valid": true, "status": "ok"},
  "delta_celsius": 2.5,
  "delta_slope_c_per_s": null,
  "trend_valid": false
}
```

- `seq` aumenta por muestra; `uptime_ms` es tiempo monotónico desde el arranque.
- `mode` distingue lecturas `real` de datos `simulated`.
- `celsius` y `delta_celsius` se representan como `null` si no son válidos. Los estados de canal son `ok`, `sensor_unavailable`, `read_error` y `out_of_range`.
- `delta_celsius` es `object - ambient`.
- `delta_slope_c_per_s` es la pendiente de una regresión lineal sobre las últimas diez diferencias válidas (aproximadamente nueve segundos a 1 Hz). Es `null` hasta completar una ventana válida; una lectura inválida o una discontinuidad mayor a dos periodos reinicia la ventana.

La telemetría es información de ingeniería del prototipo; no es un puntaje de salud ni una inferencia de actividad biológica. El sketch Arduino se encuentra en `firmware/BASPrototype/`.

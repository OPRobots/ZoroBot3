# 9. Calibraciones — ZoroBot3

## Índice
1. [Tipos de Calibración](#tipos-de-calibración)
2. [Calibración del Giroscopio](#calibración-del-giroscopio)
3. [Calibración de Sensores Frontales](#calibración-de-sensores-frontales)
4. [Calibración Frontal Media](#calibración-frontal-media)
5. [Calibración de Sensores Laterales](#calibración-de-sensores-laterales)
6. [Persistencia en EEPROM](#persistencia-en-eeprom)
7. [Procedimiento de Calibración Completo](#procedimiento-de-calibración-completo)

---

## Tipos de Calibración

Seleccionables desde el menú CONFIG → CALIBRATION:

| ID | Tipo | Función |
|----|------|---------|
| 0 | `CALIBRATE_NONE` | Sin calibración |
| 1 | `CALIBRATE_GYRO_Z` | Offset Z del giroscopio |
| 2 | `CALIBRATE_SIDE_SENSORS_OFFSET` | Offset de sensores laterales |
| 3 | `CALIBRATE_FRONT_SENSORS` | Calibración frontal de pared |
| 4 | `CALIBRATE_FRONT_SENSORS_MIDDLE` | Distancia media frontal |
| 5 | `CALIBRATE_STORE_EEPROM` | Guardar todo en EEPROM |

Archivos: [`calibrations.c`](../source_code/src/calibrations.c), [`calibrations.h`](../source_code/include/calibrations.h).

### Secuencia de Activación

1. Seleccionar el tipo de calibración en el menú.
2. Pulsar MODE para activar → LED verde se enciende.
3. Pulsar DEBUG para ejecutar → LED verde parpadea, delay 2s.
4. La calibración se ejecuta automáticamente.

---

## Calibración del Giroscopio

`lsm6dsr_gyro_z_calibration()` en [`lsm6dsr.c`](../source_code/src/lsm6dsr.c):

1. Robot **quieto** en superficie plana.
2. Toma **200 muestras** del eje Z.
3. Calcula el offset como el promedio de las lecturas.
4. Calibra para **cada full-scale** (1000, 2000, 4000 dps) independientemente.
5. Guarda offsets en EEPROM.

---

## Calibración de Sensores Frontales

`front_sensors_calibration()` en [`sensors.c`](../source_code/src/sensors.c):

1. Posicionar el robot de cara a una pared a **174 mm** (`CELL_DIMENSION − WALL_WIDTH/2`).
2. Toma `SENSOR_FRONT_CALIBRATION_READINGS = 20` muestras.
3. **Umbral raw**: `threshold = Σ(raw_filter) / 20 / (1.5)²` (fórmula corregida con ley 1/d²).
4. **Offset de distancia**: `offset = 174 mm − promedio(distancia_medida)`.
5. Guarda umbrales y offsets en EEPROM.

---

## Calibración Frontal Media

`front_sensors_middle_calibration()` en [`sensors.c`](../source_code/src/sensors.c):

1. Posicionar el robot en el **centro de la celda** (a `MIDDLE_MAZE_DISTANCE = 84 mm` de la pared).
2. Mide la distancia objetivo media y el valor raw equivalente.
3. Estos valores se usan para el control `keep_front_distance()`.

---

## Calibración de Sensores Laterales

`side_sensors_calibration(keep_sensors_on)` en [`sensors.c`](../source_code/src/sensors.c):

1. Posicionar el robot **centrado entre dos paredes**.
2. Toma `SENSOR_SIDE_CALIBRATION_READINGS = 100` muestras.
3. **Umbral raw**: `threshold = Σ(raw_filter) / 100 / (1.5)²`.
4. **Offset de distancia**: `offset = MIDDLE_MAZE_DISTANCE − promedio(distancia_medida)`.
5. Si `keep_sensors_on = false`, desactiva sensores tras calibrar.
6. Guarda en EEPROM.

Esta calibración se ejecuta automáticamente al iniciar:
- `floodfill_start_explore()` — exploración
- `floodfill_start_run()` — speed run

---

## Persistencia en EEPROM

Los datos de calibración se almacenan en el sector flash 11 con checksum para integridad. Ver [EEPROM](10-eeprom.md).

Al arrancar, `eeprom_load()` carga:
- `sensors_distance_offset[4]` — offsets de distancia
- `sensors_raw_wall_detection_threshold[4]` — umbrales raw
- `sensors_middle_target_distance[4]` — distancias medias objetivo
- `offset_z[3]` — offsets del giroscopio por full-scale

Los offsets se imprimen por puerto serie para verificación.

---

## Procedimiento de Calibración Completo

Para una calibración completa del robot:

1. **Giroscopio**: Robot quieto en superficie plana → `CALIBRATE_GYRO_Z` → `CALIBRATE_STORE_EEPROM`
2. **Sensores frontales**: Robot de cara a pared a 174mm → `CALIBRATE_FRONT_SENSORS` → `CALIBRATE_STORE_EEPROM`
3. **Sensores frontales medio**: Robot en centro de celda → `CALIBRATE_FRONT_SENSORS_MIDDLE` → `CALIBRATE_STORE_EEPROM`
4. **Sensores laterales**: Robot centrado entre dos paredes → `CALIBRATE_SIDE_SENSORS_OFFSET` → `CALIBRATE_STORE_EEPROM`

También disponible: `calibrate_manual_distances()` — Al pulsar DEBUG, imprime los valores raw filter de los 4 sensores por puerto serie para verificación manual.

---

*Documento generado el 2026-06-12. Ver también [Sensores](03-sensors.md), [EEPROM](10-eeprom.md), [Encoders y Giroscopio](11-encoders-gyro.md).*

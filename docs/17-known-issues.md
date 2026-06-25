# Problemas Conocidos

> **Fecha de análisis**: 2026-06-11
> **Última actualización**: 2026-06-12
> **Total**: 47 issues — 10 críticos, 19 moderados, 18 leves

---

## Correcciones Recientes ✅

| ID | Fecha | Descripción |
|----|-------|-------------|
| SS-01 | 2026-06-11 | Desbordamiento de tabla LUT — añadido bounds checking |
| SS-02 | 2026-06-11 | Análisis de timing: margen 97x, fallo improbable |
| SS-03 | 2026-06-11 | Filtro mediana N=3 + EMA adaptativo (α=0.2/0.6, umbral 10mm) |
| SS-07 | 2026-06-11 | Threshold raw corregido con ley 1/d² (factor 1.5× distancia) |
| SS-15 | 2026-06-11 | Checksum aditivo (complemento a 2) en EEPROM + warning_eeprom |

---

## 🔴 Críticos (10 issues)

### Floodfill

#### FF-01 — Estado insuficiente en el BFS para modo TIME_BASED
- **Archivo**: [`floodfill.c:340`](../source_code/src/floodfill.c#L340)
- **Descripción**: El floodfill almacena un único `float` por celda. En TIME_BASED, el coste de transición depende del estado (dirección + count). Un camino con tiempo ligeramente superior pero mejor dirección se descarta.
- **Impacto**: Alto — puede no encontrar la ruta óptima global.
- **Mitigación**: Condición `>=` permite caminos con mismo tiempo pero distinto estado.
- **Solución propuesta**: Ampliar estado a `floodfill[celda][direccion]` o usar A*.

#### FF-02 — Desbordamiento de la cola de prioridad
- **Archivo**: [`floodfill.h:63`](../source_code/include/floodfill.h#L63)
- **Descripción**: Cola con capacidad fija de `MAZE_CELLS`. Cada celda puede encolarse hasta 4 veces. 16×16 = 256 slots, peor caso ~1024 pushes.
- **Impacto**: Alto — corrupción de memoria.
- **Solución propuesta**: Redimensionar a `MAZE_CELLS × 4` o cola circular con bounds checking.

#### FF-03 — Penalización de giro ausente en transición ortogonal→ortogonal
- **Archivo**: [`floodfill.c:515-520`](../source_code/src/floodfill.c#L515-L520)
- **Descripción**: El caso `from_orthogonal && to_orthogonal` no verifica si la dirección cambió.
- **Impacto**: Bajo en práctica (cubierto por otros casos del switch).

### Sensores

#### SS-01 — Desbordamiento de tabla LUT ✅ CORREGIDO
- Añadido bounds checking: `if (ln_index >= LOG_LINEARIZATION_TABLE_SIZE) ln_index = LOG_LINEARIZATION_TABLE_SIZE − 1`.

#### SS-02 — Pérdida de conversiones ADC ✅ CORREGIDO
- Análisis de timing confirmó margen 97x. Sin riesgo real.

#### SS-03 — Sin filtrado de lecturas ADC ✅ CORREGIDO
- Añadido: mediana N=3 + EMA adaptativo.

### Movimiento

#### MV-01 — PID sin anti-windup (6 integradores)
- **Archivo**: [`control.c:392-451`](../source_code/src/control.c#L392-L451)
- **Descripción**: Los 6 integradores acumulan error sin clamping. Tras saturación, overshoot masivo.
- **Impacto**: Alto — inestabilidad severa.
- **Solución propuesta**: Implementar clamping condicional (back-calculation o conditional integration).

#### MV-02 — `move_inplace_turn()` siempre gira a la izquierda
- **Archivo**: [`move.c:126-143`](../source_code/src/move.c#L126-L143)
- **Descripción**: MOVE_BACK, MOVE_BACK_WALL y MOVE_BACK_STOP tienen `sign = −1` fijo.
- **Impacto**: Medio-Alto — desviación sistemática en exploración.

#### MV-03 — `lsm6dsr_read_gyro_z_raw()` depende de auto-incremento SPI no verificado
- **Archivo**: [`lsm6dsr.c:66-73`](../source_code/src/lsm6dsr.c#L66-L73)
- **Descripción**: Asume que IF_INC está habilitado para leer ZL+ZH consecutivos.
- **Impacto**: Medio — datos incorrectos si IF_INC no está configurado.

#### MV-04 — `platform_write()` y `platform_read()` ignoran parámetro `len`
- **Archivo**: [`lsm6dsr.c:48-59`](../source_code/src/lsm6dsr.c#L48-L59)
- **Descripción**: Solo transfieren 1 byte independientemente del parámetro `len`.
- **Impacto**: Medio — frágil ante cambios de la librería LSM6DSR.

---

## 🟡 Moderados (19 issues)

### Floodfill
| ID | Descripción | Impacto |
|----|-------------|---------|
| FF-04 | `total_time` incorrecto en tabla de pesos (primera celda contada 2×) | Bajo |
| FF-05 | `mmsim_get_estimated_time()` usa índice incorrecto (off-by-one) | Medio |
| FF-06 | Modelo de penalización de giro inexacto (asume deceleración completa) | Medio |
| FF-07 | Floodfill no considera orientación inicial del robot al planificar | Medio |

### Sensores
| ID | Descripción | Estado |
|----|-------------|--------|
| SS-04 | Fórmula con posible división por ~0 | NO APLICA |
| SS-05 | Parámetro c negativo (mitigado) | NO APLICA |
| SS-06 | Filtro EMA sustituido por adaptativo en SS-03 | NO APLICA |
| SS-07 | Umbral raw corregido con ley 1/d² | ✅ CORREGIDO |
| SS-08 | Crosstalk óptico potencial | NO APLICA |
| SS-09 | Umbrales cableados en `get_side_sensors_error()` | NO APLICA |
| SS-10 | Fórmula no validada contra datasheet (validada empíricamente) | NO APLICA |
| SS-15 | Sin validación de datos EEPROM | ✅ CORREGIDO |

### Movimiento
| ID | Descripción | Impacto |
|----|-------------|---------|
| MV-05 | Distancia de frenada usa velocidad ideal, no real | Medio |
| MV-06 | Velocidad angular con filtros independientes (desfase entre canales) | Bajo-Medio |
| MV-07 | PI con 4 decimales (error 0.003%) | Bajo |
| MV-08 | Fórmulas `move_inplace_turn()` no cinemáticamente exactas | Medio |
| MV-09 | Degradación de giro solo hacia abajo | Medio |
| MV-10 | Posible inconsistencia de signos en control angular | Alto potencial |
| MV-11 | `move_straight()` negativo no comprueba wall loss | Bajo |

---

## 🟢 Leves (18 issues)

### Floodfill
| ID | Descripción |
|----|-------------|
| FF-08 | Código inalcanzable tras `return` en switches |
| FF-09 | `#pragma GCC diagnostic` ignora `-Wswitch` |
| FF-10 | `time_penalty` retorna 0 si `init_speed >= speed` |
| FF-11 | `get_direction_value()` devuelve 0 para diagonales |
| FF-12 | Sesgo direccional en `floodfill_run()` (NORTH siempre gana empates) |
| FF-13 | `cells_to_max_speed` puede exceder `FLOODFILL_MAX_WEIGHTS_COUNT` |

### Sensores
| ID | Descripción | Estado |
|----|-------------|--------|
| SS-11 | `update_side_sensors_leds()` con código repetitivo | NO APLICA |
| SS-12 | `all_sensors_take_values()` bucle infinito (debug) | NO APLICA |
| SS-13 | Flag `sensors_taking_values` nunca a false | NO APLICA |
| SS-14 | División entera en calibración (error ±1 sobre ~2000) | NO APLICA |

### Movimiento
| ID | Descripción |
|----|-------------|
| MV-12 | `avg_micrometers`/`avg_millimeters` nunca actualizadas (código muerto) |
| MV-13 | `volatile` innecesario en variables de control PID |
| MV-14 | Expresión redundante `true &&` en `move_arc_turn()` |
| MV-15 | Número mágico `1.75f` en `move_back()` |
| MV-16 | Parámetros `start`/`end` negativos sin validación de rango |
| MV-17 | `check_wall_loss_correction()` solo paredes laterales |
| MV-18 | Frecuencia acoplada rígidamente a SysTick = 16 kHz |
| MV-19 | Wrap-around de timer de encoders (verificado correcto ✅) |

---

## Resumen por Prioridades

| Prioridad | Cantidad | Issues |
|-----------|:--------:|--------|
| 🔴 Críticos | 10 | FF-01..03, SS-01..03, MV-01..04 |
| 🟡 Moderados | 19 | FF-04..07, SS-04..10/15, MV-05..11 |
| 🟢 Leves | 18 | FF-08..13, SS-11..14, MV-12..19 |
| **TOTAL** | **47** | |

---

*Documento generado el 2026-06-12. Este documento es un resumen del registro completo en [BUGS.md](../BUGS.md) en la raíz del proyecto. Para detalles completos de cada issue, consultar el fichero original.*

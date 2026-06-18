# Encoders y Giroscopio

## Índice
1. [Encoders AS5145B-HSST](#encoders-as5145b-hsst)
2. [Lectura y Conversión](#lectura-y-conversión)
3. [Cálculo de Velocidad](#cálculo-de-velocidad)
4. [Giroscopio LSM6DSR](#giroscopio-lsm6dsr)
5. [Calibración y Offset](#calibración-y-offset)
6. [Integración Angular](#integración-angular)

---

## Encoders AS5145B-HSST

### Hardware

| Rueda | Timer | Función |
|-------|-------|---------|
| Izquierda | TIM4 | `read_encoder_left()` |
| Derecha | TIM3 | `read_encoder_right()` |

- **Resolución**: 12 bits (4096 posiciones/revolución)
- **Interfaz**: Timer STM32 en modo contador (cuadratura por hardware)
- **Relación**: `MICROMETERS_PER_TICK = 10.494055 μm/tick`

### Conversión Física

```
μm/tick = (π × diámetro_rueda) / 4096

distancia_μm = ticks × 10.494055
distancia_mm = distancia_μm / 1000
```

La rueda izquierda tiene el signo **invertido** por montaje mecánico inverso.

### Variables de Estado

```c
static volatile int32_t left_diff_ticks, right_diff_ticks;
static volatile int32_t left_total_ticks, right_total_ticks;
static volatile int32_t left_micrometers, right_micrometers;
static volatile int32_t left_millimeters, right_millimeters;
static volatile float left_speed, right_speed, angular_speed;
```

---

## Lectura y Conversión

`update_encoder_readings()` en [`encoders.c:161-193`](../source_code/src/encoders.c#L161-L193), ejecutada a **1 kHz**:

### 1. Lectura cruda
```c
uint16_t now_left  = timer_get_counter(TIM4);  // 0-65535
uint16_t now_right = timer_get_counter(TIM3);
```

### 2. Diferencia con `max_likelihood_counter_diff()`

```c
int32_t max_likelihood_counter_diff(uint16_t now, uint16_t before) {
    uint16_t forward_diff  = (uint16_t)(now − before);
    uint16_t backward_diff = (uint16_t)(before − now);
    return (forward_diff > backward_diff)
        ? −(int32_t)backward_diff
        : (int32_t)forward_diff;
}
```

**Principio**: Si `forward_diff < backward_diff`, el contador avanzó hacia adelante; si no, hacia atrás. Asume que entre lecturas el contador no avanza más de 32767 ticks (mitad del rango 16-bit).

**Verificación**: A velocidad máxima (~6500 mm/s), ticks por ms = `6500 / (10.494/1000) / 1000 ≈ 619`. Mucho menor que 32767. ✅

### 3. Signo
```c
left_diff_ticks = −max_likelihood_counter_diff(now_left, before_left);  // INVERTIDO
right_diff_ticks = max_likelihood_counter_diff(now_right, before_right);
```

### 4. Acumulación
```c
left_total_ticks  += left_diff_ticks;
right_total_ticks += right_diff_ticks;
left_micrometers   = left_total_ticks * MICROMETERS_PER_TICK;
```

---

## Cálculo de Velocidad

### Velocidad Lineal (mm/s)

```c
new_left_speed = left_diff_ticks * (MICROMETERS_PER_TICK / 1000) * UPDATE_FREQUENCY_HZ;
left_speed = 0.8 × left_speed + 0.2 × new_left_speed;  // EMA α=0.2, τ≈5ms
```

- Filtro EMA con α = 0.2 (constante de tiempo ≈ 5 ms).
- `measured_linear_speed = (left_speed + right_speed) / 2`.

### Velocidad Angular desde Encoders

```c
angular_speed = (left_speed − right_speed) / WHEELS_SEPARATION;  // rad/s
```

Donde `WHEELS_SEPARATION = 62 mm`. Se usa solo para detección de saturación angular.

⚠️ `left_speed` y `right_speed` tienen filtros EMA independientes, lo que introduce desfase entre canales. Ver [MV-06](15-known-issues.md#mv-06).

---

## Giroscopio LSM6DSR

### Configuración

| Parámetro | Valor |
|-----------|-------|
| Interfaz | SPI3 (PA15 = CS) |
| ODR | 1666 Hz |
| Filtro paso-bajo | LIGHT (BW ≈ 100 Hz en 1000 dps) |
| Full-scale | Variable: 1000/2000/4000 dps |
| Block Data Update | Habilitado |
| Driver | STMems (C drivers oficiales de ST) |

### Full-Scale Dinámico

| Estrategia | Full-Scale | Sensibilidad |
|-----------|:---:|:---:|
| SPEED_EXPLORE | 1000 dps | 35 mdps/LSB |
| SPEED_NORMAL → HAKI | 2000 dps | 70 mdps/LSB |

Cuando la cinemática cambia, `lsm6dsr_reload_config()` reconfigura el full-scale vía SPI y resetea los acumuladores de error.

### Lectura (`lsm6dsr_update()` @ 1 kHz)

1. **Lectura cruda**: `spi_xfer(SPI3, MPU_READ | OUTZ_L_G)` → ZL + ZH (2 bytes consecutivos, asume auto-incremento SPI).
2. **Offset**: `raw − offset_z[full_scale]`.
3. **Filtro paso-bajo**: `gyro_z_raw = α × old + (1−α) × new` (α configurable por cinemática).
4. **Conversión**: `gyro_z_dps = raw × sensitivity / 1000`.
5. **Integración**: `deg_integ −= gyro_z_dps / 1000` (grados acumulados).

### Signo para Control PID

```c
static float get_measured_angular_speed(void) {
    return −lsm6dsr_get_gyro_z_radps();  // ← SIGNO NEGADO
}
```

---

## Calibración y Offset

`lsm6dsr_gyro_z_calibration()`:
1. Robot quieto.
2. 200 muestras por cada full-scale (1000, 2000, 4000 dps).
3. Offset = promedio de las lecturas.
4. Guardado en EEPROM.

Los offsets se cargan al arrancar mediante `lsm6dsr_load_eeprom()`.

---

## Integración Angular

```c
// En lsm6dsr_update() a 1 kHz:
deg_integ −= gyro_z_dps / UPDATE_FREQUENCY_HZ;  // 1000 Hz
```

El ángulo acumulado se usa para:
- `move_inplace_angle()` — giros a ángulo absoluto
- `get_front_wall_angle()` — ángulo respecto a pared frontal
- `keep_z_angle()` — estabilización de heading

---

*Documento generado el 2026-06-12. Ver también [Movimiento](04-movement.md), [Control PID](06-control-system.md), [Calibración](09-calibration.md).*

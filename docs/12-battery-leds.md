# 12. Batería y LEDs — ZoroBot3

## Índice
1. [Monitorización de Batería](#monitorización-de-batería)
2. [LEDs de Estado e Información](#leds-de-estado-e-información)
3. [LED RGB](#led-rgb)
4. [Control de Ventilador](#control-de-ventilador)

---

## Monitorización de Batería

### Hardware

- **Batería**: LiPo 2S 260 mAh 35-70C (Turnigy nano-tech)
- **Soporte 3S**: Configurable
- **ADC**: Canal auxiliar `AUX_BATTERY_ID` (PA4, ADC_CHANNEL4)
- **Divisor de tensión**: Adapta el voltaje de batería al rango 0-3.3V del ADC

### Factores de División

| Batería | Factor | Rango |
|---------|--------|-------|
| 2S | `VOLT_DIV_FACTOR_2S = 3.15` | 7.4V − 8.4V |
| 3S | `VOLT_DIV_FACTOR_3S = 5.30` | 11.0V − 12.6V |

### Umbrales

| Batería | Alto (lleno) | Bajo (vacío) |
|---------|:----------:|:----------:|
| 2S | 8.4V | 7.4V |
| 3S | 12.6V | 11.0V |

### Lectura

`update_battery_voltage()` en [`battery.c`](../source_code/src/battery.c), ejecutada a 1 kHz:

```c
// Lectura ADC → voltaje
float voltage = adc_raw × ADC_LSB × volt_div_factor;

// Filtro paso-bajo
battery_voltage = α × voltage + (1−α) × battery_voltage;
// α = BATTERY_VOLTAGE_LOW_PASS_FILTER_ALPHA = 0.1
```

### Funciones

| Función | Descripción |
|---------|-------------|
| `get_battery_voltage()` | Voltaje actual filtrado |
| `is_battery_2s()` | Detecta si es 2S o 3S |
| `show_battery_level()` | Muestra nivel en barra de LEDs |
| `get_battery_high_limit_voltage()` | Voltaje máximo según tipo |

---

## LEDs de Estado e Información

### LED de Estado

- **GPIO**: PB13
- **Funciones**: `set_status_led(on/off)`, `toggle_status_led()`, `warning_status_led(period_ms)`
- Indica estado general, parpadea en calibración, warning en error de EEPROM

### LEDs de Información (10 unidades)

Control mediante `enum info_led` (0-9):
- `set_info_led(n)` — Encender LED específico
- `set_info_leds()` — Encender todos
- `clear_info_leds()` — Apagar todos

Funciones de visualización:
| Función | Descripción |
|---------|-------------|
| `set_leds_wave(ms)` | Efecto de onda (KITT) |
| `set_leds_side_sensors()` | Error de sensores laterales en barra |
| `set_leds_front_sensors()` | Estado de sensores frontales |
| `set_leds_front_sensors_middle()` | Sensores frontales en posición media |
| `set_leds_blink(ms)` | Parpadeo de todos los LEDs a un intervalo determinado |
| `set_leds_battery_level()` | Nivel de batería en leds |
| `show_robot_version()` | Muestra versión (LEDs A, B, C) |
| `warning_eeprom()` | Aviso de EEPROM corrupta |

---

## LED RGB

- **GPIO**: PB14 (Timer 12, canales 1-3)
- **Resolución**: 1024 niveles por canal (10 bits)
- **Funciones**:
  - `set_RGB_color(r, g, b)` — Color fijo (0-1024 por canal)
  - `set_RGB_rainbow()` — Ciclo automático de colores
  - `blink_RGB_color(r, g, b, ms)` — Parpadeo de color

### Uso en Errores

| Color | Significado |
|-------|-------------|
| Rojo (parpadeo) | Saturación de PWM |
| Azul (parpadeo) | Saturación de velocidad angular |
| Verde | Calibración activa |

---

## Control de Ventilador

El ventilador de succión proporciona downforce para mejorar la tracción en curvas:

- **GPIO**: PB10 (PWM)
- **Control**: PID con rampa de aceleración (`set_target_fan_speed(speed, ramp_ms)`)
- **Velocidad**: Configurable por estrategia y tipo de batería
  - `kinematics.fan_speed_2s` — Ventilador con batería 2S
  - `kinematics.fan_speed_3s` — Ventilador con batería 3S

El ventilador se activa automáticamente al iniciar exploración o speed run, y se desactiva en parada de emergencia.

---

*Documento generado el 2026-06-12. Ver también [Debug](08-debug-system.md), [Control PID](06-control-system.md), [Menú](07-menu-system.md).*

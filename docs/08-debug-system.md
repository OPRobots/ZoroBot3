# Sistema de Debug

## Índice
1. [Modos de Debug](#modos-de-debug)
2. [Salida Serie (USART)](#salida-serie-usart)
3. [LEDs de Información](#leds-de-información)
4. [Macroarray](#macroarray)
5. [Zona de Debug en Main](#zona-de-debug-en-main)

---

## Modos de Debug

11 modos seleccionables desde el menú CONFIG → DEBUG. Archivos: [`debug.c`](../source_code/src/debug.c), [`debug.h`](../source_code/include/debug.h).

### 0. `DEBUG_NONE` — Sin debug
Operación normal.

### 1. `DEBUG_MACROARRAY` — Datos de macroarray
Imprime los datos almacenados en el buffer de macroarray por puerto serie. Útil para analizar datos de control post-carrera.

### 2. `DEBUG_TYPE_SENSORS` — Sensores en vivo
- **Toggle con UP**: alterna entre modo raw y distancias.
- Modo raw: imprime `sensors_on`, `sensors_off`, `raw_filter` para los 4 sensores.
- Modo distancias: imprime distancias en mm, detección de paredes, errores laterales/diagonales/angulares.
- Frecuencia: ~50 ms entre lecturas.

### 3. `DEBUG_ENCODERS` — Encoders
Imprime milímetros del encoder izquierdo y derecho a ~50 ms.

### 4. `DEBUG_GYRO` — Giroscopio
Imprime valor raw, ángulo integrado y velocidad angular del giroscopio Z.

### 5. `DEBUG_FLOODFILL_MAZE` — Maze + Floodfill
Llama a `floodfill_maze_print()` que imprime:
- Laberinto completo con paredes (formato ASCII)
- Valores de floodfill por celda
- Secuencia de movimientos generada
Desactiva el debug tras la impresión.

### 6. `DEBUG_MOTORS_CURRENT` — Corriente de motores
- Activa motores a velocidad 150.
- Imprime valores de encoders.
- Permite medir consumo con los motores en movimiento.

### 7. `DEBUG_TIMETRIAL` — Demo Time Trial
Ejecuta una demo: 2 celdas recto + 3× (1 celda + giro 90° derecha).
Usa la configuración cinemática actual.

### 8. `DEBUG_KEEP_FRONT_DISTANCE` — Demo distancia frontal
- Activa sensores frontales.
- Configura control para mantener distancia a pared frontal.
- Prueba el PID de distancia frontal.

### 9. `DEBUG_GYRO_DEMO` — Demo estabilización giro
Ejecuta `keep_z_angle()` en bucle para probar la estabilización del giroscopio.

### 10. `DEBUG_FAN_DEMO` — Demo ventilador
Activa el ventilador a la velocidad configurada para la cinemática actual.

---

## Salida Serie (USART)

USART3 a 115200 baud:
- **TX**: PC6
- **RX**: PC7
- `printf()` redirigido mediante `_write()` en [`usart.c`](../source_code/src/usart.c).

### Formato de Impresión

```
// Sensores raw
"S1: 1234 S2: 2345 S3: 3456 S4: 0123"

// Sensores distancia
"D1: 180 D2: 175 W: L01 R01 F00 | err: +1.5 dErr: -0.3 aErr: +2"

// Aux analógicos
"BA: 3456 CI: 0123 CD: 2345 BO: 1234"

// Batería
"BA: 8.23V"

// Encoders
"L: 12345 R: 12346"

// Giroscopio
"Z(raw): -123 Z(dps): -4.305 Z(deg): 12.34"
```

---

## LEDs de Información

10 LEDs de información + 1 LED de estado + 1 LED RGB. Ver [Batería y LEDs](12-battery-leds.md) para detalles completos.

Funciones de visualización para debug:
- `update_side_sensors_leds()` — Muestra error de sensores laterales en barra de 8 LEDs.
- `set_leds_front_sensors()` — Muestra estado de sensores frontales.
- `set_info_led()` / `set_info_leds()` / `clear_info_leds()` — Control individual de LEDs.
- `set_RGB_color(r,g,b)` — LED RGB (hasta 1024 niveles por canal).
- `set_RGB_rainbow()` — Ciclo de colores automático.

---

## Macroarray

Sistema de logging para depuración post-carrera ([`macroarray.h`](../source_code/include/macroarray.h)):

```c
#define MACROARRAY_LENGTH 30000
```

- `macroarray_store(data)` — Almacena datos de control durante la carrera.
- `macroarray_print()` — Imprime el buffer completo por USART.
- Activado cuando `race_started` y hay movimiento no nulo.

Los datos almacenados incluyen errores de control, velocidades y PWM para análisis offline.

---

## Zona de Debug en Main

Al final de [`main.c:158-202`](../source_code/src/main.c#L158-L202) hay una **zona de debug temporal** con código comentado para pruebas rápidas:

```c
// LED WARNING     → warning_status_led(125)
// LEDS MENU       → set_leds_wave(125)
// LED RGB         → set_RGB_color(r,g,b) / set_RGB_rainbow()
// MPU             → lsm6dsr_get_gyro_z_*
// VENTILADOR      → set_fan_speed(50)
// MOTORES         → set_motors_speed(20, 20)
// SENSORES        → get_sensor_raw(...)
// AUX ANALÓGICOS  → get_aux_raw(...)
// ENCODERS        → get_encoder_left_millimeters()
```

Esta zona es inalcanzable en operación normal (está después de un `while(1)` infinito), pero sirve como referencia de pruebas unitarias.

---

*Documento generado el 2026-06-12. Ver también [Menú](07-menu-system.md), [Batería y LEDs](12-battery-leds.md), [Arquitectura Software](02-software-architecture.md).*

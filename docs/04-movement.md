# Sistema de Movimiento


## Arquitectura General

### Bucle de ejecución

| Nivel | Contexto | Frecuencia | Responsabilidad |
|-------|----------|------------|-----------------|
| **ISR (SysTick)** | Interrupción | 1 kHz | Control PID en tiempo real |
| **Main Loop** | Bucle principal | Variable | Funciones bloqueantes de movimiento |

El main loop llama a funciones bloqueantes (`move_straight`, `move_arc_turn`, etc.) que establecen velocidades objetivo y esperan condiciones de finalización. El ISR ejecuta el control PID en segundo plano.

```
Main Loop                         ISR (1 kHz)
    │                                  │
    ├─ move_straight(d, speed) ──►     │
    │  ├─ set_target_linear_speed()    ├─ control_loop()
    │  └─ while(dist < target)         │  ├─ update_ideal_linear_speed()
    │     │  (espera ocupada)          │  ├─ PID lineal (error velocidad)
    │     │                            │  ├─ PID angular (error giro)
    │     │                            │  ├─ PID sensores (error paredes)
    │     │                            │  └─ set_motors_pwm()
    │     └─ (distancia alcanzada)     │
    └──────────────────────────────────┘
```

Archivos: [`move.h`](../source_code/include/move.h), [`move.c`](../source_code/src/move.c) (~60 KB, el archivo más grande del proyecto).

---

## Tipos de Movimiento

26 tipos definidos en `enum movement` ([`move.h`](../source_code/include/move.h)):

### Especiales
| Movimiento | Descripción |
|-----------|-------------|
| `MOVE_NONE` | No operación |
| `MOVE_HOME` | Secuencia de vuelta al inicio |
| `MOVE_START` | Primer movimiento desde la salida |
| `MOVE_END` | Secuencia de fin de carrera |
| `MOVE_DIAGONAL` | Recta en diagonal (solo en run sequences) |

### Giros en Arco (Exploración)
| Movimiento | Descripción |
|-----------|-------------|
| `MOVE_LEFT` / `MOVE_RIGHT` | Giro 90° en arco (exploración) |

### Giros en Arco (Carrera)
| Movimiento | Descripción |
|-----------|-------------|
| `MOVE_LEFT_90` / `MOVE_RIGHT_90` | Giro 90° |
| `MOVE_LEFT_180` / `MOVE_RIGHT_180` | Giro 180° (U-turn) |
| `MOVE_LEFT_TO_45` / `MOVE_RIGHT_TO_45` | Giro terminando a 45° |
| `MOVE_LEFT_TO_135` / `MOVE_RIGHT_TO_135` | Giro terminando a 135° |
| `MOVE_LEFT_45_TO_45` / `MOVE_RIGHT_45_TO_45` | Curva S entre paredes a 45° |
| `MOVE_LEFT_FROM_45` / `MOVE_RIGHT_FROM_45` | Giro desde pared a 45° |
| `MOVE_LEFT_FROM_45_180` / `MOVE_RIGHT_FROM_45_180` | 180° desde pared a 45° |

### Rectos y Giros en Sitio
| Movimiento | Descripción |
|-----------|-------------|
| `MOVE_FRONT` | Avanzar una celda |
| `MOVE_BACK` | Girar 180° en sitio, avanzar una celda |
| `MOVE_LEFT_INPLACE` / `MOVE_RIGHT_INPLACE` | Giro 90° en sitio |
| `MOVE_BACK_WALL` | Como BACK pero contra una pared |
| `MOVE_BACK_STOP` | Como BACK pero sin avanzar celda |

---

## Movimientos Lineales

### `move_straight(distance, speed, check_wall_loss, stop)` ([`move.c:1339-1378`](../source_code/src/move.c#L1339-L1378))

Movimiento recto bloqueante de bajo nivel:

1. Establecer velocidad angular = 0.
2. Establecer velocidad objetivo = `speed`.
3. Mientras `distancia_recorrida < distance`:
   - Si `check_wall_loss`: detectar pérdida de pared y reajustar distancia.
   - Si `stop`: calcular distancia de frenada necesaria.
4. Si `stop`: esperar a que velocidad ideal llegue a 0.

- **Stop distance**: calculada con `calc_straight_to_speed_distance(ideal_speed, 0)` usando `break_accel`.
- ⚠️ Usa velocidad ideal en lugar de velocidad real (ver [MV-05](15-known-issues.md#mv-05)).

### `move_straight_until_front_distance(distance, speed, stop)` ([`move.c:1387-1415`](../source_code/src/move.c#L1387-L1415))

Avanza/retrocede hasta que el sensor frontal detecta la distancia especificada. La dirección se determina automáticamente.

### `run_straight(distance, start_offset, end_offset, cells, has_begin, speed, final_speed, next_turn_sign)` ([`move.c:1492-1637`](../source_code/src/move.c#L1492-L1637))

Movimiento recto multi-celda para speed run:

1. Activar corrección de sensores laterales.
2. Para cada celda:
   - Avanzar `CELL_DIMENSION` + offsets.
   - Detectar cambio de celda por distancia recorrida.
   - Comprobar pérdida de pared (wall loss).
   - Última celda: verificar pared del lado del próximo giro.
   - Calcular distancia de frenada si `final_speed != speed`.
3. Al detectar pérdida de pared: reajustar distancia.

### `run_diagonal(distance, end_offset, cells, speed, final_speed)` ([`move.c:1727-1768`](../source_code/src/move.c#L1727-L1768))

Similar a `run_straight` pero para diagonales:
- Longitud de celda: `CELL_DIAGONAL` (127.3 mm)
- Corrección diagonal de sensores frontales (>1 celda)
- Sin corrección diagonal en la última celda (<127.3 mm del final)

---

## Movimientos Angulares

### `move_arc_turn(turn)` ([`move.c:1776-1803`](../source_code/src/move.c#L1776-L1803))

Giro en arco con perfil de velocidad **sinusoidal** (jerk limitado):

```
Fase 1 (entrada):       ω(t) = ω_max × sin(t/T × π/2)    [aceleración suave]
Fase 2 (arco):          ω(t) = ω_max                      [velocidad constante]
Fase 3 (salida):        ω(t) = ω_max × sin((t−T_arc)/T × π/2) [deceleración suave]
```

Donde `T = transition` y el arco total = `2 × transition + arc`.

La variable independiente es la **distancia lineal recorrida** (no el tiempo), lo que hace el perfil independiente de la velocidad lineal.

```c
factor = travelled / turn.transition;         // 0 → 1 en fase de entrada
angular_speed = sign × max × sin(factor × π/2);
```

### `move_inplace_turn(movement)` ([`move.c:1805-1842`](../source_code/src/move.c#L1805-L1842))

Giro en el sitio (velocidad lineal = 0) con perfil sinusoidal **temporal**.

⚠️ La dirección está fijada a **izquierda** (−1) para MOVE_BACK, MOVE_BACK_WALL y MOVE_BACK_STOP. Ver [MV-02](15-known-issues.md#mv-02).

### `move_inplace_angle(angle, rads)` ([`move.c:1850-1872`](../source_code/src/move.c#L1850-L1872))

Giro a velocidad angular constante hasta alcanzar un ángulo absoluto usando el integrador del giroscopio.

---

## Orquestración de Speed Run

### `move_run_sequence(sequence_movements)` ([`move.c:1920-2067`](../source_code/src/move.c#L1920-L2067))

Convierte la secuencia de movimientos del floodfill en comandos físicos:

```
Para cada movimiento en la secuencia:
├── MOVE_START / MOVE_FRONT / MOVE_DIAGONAL
│   └── Acumular distancia (straight_cells++)
├── MOVE_HOME
│   ├── Ejecutar tramo recto/diagonal acumulado
│   └── Ejecutar MOVE_HOME
└── MOVE_LEFT_* / MOVE_RIGHT_*
    ├── Verificar si hay suficiente distancia para decelerar
    │   └── Si no: DEGRADAR velocidad de giro
    ├── Ejecutar tramo recto/diagonal acumulado (con end_offset del giro)
    └── Ejecutar giro
```

### Acumulación de Tramos Rectos

Los tramos rectos consecutivos se acumulan en un solo `run_straight` multi-celda:

```
secuencia: F F F R F F S
           └─┬─┘   └─┬─┘
             3 celdas  2 celdas
```

Esto minimiza transiciones y mantiene velocidades altas en rectas largas.

### Degradación de Velocidad de Giro

Si no hay suficiente distancia para decelerar hasta la velocidad del giro, se degrada la estrategia:

```c
while (distance + end_offset < calc_straight_to_speed_distance(ideal_speed, turn.linear_speed)) {
    if (speed_strategy <= SPEED_NORMAL) {
        turn = kinematics_settings[SPEED_NORMAL].turns[...];  // mínimo
        break;
    }
    turn = kinematics_settings[--speed_strategy].turns[...];   // bajar un nivel
}
```

La degradación es **solo hacia abajo**. Si sobra distancia, no se intenta un giro más rápido (ver [MV-09](15-known-issues.md#mv-09)).

---

## Corrección por Pérdida de Pared

`check_wall_loss_correction(initial_walls)` ([`move.c:942-978`](../source_code/src/move.c#L942-L978)):

1. Compara el estado de paredes al entrar en la celda contra el estado actual.
2. Si una pared lateral que existía desaparece: activa flag `current_cell_wall_lost`.
3. Reajusta la distancia restante a `WALL_LOSS_TO_SENSING_POINT_DISTANCE` (116 mm).

⚠️ Solo detecta pérdida de paredes laterales, no frontales (ver [MV-17](15-known-issues.md#mv-17)).

---

## Detección de Saturación

### Saturación de PWM

```c
if (abs(pwm) >= MOTORES_SATURATION_PWM)   // 1000 de 1024
    saturation_count++;
```
100 lecturas consecutivas (100 ms @ 1 kHz) → parada de emergencia.

### Saturación de Velocidad Angular

```c
if (abs(encoder_angular_speed) >= 50)  // rad/s
    angular_saturation_count++;
```
60 lecturas consecutivas (60 ms) → parada de emergencia.

### Acción ante Saturación

1. Motores → 0 (parada de emergencia).
2. Ventilador → 0.
3. LED RGB parpadea (rojo = PWM, azul = angular).
4. Si pasan 3 segundos: se detiene la carrera.

Durante `MOVE_BACK_WALL`, la comprobación se deshabilita para evitar falsos positivos al empujar contra una pared.

---

## Funciones del gestor de movimientos Principal

### `move(movement)` — Movimientos de alto nivel

```c
MOVE_HOME              → move_home()
MOVE_START / MOVE_FRONT → move_front()
MOVE_END               → move_end()
MOVE_LEFT* / MOVE_RIGHT* → move_side(movement)
MOVE_BACK*             → move_back(movement)
```

### `move_front()`

```c
move_straight(CELL_DIMENSION − SENSING_POINT_DISTANCE − current_cell_start_mm, ...)
enter_next_cell()
```

### `move_side(movement)`

1. Ejecuta tramo recto pre-giro (distancia = `turn.start`).
2. Ejecuta `move_arc_turn(turn)`.
3. Ejecuta tramo recto post-giro (distancia = `turn.end`).
4. Deshabilita correcciones de sensores durante el giro.

---

## Problemas Conocidos

Los problemas del sistema de movimiento están documentados en el [registro de issues](15-known-issues.md) con IDs MV-01 a MV-19. Los más relevantes:

- **MV-01**: PID sin anti-windup (6 integradores) — 🔴 Crítico
- **MV-02**: `move_inplace_turn()` siempre gira a la izquierda — 🔴 Crítico
- **MV-05**: Distancia de frenada usa velocidad ideal, no real — 🟡 Moderado
- **MV-08**: Fórmulas `move_inplace_turn()` no cinemáticamente exactas — 🟡 Moderado
- **MV-10**: Posible inconsistencia de signos en control angular — 🟡 Moderado

---

## Diagrama de Flujo

```
┌─────────────────────────────────────────┐
│        Main Loop (bloqueante)           │
│                                         │
│  move(movement)                         │
│    ├─ move_front()                      │
│    │   └─ move_straight(180mm, speed)   │
│    │       ├─ set_target_linear_speed() │
│    │       └─ while(dist < target)      │
│    │           └─ (ISR ejecuta PID)     │
│    ├─ move_side(movement)               │
│    │   ├─ move_straight(turn.start)     │
│    │   ├─ move_arc_turn(turn)           │
│    │   │   └─ Perfil sinusoidal         │
│    │   └─ move_straight(turn.end)       │
│    └─ move_back(movement)               │
│        └─ move_inplace_turn(180°)       │
│                                         │
│  move_run_sequence() [speed run]        │
│    ├─ Acumula tramos rectos             │
│    ├─ Ajusta velocidad de giro          │
│    ├─ run_straight(d, offsets, cells)   │
│    ├─ run_side(movement, turn, next)    │
│    └─ run_diagonal(d, end, cells)       │
└─────────────────────────────────────────┘
```

---

*Documento generado el 2026-06-12. Ver también [Cinemática](14-kinematics.md), [Control PID](06-control-system.md), [Encoders y Giroscopio](11-encoders-gyro.md).*

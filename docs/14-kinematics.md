# 14. Cinemática y Estrategias de Velocidad — ZoroBot3

## Índice
1. [Estructuras de Datos](#estructuras-de-datos)
2. [Estrategias de Velocidad](#estrategias-de-velocidad)
3. [Parámetros de Giro](#parámetros-de-giro)
4. [Perfiles de Aceleración](#perfiles-de-aceleración)
5. [Degradación de Velocidad](#degradación-de-velocidad)

---

## Estructuras de Datos

Archivo: [`move.h`](../source_code/include/move.h)

### `struct kinematics` — Configuración cinemática activa

```c
struct kinematics {
    int16_t linear_speed;                  // Velocidad lineal por defecto (mm/s)
    struct linear_accel_params linear_accel; // Aceleraciones
    int16_t fan_speed_2s;                  // Ventilador en 2S
    int16_t fan_speed_3s;                  // Ventilador en 3S
    struct turn_params *turns;             // Array de parámetros de giro
    struct kpi_params *kpi;                // Ganancias PID
    struct mpu_params mpu;                 // Config del giroscopio
};
```

### `struct turn_params` — Parámetros de un giro

```c
struct turn_params {
    float start;              // Distancia desde el sensing point de entrada (mm).
                              //   Negativo → el perfil empieza antes del sensing point
                              //              (reduce el tramo recto previo).
                              //   Positivo → el perfil empieza después del sensing point
                              //              (alarga el tramo recto previo).
    float end;                // Distancia desde el sensing point de salida (mm).
                              //   Negativo → el perfil termina después del sensing point
                              //              (reduce el tramo recto posterior).
                              //   Positivo → el perfil termina antes del sensing point
                              //              (alarga el tramo recto posterior).
    int16_t linear_speed;     // Velocidad lineal durante el giro (mm/s)
    float max_angular_speed;  // Velocidad angular máxima (rad/s)
    float transition;         // Duración de la transición sinusoidal (ms)
    float arc;                // Duración del arco a velocidad constante (ms)
    int8_t sign;              // −1 = izquierda, +1 = derecha
};
```

### `struct linear_accel_params` — Aceleraciones lineales

```c
struct linear_accel_params {
    int16_t break_accel;      // Deceleración (mm/s²)
    int16_t accel_hard;       // Aceleración inicial (mm/s²)
    int16_t speed_hard;       // Umbral para cambiar a accel_soft (mm/s)
    int16_t accel_soft;       // Aceleración reducida (mm/s²)
};
```

---

## Estrategias de Velocidad

6 niveles de `enum speed_strategy`:

| Estrategia | Velocidad (mm/s) | Break | accel_hard | speed_hard | accel_soft | Full-Scale Gyro |
|-----------|:---:|:---:|:---:|:---:|:---:|:---:|
| `SPEED_EXPLORE` | 800 (explore) / 3500 (run) | 8000 | 8000 | — | — | 1000 dps |
| `SPEED_NORMAL` | 3000 | 12000 | 12000 | — | — | 2000 dps |
| `SPEED_MEDIUM` | 5000 | 15000 | 15000 | — | — | 2000 dps |
| `SPEED_FAST` | 5500 | 20000 | 20000 | 3500 | 15000 | 2000 dps |
| `SPEED_SUPER` | 6000 | 25000 | 25000 | 4000 | 20000 | 2000 dps |
| `SPEED_HAKI` | 6500 | 30000 | 30000 | 4500 | 25000 | 2000 dps |

### Notas

- **SPEED_EXPLORE**: Velocidad dual — 800 mm/s en exploración (`configure_explore_kinematics(false)`), 3500 mm/s en run (`configure_explore_kinematics(true)`).
- **SPEED_NORMAL y MEDIUM**: Aceleración simple (sin `speed_hard`/`accel_soft`).
- **SPEED_FAST, SUPER, HAKI**: Aceleración en dos etapas para evitar deslizamiento de ruedas a altas velocidades.

### Cambio de Estrategia

```c
configure_kinematics(SPEED_FAST);
// → Copia la estrategia a 'kinematics'
// → Actualiza full-scale del giroscopio si cambió
// → Resetea acumuladores de error PID
```

---

## Parámetros de Giro

Cada estrategia tiene su propio array `turn_params[]` indexado por `enum movement`. Ejemplo para `SPEED_HAKI` (`MOVE_LEFT_90`):

| Parámetro | Valor | Significado |
|-----------|-------|-------------|
| `start` | −68.8 mm | El perfil empieza 68.8 mm **antes** del sensing point de entrada → reduce el tramo recto previo |
| `end` | −68.8 mm | El perfil termina 68.8 mm **después** del sensing point de salida → reduce el tramo recto posterior |
| `linear_speed` | 2745 mm/s | Velocidad lineal durante el giro |
| `max_angular_speed` | 20.33 rad/s | Velocidad angular máxima |
| `transition` | 63.37 ms | Duración de la rampa sinusoidal de aceleración/deceleración |
| `arc` | 131.40 ms | Duración del giro a velocidad angular constante |
| `sign` | −1 | Giro a la izquierda |

### Significado geométrico

El punto de referencia es la **línea divisoria de casillas** (sensing point):

```
                sensing point entrada          sensing point salida
                      │                              │
   ───────────────────┬──────────────────────────────┬──────────────────
      tramo recto     │         perfil de giro       │   tramo recto
       reducido       │                              │    reducido
     ←── start (-) ──→│                              │←── end (-) ──→
                      │  ┌──────────────────────┐    │
                      │  │ transition (63 ms)   │    │
                      │  │ arc (131 ms)         │    │
                      │  │ transition (63 ms)   │    │
                      │  └──────────────────────┘    │
```

- **`start` negativo**: el perfil comienza antes del sensing point → el tramo recto previo se **reduce** (el robot empieza a girar antes de cruzar la línea divisoria).
- **`start` positivo**: el perfil comienza después del sensing point → el tramo recto previo se **alarga** (el robot ya ha entrado en la celda cuando empieza a girar).
- **`end` negativo**: el perfil termina después del sensing point → el tramo recto posterior se **reduce**.
- **`end` positivo**: el perfil termina antes del sensing point → el tramo recto posterior se **alarga**.
- **`transition`**: perfil sinusoidal de aceleración angular (entrada) y deceleración (salida), duración en ms.
- **`arc`**: segmento central a velocidad angular constante, duración en ms.

---

## Perfiles de Aceleración

### Aceleración Lineal

```c
if (ideal_speed < target_speed) {
    if (ideal_speed < speed_hard)
        accel = accel_hard;    // Fase 1: agresiva
    else
        accel = accel_soft;    // Fase 2: suave (reduce slip)
    ideal_speed += accel / 1000;  // 1000 = CONTROL_FREQUENCY_HZ
}
```

### Frenada

```c
if (ideal_speed > target_speed) {
    ideal_speed −= break_accel / 1000;
}
```

### Distancia de Frenada

```c
calc_straight_to_speed_distance(current_speed, target_speed)
// Usa break_accel para calcular distancia necesaria
```

⚠️ Usa `ideal_linear_speed` en lugar de `measured_linear_speed`. Ver [MV-05](15-known-issues.md#mv-05).

---

## Degradación de Velocidad

En `move_run_sequence()`, antes de ejecutar un giro:

```c
while (distance + end_offset < required_braking_distance) {
    if (speed_strategy <= SPEED_NORMAL) break;  // mínimo
    turn = kinematics_settings[−−speed_strategy].turns[...];  // bajar nivel
}
```

La velocidad de giro se **degrada** (reduce) si no hay suficiente distancia para frenar. Pero **nunca se optimiza hacia arriba** si sobra espacio. Ver [MV-09](15-known-issues.md#mv-09).


---

*Documento generado el 2026-06-12. Ver también [Movimiento](04-movement.md), [Control PID](06-control-system.md), [Floodfill](05-floodfill.md).*

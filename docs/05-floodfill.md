# 5. Algoritmo Floodfill — ZoroBot3

## Índice
1. [Visión General](#visión-general)
2. [Estructuras de Datos](#estructuras-de-datos)
3. [Modos de Floodfill](#modos-de-floodfill)
4. [Cálculo de Pesos (Time-Based)](#cálculo-de-pesos-time-based)
5. [Algoritmo BFS/Dijkstra](#algoritmo-bfsdijkstra)
6. [Seguimiento de Dirección](#seguimiento-de-dirección)
7. [Path Following (Seguimiento de Ruta)](#path-following-seguimiento-de-ruta)
8. [Exploración](#exploración)
9. [Speed Run (Carrera)](#speed-run-carrera)
10. [Estimación de Tiempo (Simulador)](#estimación-de-tiempo-simulador)

---

## Visión General

El floodfill es el núcleo de navegación para resolver laberintos. Opera en dos fases:

1. **Exploración**: Recorre el laberinto construyendo un mapa de paredes. Usa floodfill para encontrar el camino óptimo hacia la meta o hacia celdas no visitadas.
2. **Speed Run**: Una vez explorado, calcula la ruta más rápida desde la salida hasta la meta y la ejecuta a máxima velocidad.

Archivos: [`floodfill.c`](../source_code/src/floodfill.c), [`floodfill_weigths.c`](../source_code/src/floodfill_weigths.c), [`floodfill.h`](../source_code/include/floodfill.h).

---

## Estructuras de Datos

### Laberinto (`maze[MAZE_CELLS]`)

Array lineal de `int16_t`. Cada celda codifica:

| Bit | Máscara | Significado |
|-----|---------|-------------|
| 0 | `VISITED_BIT` (1) | Celda visitada |
| 1 | `EAST_BIT` (2) | Pared al este |
| 2 | `SOUTH_BIT` (4) | Pared al sur |
| 3 | `WEST_BIT` (8) | Pared al oeste |
| 4 | `NORTH_BIT` (16) | Pared al norte |

### Direcciones (`enum compass_direction`)

Los valores son desplazamientos en el array lineal:

```c
TARGET     = 0                     // Origen (celda meta en BFS)
EAST       = 1                     // +1 columna
SOUTH_EAST = 1 − MAZE_COLUMNS      // Diagonal SE
SOUTH      = −MAZE_COLUMNS          // −1 fila
SOUTH_WEST = −1 − MAZE_COLUMNS      // Diagonal SW
WEST       = −1                     // −1 columna
NORTH_WEST = −1 + MAZE_COLUMNS      // Diagonal NW
NORTH      = MAZE_COLUMNS           // +1 fila
NORTH_EAST = 1 + MAZE_COLUMNS       // Diagonal NE
```

### Cola de Prioridad (`cells_queue`)

Cola ordenada por inserción (ascendente por valor floodfill). Cada elemento:
- `cell`: posición de la celda
- `direction`: tipo de dirección de llegada
- `last_step`: último paso físico (EAST/SOUTH/WEST/NORTH)
- `count`: pasos consecutivos en la misma dirección

### Pesos (`straight_weights[]` y `diagonal_weights[]`)

Arrays de `struct cell_weigth` (máx. 15 elementos):
- `speed`: velocidad al final de la celda
- `time`: tiempo para recorrer esta celda
- `penalty`: penalización por giro = `2 × (speed − init_speed) / accel`

---

## Modos de Floodfill

Seleccionable desde el menú (`menu_run_get_floodfill_type()`):

### 1. `FLOODFILL_TYPE_BASIC` (0)

Cada celda suma **1.0** independientemente de la dirección. Distancia Manhattan al objetivo.

- Empates: prioridad FRONT (seguir recto).
- Actualización: `floodfill[next] > next_distance` (estrictamente menor).

### 2. `FLOODFILL_TYPE_DIAGONAL` (1)

- Ortogonal: coste **1.0**
- Diagonal: coste **0.7** (≈ 1/√2)
- Actualización: `floodfill[next] >= next_distance` (permite reemplazar igual).

### 3. `FLOODFILL_TYPE_TIME` (2)

**El modo más avanzado.** Costes basados en **tiempos reales estimados** usando modelo cinemático:

- Cada celda recta: tiempo de `straight_weights[]` (180 mm/celda con aceleración).
- Cada celda diagonal: tiempo de `diagonal_weights[]` (127.3 mm/celda).
- Los giros incurren en penalizaciones de tiempo (decelerar + re-acelerar).
- Permite que caminos con más celdas pero menos giros sean seleccionados como óptimos.

---

## Cálculo de Pesos (Time-Based)

`floodfill_weights_table()` en [`floodfill_weigths.c`](../source_code/src/floodfill_weigths.c):

### Cinemática

```
d = v₀·t + ½·a·t²
```

Resuelta con fórmula cuadrática. Si se alcanza `max_speed` antes de completar la distancia, el tiempo restante se calcula a velocidad constante.

### Penalización de Giro

```
penalty = 2 × (v_actual − v₀) / aceleración
```

Modela: decelerar desde velocidad actual hasta velocidad base + re-acelerar. Es una aproximación — en realidad el robot puede no frenar completamente.

### Celdas hasta Velocidad Máxima

```c
cells = round(distancia_hasta_max / distancia_por_celda) + 3
```

El `+3` es margen de seguridad para entradas suficientes en la tabla.

---

## Algoritmo BFS/Dijkstra

### `update_floodfill()` ([`floodfill.c:722`](../source_code/src/floodfill.c#L722))

1. **Lazy-init**: Si las tablas de pesos no están calculadas, se generan.
2. **Reset**: `floodfill[] = MAZE_MAX_DISTANCE`, cola vacía.
3. **Seed**: Cada celda objetivo → `floodfill = 0`, se añade a la cola con `direction = TARGET`.
4. **BFS**: Mientras la cola no esté vacía:
   - Extraer celda con menor floodfill.
   - Para cada vecino accesible (sin pared):
     - Calcular `next_direction` (8-direccional).
     - Calcular `next_count` (consecutivos).
     - Calcular `next_distance` con `get_next_floodfill_distance()`.
     - Si mejora (o iguala en modos no-BASIC): actualizar y encolar.

### Condición de Actualización

```c
// BASIC: estrictamente menor
floodfill[next] > next_distance

// DIAGONAL y TIME: menor o igual (esencial para TIME)
floodfill[next] >= next_distance
```

En TIME, permitir `≥` mitiga parcialmente el problema de estado insuficiente (ver [FF-01](15-known-issues.md#ff-01)).

---

## Seguimiento de Dirección

### `get_next_floodfill_direction(from_dir, from_step, to_step)` ([`floodfill.c:568`](../source_code/src/floodfill.c#L568))

Determina la dirección resultante al transitar de una celda a otra. Un giro de 90° desde una dirección ortogonal produce dirección **diagonal** (ej: NORTH + EAST → NORTH_EAST). Esto activa las penalizaciones por giro.

### `get_next_floodfill_distance(from_dir, to_dir, count, last_count)` ([`floodfill.c:465`](../source_code/src/floodfill.c#L465))

El núcleo del cálculo de costes. En TIME_BASED:

| Desde | Hacia | Coste |
|-------|-------|-------|
| Ortogonal | Ortogonal (misma dir) | `straight_weights[count].time` |
| Diagonal | Diagonal (misma dir) | `diagonal_weights[count].time` |
| Diagonal | Diagonal (distinta dir) | `diag[0].time + diag[last_count].penalty` |
| Ortogonal | Diagonal | `diag[0].time + straight[last_count].penalty` |
| Diagonal | Ortogonal | `straight[0].time + diag[last_count].penalty` |

---

## Path Following (Seguimiento de Ruta)

### `get_next_floodfill_step(walls, last_step)` ([`floodfill.c:382`](../source_code/src/floodfill.c#L382))

Selecciona el siguiente paso siguiendo valores decrecientes de floodfill (greedy):

- **BASIC**: Prioriza FRONT, luego LEFT/RIGHT alternando para evitar oscilaciones.
- **DIAGONAL/TIME**: Prioriza LEFT/RIGHT, luego FRONT.

Devuelve `BACK` si todas las direcciones están bloqueadas.

### `floodfill_run()` ([`floodfill.c:1083`](../source_code/src/floodfill.c#L1083))

Optimización que "corre en recto" por celdas ya visitadas. Acumula celdas consecutivas en la misma dirección y las ejecuta como `run_straight()` multi-celda.

---

## Exploración

### `go_to_target()` ([`floodfill.c:1144`](../source_code/src/floodfill.c#L1144))

Bucle principal de exploración:

1. Calcular floodfill hacia el objetivo actual.
2. Si la celda no está visitada: leer sensores, actualizar paredes, recalcular floodfill.
3. Si `EXPLORE_COMPLETE` y nueva información cambió el camino: buscar nueva celda interesante.
4. Ejecutar movimiento (FRONT/LEFT/RIGHT/BACK).
5. Repetir hasta `floodfill = 0` (objetivo alcanzado).

### `find_closest_unknown_interesting_cell()` ([`floodfill.c:982`](../source_code/src/floodfill.c#L982))

Busca la celda no visitada más cercana siguiendo el floodfill desde la posición actual y desde la salida, comparando distancias para decidir cuál explorar.

### Estrategias de Exploración

| Estrategia | Descripción |
|-----------|-------------|
| `EXPLORE_SIMPLE` | Va directo a la meta, explorando en el camino |
| `EXPLORE_HOME` | Al llegar a la meta, vuelve al inicio |
| `EXPLORE_COMPLETE` | Explora TODAS las celdas no visitadas antes de volver |

---

## Speed Run (Carrera)

### `build_run_sequence(type)` ([`floodfill.c:1241`](../source_code/src/floodfill.c#L1241))

1. Rellena celdas no visitadas con paredes (fuerza camino por celdas conocidas).
2. Calcula floodfill desde la meta hacia la salida.
3. Sigue el gradiente decreciente generando cadena: `"BFFFFRFFFRFLLFFS"`.
4. Ajusta orientación final para quedar mirando hacia atrás.

### `smooth_run_sequence(speed)` ([`floodfill.c:1351`](../source_code/src/floodfill.c#L1351))

Convierte la cadena de caracteres en movimientos físicos:

- **SOLVE_STANDARD**: Giros simples (L→LEFT_90, LL→LEFT_180, etc.)
- **SOLVE_DIAGONALS**: Optimiza secuencias con movimientos diagonales compuestos (L+F = giro 90° con entrada diagonal, L+R = diagonal pura, etc.)

---

## Estimación de Tiempo (Simulador)

`mmsim_get_estimated_time()` ([`floodfill.c:1584`](../source_code/src/floodfill.c#L1584)):

Recorre la secuencia de movimientos acumulando tiempos de las tablas de pesos. Aplica tiempos de celda y penalizaciones por cambios de dirección. Solo disponible en MMSIM.

---

## Diagrama de Flujo

```
Inicio (start_explore o start_run)
    │
    ▼
Inicializar maze / Cargar desde EEPROM
    │
    ▼
Establecer objetivos (goals)
    │
    ▼
┌──────────────────────────────────────┐
│        update_floodfill()            │
│  ┌────────────────────────────────┐  │
│  │ Calcular tablas de pesos       │  │
│  │ (si no calculadas aún)         │  │
│  └────────────┬───────────────────┘  │
│               ▼                      │
│  ┌────────────────────────────────┐  │
│  │ Reset floodfill[] = MAX        │  │
│  │ Vaciar cola                    │  │
│  └────────────┬───────────────────┘  │
│               ▼                      │
│  ┌────────────────────────────────┐  │
│  │ Pushear goals con dist=0       │  │
│  └────────────┬───────────────────┘  │
│               ▼                      │
│  ┌────────────────────────────────┐  │
│  │ BFS Dijkstra con cola prioridad│  │
│  │ Para cada celda:               │  │
│  │   Para cada vecino sin pared:  │  │
│  │     - next_direction           │  │
│  │     - next_count               │  │
│  │     - next_distance (coste)    │  │
│  │     - Si mejora: actualizar    │  │
│  └────────────────────────────────┘  │
└──────────────────────────────────────┘
    │
    ▼
┌──────────────────────────────────────┐
│     Path Following / Exploración     │
│  Mientras floodfill[pos] > 0:        │
│    - Elegir paso con min floodfill   │
│    - Ejecutar movimiento             │
│    - Actualizar posición/dirección   │
└──────────────────────────────────────┘
    │
    ▼
¿Explorando? → Buscar siguiente celda interesante
¿En carrera? → build_run_sequence() → move_run_sequence()
```

---

*Documento generado el 2026-06-12. Los problemas conocidos están en [FF-01 a FF-13](15-known-issues.md). Ver también [Cinemática](14-kinematics.md), [Movimiento](04-movement.md).*

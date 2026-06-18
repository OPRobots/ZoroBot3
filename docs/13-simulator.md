# Simulador MMSIM

## Índice
1. [Integración](#integración)
2. [API de Paredes Virtuales](#api-de-paredes-virtuales)
3. [Estimación de Tiempo](#estimación-de-tiempo)
4. [Compilación Condicional](#compilación-condicional)

---

## Integración

El proyecto soporta el simulador **Micromouse Simulator (MMSIM)** mediante compilación condicional:

```c
#ifdef MMSIM_ENABLED
```

Cuando `MMSIM_ENABLED` está definido:
- Las lecturas de hardware real (ADC, SPI, GPIO) se reemplazan por la API del simulador.
- El robot "virtual" ejecuta el mismo código de navegación y control.

La API se encuentra en [`source_code/lib/mmsim_api/`](../source_code/lib/mmsim_api/).

---

## API de Paredes Virtuales

En modo simulación, las paredes se obtienen del simulador:

```c
API_wallFront()  → bool  // ¿Hay pared al frente?
API_wallLeft()   → bool  // ¿Hay pared a la izquierda?
API_wallRight()  → bool  // ¿Hay pared a la derecha?
```

Estas reemplazan las lecturas de `get_walls()` que en hardware real usan los sensores IR.

### Visualización

En `update_floodfill()`, tras el BFS, se colorean las celdas del camino óptimo:
- **Rojo**: celdas en la ruta óptima
- **Azul**: otras celdas visitadas

---

## Estimación de Tiempo

`mmsim_get_estimated_time()` en [`floodfill.c:1584`](../source_code/src/floodfill.c#L1584):

- Recorre la secuencia de movimientos generada.
- Acumula tiempos de las tablas de pesos cinemáticos (`straight_weights[]`, `diagonal_weights[]`).
- Aplica penalizaciones por cambios de dirección.
- Disponible solo en `MMSIM_ENABLED`.

### Algoritmo

```
Para cada movimiento en la secuencia:
├── Si es recto (F):
│   tiempo += straight_weights[move_count].time
├── Si es giro (L/R):
│   tiempo += diagonal_weights[move_count].time
│   Si cambió la dirección respecto al movimiento anterior:
│       penalty += diagonal_weights[last_count].penalty
└── move_count++ (misma dirección) o move_count=0 (cambio)
```

### Issue Conocido

- **FF-05**: `move_count` se incrementa antes de usarse como índice. La primera celda usa `weights[1].time` en lugar de `weights[0].time`. Impacto: estimación incorrecta en simulador.

---

## Compilación Condicional

### Archivos afectados por `MMSIM_ENABLED`

| Archivo | Cambio |
|---------|--------|
| `sensors.c` | `get_walls()` usa API simulador en lugar de ADC |
| `sensors.h` | Omite includes de hardware |
| `floodfill.c` | `mmsim_get_estimated_time()`, `mmsim_finish_explore()` |
| `control.c` | Versión simplificada sin acceso a hardware |
| `config.h` | Omite includes de hardware |
| `move.c` | Ajustes en includes |

### Funciones exclusivas del simulador

| Función | Descripción |
|---------|-------------|
| `mmsim_get_estimated_time()` | Estima tiempo total de la carrera |
| `mmsim_finish_explore()` | Finaliza exploración, imprime maze, llama `API_setTime()` |

---

## Herramientas Complementarias

En [`source_code/utils/`](../source_code/utils/):
- **simulator/**: Simulador de laberinto en PC
- **visualizer/**: Visualizador de rutas en Python

---

*Documento generado el 2026-06-12. Ver también [Floodfill](05-floodfill.md), [Arquitectura Software](02-software-architecture.md).*

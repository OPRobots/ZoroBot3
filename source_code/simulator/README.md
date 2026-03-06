# ZoroBot3 Maze Simulator (Standalone)

Este simulador permite ejecutar el algoritmo de mapeo y resolución de laberintos de ZoroBot3 en PC, usando exactamente las mismas funciones y lógica que el robot real.

## ¿Qué hace?
- Recibe como input un array de laberinto en el formato interno de ZoroBot3 (por consola o fichero).
- Simula la exploración y resolución del laberinto, igual que el robot.
- Imprime el laberinto mapeado con las celdas visitadas.
- Usa el mismo flujo que el simulador MMS (`floodfill_start_explore()` + `floodfill_loop()`).

## Arquitectura

El simulador usa los **ficheros originales de ZoroBot3** sin modificarlos:

```
simulator/
├── sim_api.c      # Implementa API_* (lee paredes de array interno)
├── sim_main.c     # main() - lee laberinto y ejecuta exploración
└── README.md

src/               # Ficheros originales (no modificados)
├── floodfill.c    # Algoritmo de exploración
├── maze.c         # Gestión del laberinto
├── move.c         # Movimientos (usa API_* cuando MMSIM_ENABLED)
├── sensors.c      # Sensores (usa API_wall* cuando MMSIM_ENABLED)
└── ...
```

El flag `-DMMSIM_ENABLED` activa los bloques condicionales en el código original que usan `API_moveForward()`, `API_turnLeft()`, `API_wallFront()`, etc.

## Compilación
```bash
make -f Makefile.simulator
```

O manualmente:
```bash
gcc -DMMSIM_ENABLED -I./include -I./lib/mmsim_api \
    simulator/sim_main.c simulator/sim_api.c \
    src/floodfill.c src/maze.c src/move.c src/floodfill_weigths.c \
    src/sensors.c src/menu_run.c src/control.c \
    -o simulator/maze_sim.exe -lm
```

## Uso

```bash
# Desde fichero
./simulator/maze_sim.exe -floodfill-type=2 < simulator/test_maze_input.txt

# Interactivo
./simulator/maze_sim.exe -floodfill-type=2
# Introduce: 16 (tamaño)
# Pega: 14,12,20,20,...
```

## Argumentos
- `-floodfill-type=0` : Floodfill básico (Manhattan)
- `-floodfill-type=1` : Floodfill diagonal
- `-floodfill-type=2` : Floodfill por tiempo (default)
- `-floodfill-type=3` : Floodfill por tiempo v2

## Formato del fichero de entrada

```
16
14,12,20,20,20,20,20,20,4,20,20,20,20,20,20,6,...
```

- **Línea 1**: Tamaño del laberinto (16 = 16x16)
- **Línea 2+**: Valores de celdas separados por coma (256 valores para 16x16)

Cada valor codifica las paredes (bits): N=1, E=2, S=4, W=8

## Ejemplo de salida

```
=== ZoroBot3 Standalone Maze Simulator ===
Floodfill type: 2
Maze loaded: 16x16 (256 cells)
Starting exploration...

Interesting cell 0,2 dist: 2.00
Interesting cell 0,3 dist: 3.00
...

=== MAPPED MAZE ===
·═══════·═══════·═══════·...
║   V   │   V   │   V   │...
```

Las casillas marcadas con `V` son las que el robot ha visitado durante la exploración.

## Diferencia con MMS

El simulador MMS (`lib/mmsim_api/mmsim_api.c`) se comunica con un GUI externo mediante stdout/stdin. Este simulador standalone (`simulator/sim_api.c`) lee las paredes de un array interno, permitiendo ejecutar el algoritmo sin necesidad de GUI.

Ambos usan el mismo flag `MMSIM_ENABLED` y las mismas funciones `API_*`.

# ZoroBot3 Maze Simulator (Standalone)

Este simulador permite ejecutar el algoritmo de mapeo y resolución de ZoroBot3 en PC, usando la lógica del robot real.

## ¿Qué hace?

- Recibe como input un fichero de laberinto en formato .map.
- Simula la exploración y resolución del laberinto, igual que el robot.
- Muestra **stats de exploración** (igual que MMS).
- Imprime el laberinto con los **tiempos**.
- Imprime el laberinto mapeado con las celdas visitadas y el **camino óptimo**.

## Arquitectura

El simulador usa los **ficheros originales de ZoroBot3** sin modificarlos:

```
simulator/
├── mazes/              # Ficheros de laberintos (.map o .txt)
|    ├── Portuguese Micromouse Contest 2025.map
|    └── ...
├── sim_api.c           # Implementa API_* (lee paredes de array interno)
├── sim_main.c          # main() - lee laberinto y ejecuta exploración
├── Makefile.linux      # Makefile para compilar en Linux
├── Makefile.simulator  # Makefile para compilar en Windows
└── README.md

src/               # Ficheros originales (no modificados)
├── floodfill.c    # Algoritmo de exploración
├── maze.c         # Gestión del laberinto
├── move.c         # Movimientos (usa API_* cuando MMSIM_ENABLED)
├── sensors.c      # Sensores (usa API_wall* cuando MMSIM_ENABLED)
└── ...
```

El flag `-DMMSIM_ENABLED` activa los bloques condicionales en el código original.

## Compilación

```bash
cd simulator
make -f Makefile.simulator
```

## Uso

```bash
# Ejecutar con fichero .map
simulator\maze_sim.exe -floodfill-type=0 -explore-type=0 maze.map
```

## Argumentos

- `-floodfill-type=0` : FLOODFILL_BASIC
- `-floodfill-type=1` : FLOODFILL_DIAGONAL
- `-floodfill-type=2` : FLOODFILL_TIME
- `-floodfill-type=3` : FLOODFILL_TIMEv2 (default)

- `-explore-type=0` : EXPLORE_SIMPLE
- `-explore-type=1` : EXPLORE_HOME
- `-explore-type=2` : EXPLORE_COMPLETE (default)

## Stats de Exploración

El simulador muestra las mismas estadísticas que el MMS:

```
=== Exploration Stats ===
Total Distance: 836
Current Run Distance: 268
Total Effective Distance: 836.0
Current Run Effective Distance: 268.0
Total Turns: 356
Current Run Turns: 114
Best Run Distance: 268
Best Run Effective Distance: 268.0
Best Run Turns: 114
Score: 501.2
```

## Camino Óptimo

El camino óptimo desde el inicio hasta la meta se marca con ███

```
=== CAMINO ÓPTIMO ===
·═══════·═══════·═══════·
║  ███  │       │  ███  ║
·       ·═══════·       ·
║  ███     ███     ███  ║
·═══════·═══════·═══════·
```

## Formato del Laberinto (.map)

El fichero .map tiene el mismo formato que usa MMS:

```
+---+---+---+---+
|               |
+   +---+   +   +
|   |       |   |
+   +   +---+   +
|       |       |
+---+---+---+---+

[14,12,6,12,8,18,8,18,10,10,24,6,10,24,20,18]
```

- **Primera parte**: Representación ASCII del laberinto (se ignora)
- **Entre []**: Valores de celdas separados por coma (256 valores para 16x16)

Cada valor codifica las paredes (bits): N=16, E=2, S=4, W=8, Visited=1

Los ficheros .map se guardan automáticamente desde MMS.

## Ejemplo de Output

```
=== ZoroBot3 Maze Simulator (Standalone) ===
Laberinto: ../others/Portuguese Micromouse Contest 2025.map (16x16)
Floodfill type: 0

Interesting cell: 1 - 2
Interesting cell: 2 - 3
Interesting cell: 1 - 11
Interesting cell: 1 - 11
Interesting cell: 6 - 4
Interesting cell: 6 - 3
Interesting cell: 7 - 3
Interesting cell: 8 - 3
Interesting cell: 9 - 3
Interesting cell: 1 - 1
Interesting cell: 1 - 1


Current position: 0
Current direction: 16
Run sequence: BRRLLRRLLRRLLRRLLFFRRFFLFFFFLRLFFFFFFFLLRLRLRFRRLLRLLRFS
Final position: 136
Final direction: 1
Solve strategy: 1
MOVE_START > MOVE_RIGHT_TO_135 > MOVE_LEFT_45_TO_45 > MOVE_RIGHT_45_TO_45 > ...
> MOVE_HOME

=== Exploración completada ===
Loops ejecutados: 1
=== Exploration Stats ===
Total Distance: 600
Current Run Distance: 360
Total Effective Distance: 600.0
Current Run Effective Distance: 360.0
Total Turns: 152
Current Run Turns: 82
Best Run Distance: 360
Best Run Effective Distance: 360.0
Best Run Turns: 82
Score: 517.2

=== TIEMPOS (FLOODFILL) ===
·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·
║ 59.00 ║ 54.00   55.00   54.00   53.00   52.00   53.00 ║ 36.00   35.00   34.00   33.00   32.00   31.00   30.00   31.00   32.00 ║
·       ·       ·═══════·       ·═══════·       ·═══════·       ·═══════·═══════·═══════·═══════·═══════·       ·       ·       ·
║ 58.00 ║ 53.00   52.00 ║ 53.00   52.00   51.00   52.00 ║ 37.00   38.00   39.00   40.00   41.00   42.00 ║ 29.00 ║ 30.00 ║ 33.00 ║
·       ·       ·       ·       ·═══════·       ·═══════·═══════·═══════·═══════·═══════·═══════·       ·       ·       ·       ·
║ 57.00 ║ 54.00 ║ 51.00 ║ 52.00   51.00   50.00   49.00   48.00   47.00   46.00   45.00   44.00   43.00 ║ 28.00   29.00 ║ 34.00 ║
·       ·       ·       ·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·       ·═══════·       ·
║ 56.00   55.00 ║ 50.00 ║ 11.00   10.00   9.000   10.00   11.00   12.00   13.00   14.00   15.00   16.00 ║ 27.00 ║ 36.00   35.00 ║
·       ·       ·       ·       ·═══════·       ·═══════·       ·═══════·═══════·═══════·═══════·       ·       ·       ·═══════·
║ 57.00 ║ 56.00 ║ 49.00 ║ 10.00   9.000   8.000   9.000   10.00   11.00 ║ 20.00   19.00   18.00   17.00 ║ 26.00 ║ 37.00 ║ 40.00 ║
·       ·       ·       ·       ·═══════·       ·═══════·       ·═══════·       ·       ·       ·═══════·       ·       ·       ·
║ 58.00 ║ 57.00 ║ 48.00 ║ 9.000   8.000   7.000   8.000   9.000   10.00 ║ 21.00 ║ 20.00 ║ 19.00 ║ 24.00 ║ 25.00 ║ 38.00   39.00 ║
·       ·       ·       ·       ·═══════·       ·═══════·═══════·═══════·       ·       ·       ·       ·       ·═══════·       ·
║ 59.00   58.00 ║ 47.00 ║ 8.000   7.000   6.000 ║ 3.000   2.000   1.000 ║ 22.00 ║ 21.00 ║ 20.00 ║ 23.00 ║ 24.00 ║ 43.00 ║ 40.00 ║
·       ·       ·       ·═══════·═══════·       ·       ·═══════·       ·       ·       ·       ·       ·       ·       ·       ·
║ 60.00 ║ 59.00 ║ 46.00   47.00 ║ 6.000   5.000   4.000 ║ 0.000   0.000 ║ 23.00   22.00   21.00   22.00 ║ 25.00 ║ 42.00   41.00 ║
·       ·       ·       ·═══════·═══════·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·
║ 61.00 ║ 60.00 ║ 45.00   44.00   45.00 ║ 6.000 ║ 5.000 ║ 0.000   0.000 ║ 24.00 ║ 23.00 ║ 22.00 ║ 23.00 ║ 26.00 ║ 43.00 ║ 42.00 ║
·       ·       ·═══════·       ·═══════·═══════·       ·═══════·═══════·       ·       ·       ·       ·       ·       ·       ·
║ 62.00   61.00 ║ 44.00   43.00   42.00   43.00 ║ 6.000 ║ 27.00   26.00   25.00 ║ 24.00 ║ 23.00 ║ 24.00 ║ 27.00 ║ 44.00 ║ 43.00 ║
·       ·       ·       ·═══════·       ·═══════·═══════·       ·═══════·       ·       ·       ·       ·       ·       ·       ·
║ 63.00 ║ 62.00 ║ 43.00   42.00   41.00   40.00 ║ 39.00 ║ 28.00   27.00   26.00   25.00   24.00   25.00 ║ 28.00 ║ 45.00   44.00 ║
·       ·       ·       ·       ·═══════·       ·       ·═══════·═══════·═══════·═══════·═══════·       ·       ·       ·       ·
║ 64.00 ║ 63.00 ║ 44.00 ║ 43.00   42.00 ║ 39.00   38.00 ║ 35.00   34.00 ║ 31.00   30.00 ║ 29.00 ║ 26.00 ║ 29.00 ║ 46.00 ║ 45.00 ║
·       ·       ·═══════·═══════·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·
║ 65.00 ║ 64.00   63.00   62.00 ║ 41.00   40.00 ║ 37.00   36.00 ║ 33.00   32.00 ║ 29.00   28.00   27.00 ║ 30.00 ║ 47.00 ║ 46.00 ║
·       ·═══════·═══════·       ·═══════·       ·       ·       ·       ·       ·       ·       ·═══════·       ·       ·       ·
║ 66.00   65.00   64.00 ║ 61.00 ║ 40.00   39.00   38.00 ║ 35.00   34.00 ║ 31.00   30.00 ║ 29.00   30.00   31.00 ║ 48.00   47.00 ║
·       ·═══════·       ·       ·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·       ·
║ 67.00 ║ 64.00 ║ 63.00 ║ 60.00   59.00   58.00   57.00   56.00   55.00   54.00   53.00   52.00   51.00   50.00   49.00   48.00 ║
·       ·       ·       ·═══════·═══════·═══════·       ·═══════·═══════·       ·═══════·═══════·═══════·═══════·       ·       ·
║ 68.00 ║ 63.00   62.00   61.00   60.00   59.00   58.00   57.00   56.00   55.00   54.00   53.00   52.00   51.00   50.00 ║ 49.00 ║
·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·

=== CELDAS VISITADAS Y CAMINO ÓPTIMO ===
·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·
║       ║   V       V       V                           ║   V       V       V       V       V       V       V       V       V   ║
·       ·       ·═══════·       ·═══════·       ·═══════·       ·═══════·═══════·═══════·═══════·═══════·       ·       ·       ·
║       ║  ███     ███  ║   V                           ║   V       V       V       V       V       V   ║       ║       ║   V   ║
·       ·       ·       ·       ·═══════·       ·═══════·═══════·═══════·═══════·═══════·═══════·       ·       ·       ·       ·
║       ║  ███  ║  ███  ║   V       V       V       V       V       V       V       V       V       V   ║               ║   V   ║
·       ·       ·       ·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·       ·═══════·       ·
║  ███     ███  ║  ███  ║                  ███     ███     ███     ███     ███     ███     ███     ███  ║       ║   V       V   ║
·       ·       ·       ·       ·═══════·       ·═══════·       ·═══════·═══════·═══════·═══════·       ·       ·       ·═══════·
║  ███  ║   V   ║  ███  ║                  ███              V           ║   V       V      ███     ███  ║       ║   V   ║       ║
·       ·       ·       ·       ·═══════·       ·═══════·       ·═══════·       ·       ·       ·═══════·       ·       ·       ·
║  ███  ║   V   ║  ███  ║                  ███      V       V       V   ║   V   ║       ║  ███  ║       ║       ║   V       V   ║
·       ·       ·       ·       ·═══════·       ·═══════·═══════·═══════·       ·       ·       ·       ·       ·═══════·       ·
║  ███      V   ║  ███  ║                  ███  ║  ███     ███     ███  ║   V   ║       ║  ███  ║       ║       ║   V   ║   V   ║
·       ·       ·       ·═══════·═══════·       ·       ·═══════·       ·       ·       ·       ·       ·       ·       ·       ·
║  ███  ║   V   ║  ███      V   ║          ███     ███  ║          ███  ║   V       V      ███     ███  ║       ║   V       V   ║
·       ·       ·       ·═══════·═══════·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·
║  ███  ║   V   ║  ███     ███      V   ║       ║   V   ║           V   ║   V   ║       ║   V   ║  ███  ║       ║   V   ║   V   ║
·       ·       ·═══════·       ·═══════·═══════·       ·═══════·═══════·       ·       ·       ·       ·       ·       ·       ·
║  ███      V   ║          ███     ███      V   ║       ║   V       V       V   ║       ║   V   ║  ███  ║       ║   V   ║   V   ║
·       ·       ·       ·═══════·       ·═══════·═══════·       ·═══════·       ·       ·       ·       ·       ·       ·       ·
║  ███  ║   V   ║                  ███     ███  ║   V   ║                                   V      ███  ║       ║   V       V   ║
·       ·       ·       ·       ·═══════·       ·       ·═══════·═══════·═══════·═══════·═══════·       ·       ·       ·       ·
║  ███  ║   V   ║       ║               ║  ███     ███  ║   V       V   ║  ███     ███  ║   V   ║  ███  ║       ║   V   ║   V   ║
·       ·       ·═══════·═══════·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·       ·
║  ███  ║   V       V       V   ║           V   ║  ███     ███  ║  ███     ███  ║  ███     ███     ███  ║       ║   V   ║   V   ║
·       ·═══════·═══════·       ·═══════·       ·       ·       ·       ·       ·       ·       ·═══════·       ·       ·       ·
║  ███                  ║   V   ║           V       V   ║  ███     ███  ║   V       V   ║                       ║   V       V   ║
·       ·═══════·       ·       ·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·       ·
║  ███  ║       ║       ║   V       V       V       V       V       V       V       V       V       V       V       V       V   ║
·       ·       ·       ·═══════·═══════·═══════·       ·═══════·═══════·       ·═══════·═══════·═══════·═══════·       ·       ·
║  ███  ║                                                                                                               ║       ║
·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·═══════·
```

## Diferencia con MMS

El simulador MMS (`lib/mmsim_api/mmsim_api.c`) se comunica con un GUI externo mediante stdout/stdin. Este simulador standalone (`simulator/sim_api.c`) lee las paredes de un array interno, permitiendo ejecutar el algoritmo sin necesidad de GUI y sin los retrasos en las comunicaciones, por lo que se pueden obtener los stats de manera rapida y eficaz.

Ambos usan el mismo flag `MMSIM_ENABLED` y las mismas funciones `API_*` definidas en zoro.

## Uso del script de Python para pruebas automáticas

El script `autotest_mazes.py` permite ejecutar el simulador sobre una carpeta de laberintos (.map o .txt) y guardar los resultados de distancia para cada tipo de floodfill.

### Ejemplo de uso:

```bash
python simulator/autotest_mazes.py <mazes_folder>
```

- `<mazes_folder>`: Ruta a la carpeta que contiene los archivos .map o .txt

El script ejecuta el simulador para cada laberinto y cada tipo de floodfill (0, 1, 2, 3), y guarda los resultados en `resultados_floodfill.txt`.

### Requisitos:

- Tener `maze_sim.exe` compilado en la carpeta `simulator/`
- Python 3 instalado

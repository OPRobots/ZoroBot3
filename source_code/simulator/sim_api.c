// sim_api.c
// Implementación standalone de la API del simulador
// Imita el mmsim_api.c: en lugar de comunicarse con MMS,
// lee directamente de un array interno (true_maze)

#include "mmsim_api.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// Bits de pared (mismos que en floodfill.h)
#define EAST_BIT 2
#define SOUTH_BIT 4
#define WEST_BIT 8
#define NORTH_BIT 16

// Direcciones
#define DIR_NORTH 0
#define DIR_EAST 1
#define DIR_SOUTH 2
#define DIR_WEST 3

// Estado interno del simulador
static int16_t true_maze[256];   // Laberinto real (max 16x16)
static float sim_floodfill[256]; // Copia de los valores de floodfill
static char cell_colors[256];    // Colores de las celdas ('B'=azul, 'R'=rojo, etc)
static bool sim_visited[256];    // Celdas visitadas por el robot
static int maze_width = 16;
static int maze_height = 16;
static int current_x = 0;
static int current_y = 0;
static int current_dir = DIR_NORTH;  // 0=N, 1=E, 2=S, 3=W

// Stats de exploración (igual que MMS)
static int total_distance = 0;
static int current_run_distance = 0;
static float total_effective_distance = 0.0f;
static float current_run_effective_distance = 0.0f;
static int total_turns = 0;
static int current_run_turns = 0;
static int best_run_distance = 0;
static float best_run_effective_distance = 0.0f;
static int best_run_turns = 0;
static bool run_started = false;
static bool solved = false;
static bool has_left_start = false;  // Si el ratón ha salido de (0,0) alguna vez

static float get_effective_distance(int distance) {
    return distance > 2 ? distance / 2.0f + 1.0f : (float)distance;
}

// Detecta si la posición está en el centro (meta)
static bool is_in_goal(int x, int y) {
    int half = maze_width / 2;
    return (x == half - 1 || x == half) && (y == half - 1 || y == half);
}

// ============== Funciones de configuración (llamadas desde sim_main.c) ==============

void sim_api_set_maze_size(int width, int height) {
    maze_width = width;
    maze_height = height;
    // Inicializar arrays
    for (int i = 0; i < 256; i++) {
        sim_floodfill[i] = 9999.0f;
        cell_colors[i] = 0;  // Sin color
        sim_visited[i] = false;
    }
}

void sim_api_set_maze_cell(int index, int16_t value) {
    if (index >= 0 && index < 256) {
        true_maze[index] = value;
    }
}

void sim_api_reset_position(void) {
    current_x = 0;
    current_y = 0;
    current_dir = DIR_NORTH;
    has_left_start = false;
    // Marcar celda inicial como visitada
    sim_visited[0] = true;
}

static void start_run(void) {
    current_run_distance = 0;
    current_run_effective_distance = 0.0f;
    current_run_turns = 0;
    run_started = true;
}

static void finish_run(void) {
    run_started = false;
    solved = true;
    float current_score = current_run_turns + current_run_effective_distance;
    float best_score = best_run_turns + best_run_effective_distance;
    // Si es el primer run o es mejor que el mejor
    if (best_run_distance == 0 || current_score < best_score) {
        best_run_distance = current_run_distance;
        best_run_effective_distance = current_run_effective_distance;
        best_run_turns = current_run_turns;
    }
}

void sim_api_print_stats(void) {
    float score = 2000.0f;
    if (solved) {
        // Fórmula MMS: best_effective + best_turns + 0.1*(total_effective + total_turns)
        score = best_run_effective_distance + best_run_turns +
                0.1f * (total_effective_distance + total_turns);
    }
    API_log("=== Exploration Stats ===");
    char buf[80];
    
    snprintf(buf, sizeof(buf), "Total Distance: %d", total_distance);
    API_log(buf);
    
    snprintf(buf, sizeof(buf), "Current Run Distance: %d", current_run_distance);
    API_log(buf);
    
    snprintf(buf, sizeof(buf), "Total Effective Distance: %.1f", total_effective_distance);
    API_log(buf);
    
    snprintf(buf, sizeof(buf), "Current Run Effective Distance: %.1f", current_run_effective_distance);
    API_log(buf);
    
    snprintf(buf, sizeof(buf), "Total Turns: %d", total_turns);
    API_log(buf);
    snprintf(buf, sizeof(buf), "Current Run Turns: %d", current_run_turns);
    API_log(buf);
    
    snprintf(buf, sizeof(buf), "Best Run Distance: %d", best_run_distance);
    API_log(buf);
    snprintf(buf, sizeof(buf), "Best Run Effective Distance: %.1f", best_run_effective_distance);
    API_log(buf);
    snprintf(buf, sizeof(buf), "Best Run Turns: %d", best_run_turns);
    API_log(buf);
    
    snprintf(buf, sizeof(buf), "Score: %.1f", score);
    API_log(buf);
}

// Devuelve el color de una celda (para floodfill_maze_print)
char sim_api_get_cell_color(int x, int y) {
    int idx = x + y * maze_width;
    if (idx >= 0 && idx < 256) {
        return cell_colors[idx];
    }
    return 0;
}

// ============== Implementación de API (usada por ficheros originales) ==============

int API_mazeWidth(void) {
    return maze_width;
}

int API_mazeHeight(void) {
    return maze_height;
}

// Obtiene el índice de celda en el array
static int get_cell_index(int x, int y) {
    return x + y * maze_width;
}

// Detecta si hay pared en la dirección absoluta dada
static bool has_wall(int x, int y, int abs_dir) {
    if (x < 0 || x >= maze_width || y < 0 || y >= maze_height) {
        return true;  // Fuera del laberinto = pared
    }
    int cell = true_maze[get_cell_index(x, y)];
    switch (abs_dir) {
        case DIR_NORTH: return (cell & NORTH_BIT) != 0;
        case DIR_EAST:  return (cell & EAST_BIT) != 0;
        case DIR_SOUTH: return (cell & SOUTH_BIT) != 0;
        case DIR_WEST:  return (cell & WEST_BIT) != 0;
    }
    return true;
}

int API_wallFront(void) {
    return has_wall(current_x, current_y, current_dir);
}

int API_wallLeft(void) {
    return has_wall(current_x, current_y, (current_dir + 3) % 4);
}

int API_wallRight(void) {
    return has_wall(current_x, current_y, (current_dir + 1) % 4);
}

int API_moveForward(void) {
    // Verificar si hay pared
    if (API_wallFront()) {
        fprintf(stderr, "CRASH: Intento de avanzar hacia pared en (%d,%d) dir=%d\n", 
                current_x, current_y, current_dir);
        return 0;  // Crash
    }
    
    // Detectar inicio de run: cuando salimos de (0,0)
    if (current_x == 0 && current_y == 0 && !has_left_start) {
        start_run();
        has_left_start = true;
    }
    
    // Actualizar stats (cada celda = 2 unidades como en MMS)
    total_distance += 2;
    float eff = get_effective_distance(2);
    total_effective_distance += eff;
    if (run_started) {
        current_run_distance += 2;
        current_run_effective_distance += eff;
    }
    
    // Mover en la dirección actual
    switch (current_dir) {
        case DIR_NORTH: current_y++; break;
        case DIR_EAST:  current_x++; break;
        case DIR_SOUTH: current_y--; break;
        case DIR_WEST:  current_x--; break;
    }
    
    // Marcar celda como visitada
    int idx = current_x + current_y * maze_width;
    if (idx >= 0 && idx < 256) {
        sim_visited[idx] = true;
    }
    
    // Detectar llegada a meta
    if (run_started && is_in_goal(current_x, current_y)) {
        finish_run();
    }
    
    // Detectar vuelta al inicio (fin de run no completado)
    if (current_x == 0 && current_y == 0 && has_left_start) {
        if (run_started) {
            run_started = false;  // endUnfinishedRun
        }
        // Siguiente movimiento inicia nuevo run
        has_left_start = false;
    }
    
    return 1;
}

void API_turnRight(void) {
    current_dir = (current_dir + 1) % 4;
    total_turns++;
    if (run_started) {
        current_run_turns++;
    }
}

void API_turnLeft(void) {
    current_dir = (current_dir + 3) % 4;
    total_turns++;
    if (run_started) {
        current_run_turns++;
    }
}

void API_moveBack(void) {
    API_turnRight();
    API_turnRight();
    API_moveForward();
}

// ============== Funciones de UI ==============

void API_setWall(int x, int y, char direction) {
    (void)x; (void)y; (void)direction;
}

void API_clearWall(int x, int y, char direction) {
    (void)x; (void)y; (void)direction;
}

void API_setColor(int x, int y, char color) {
    // Guardar color de la celda (usado por floodfill.c para marcar camino)
    int idx = x + y * maze_width;
    if (idx >= 0 && idx < 256) {
        cell_colors[idx] = color;
    }
}

void API_clearColor(int x, int y) {
    int idx = x + y * maze_width;
    if (idx >= 0 && idx < 256) {
        cell_colors[idx] = 0;
    }
}

void API_clearAllColor(void) {
    for (int i = 0; i < 256; i++) {
        cell_colors[i] = 0;
    }
}

void API_setText(int x, int y, char *text) {
    (void)x; (void)y; (void)text;
}

void API_setFloodFill(int x, int y, float value) {
    // Guardar valores de floodfill
    int idx = x + y * maze_width;
    if (idx >= 0 && idx < 256) {
        sim_floodfill[idx] = value;
    }
}

void API_clearText(int x, int y) {
    (void)x; (void)y;
}

void API_clearAllText(void) {
}

int API_wasReset(void) {
    return 0;
}

void API_ackReset(void) {
}

void API_log(char *text) {
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}

// Imprime el laberinto con tiempos (valores de floodfill)
void sim_api_print_times_maze(void) {
    printf("\n=== TIEMPOS (FLOODFILL) ===\n");
    
    for (int r = maze_height - 1; r >= 0; r--) {
        // Borde superior
        printf("·");
        for (int c = 0; c < maze_width; c++) {
            int idx = c + r * maze_width;
            if (true_maze[idx] & NORTH_BIT) {
                printf("═══════·");
            } else {
                printf("       ·");
            }
        }
        printf("\n");
        
        // Celdas con tiempos
        for (int c = 0; c < maze_width; c++) {
            int idx = c + r * maze_width;
            // Pared oeste
            if (true_maze[idx] & WEST_BIT) {
                printf("║");
            } else {
                printf(" ");
            }
            // Valor de floodfill (formato: 6 chars + espacio = 7 total)
            float val = sim_floodfill[idx];
            if (val > 99) {
                printf("%6.1f ", val);
            } else if (val > 9) {
                printf("%6.2f ", val);
            } else {
                printf("%6.3f ", val);
            }
            // Pared este (solo al final de la fila)
            if (c == maze_width - 1 && (true_maze[idx] & EAST_BIT)) {
                printf("║");
            }
        }
        printf("\n");
    }
    
    // Borde inferior
    printf("·");
    for (int c = 0; c < maze_width; c++) {
        int idx = c;  // Fila 0
        if (true_maze[idx] & SOUTH_BIT) {
            printf("═══════·");
        } else {
            printf("       ·");
        }
    }
    printf("\n");
}

// Imprime el laberinto combinando visitadas (V) y camino óptimo (███)
void sim_api_print_path_maze(void) {
    printf("\n=== CELDAS VISITADAS Y CAMINO ÓPTIMO ===\n");
    
    for (int r = maze_height - 1; r >= 0; r--) {
        // Borde superior
        printf("·");
        for (int c = 0; c < maze_width; c++) {
            int idx = c + r * maze_width;
            if (true_maze[idx] & NORTH_BIT) {
                printf("═══════·");
            } else {
                printf("       ·");
            }
        }
        printf("\n");
        
        // Celdas
        for (int c = 0; c < maze_width; c++) {
            int idx = c + r * maze_width;
            // Pared oeste
            if (true_maze[idx] & WEST_BIT) {
                printf("║");
            } else {
                printf(" ");
            }
            // Contenido: ███ para camino azul, V para visitadas
            if (cell_colors[idx] == 'B') {
                printf("  ███  ");
            } else if (sim_visited[idx]) {
                printf("   V   ");
            } else {
                printf("       ");
            }
            // Pared este (solo al final de la fila)
            if (c == maze_width - 1 && (true_maze[idx] & EAST_BIT)) {
                printf("║");
            }
        }
        printf("\n");
    }
    
    // Borde inferior
    printf("·");
    for (int c = 0; c < maze_width; c++) {
        int idx = c;  // Fila 0
        if (true_maze[idx] & SOUTH_BIT) {
            printf("═══════·");
        } else {
            printf("       ·");
        }
    }
    printf("\n");
}

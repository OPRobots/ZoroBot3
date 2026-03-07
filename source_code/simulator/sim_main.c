// sim_main.c
// Punto de entrada del simulador standalone
// Lee laberinto de stdin y ejecuta exploración usando código original de ZoroBot3

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "floodfill.h"
#include "control.h"

// Declaraciones externas de sim_api.c
extern void sim_api_set_maze_size(int w, int h);
extern void sim_api_set_maze_cell(int i, int16_t v);
extern void sim_api_reset_position(void);
extern void sim_api_print_stats(void);
extern char sim_api_get_cell_color(int x, int y);
extern void sim_api_print_times_maze(void);
extern void sim_api_print_path_maze(void);

// Variable global para floodfill_type (usada por menu_run.c cuando MMSIM_ENABLED)
int MMSIM_FLOODFILL_TYPE = 2;  // Default: FLOODFILL_TYPE_TIME

static void print_usage(const char *prog) {
    fprintf(stderr, "Uso: %s [-floodfill-type=N] < maze_file.txt\n", prog);
    fprintf(stderr, "  N: 0=BASIC, 1=DIAGONAL, 2=TIME (default), 3=TIMEv2\n");
    fprintf(stderr, "\nFormato del fichero de entrada:\n");
    fprintf(stderr, "  Linea 1: tamaño (ej: 16 para 16x16)\n");
    fprintf(stderr, "  Linea 2+: valores de celdas separados por coma\n");
}

int main(int argc, char *argv[]) {
    // Parsear argumentos
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-floodfill-type=", 16) == 0 ||
            strncmp(argv[i], "--floodfill-type=", 17) == 0) {
            const char *val = strchr(argv[i], '=') + 1;
            if (val) {
                MMSIM_FLOODFILL_TYPE = atoi(val);
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    // Leer tamaño del laberinto
    int size;
    if (scanf("%d", &size) != 1 || size <= 0 || size > 16) {
        fprintf(stderr, "Error: tamaño de laberinto inválido (debe ser 1-16)\n");
        return 1;
    }

    sim_api_set_maze_size(size, size);

    // Leer valores de celdas
    int cells = size * size;
    for (int i = 0; i < cells; i++) {
        int16_t value;
        if (scanf(" %hd", &value) != 1) {
            // Intentar con coma
            char comma;
            if (scanf(" %c %hd", &comma, &value) != 2) {
                fprintf(stderr, "Error leyendo celda %d\n", i);
                return 1;
            }
        }
        sim_api_set_maze_cell(i, value);
        
        // Consumir coma separadora si existe
        int c = getchar();
        if (c != ',' && c != ' ' && c != '\n' && c != '\r' && c != EOF) {
            ungetc(c, stdin);
        }
    }

    printf("=== ZoroBot3 Maze Simulator (Standalone) ===\n");
    printf("Laberinto: %dx%d (%d celdas)\n", size, size, cells);
    printf("Floodfill type: %d\n\n", MMSIM_FLOODFILL_TYPE);

    // Ejecutar exploración (igual que mmsim.c)
    sim_api_reset_position();
    set_race_started(true);
    floodfill_start_explore();
    
    uint32_t loop_count = 0;
    while (is_race_started()) {
        floodfill_loop();
        loop_count++;
        if (loop_count > 100000) {
            fprintf(stderr, "ERROR: Bucle infinito detectado (%u iteraciones)\n", loop_count);
            break;
        }
    }

    printf("\n=== Exploración completada ===\n");
    printf("Loops ejecutados: %u\n", loop_count);
    
    // Imprimir stats de exploración
    sim_api_print_stats();
    
    // Imprimir laberinto con tiempos
    sim_api_print_times_maze();
    
    // Imprimir mapa combinado de visitados + camino óptimo
    sim_api_print_path_maze();

    return 0;
}

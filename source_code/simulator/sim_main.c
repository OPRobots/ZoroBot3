// sim_main.c
// Punto de entrada del simulador standalone
// Lee laberinto de stdin y ejecuta exploración usando código original de ZoroBot3

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "control.h"
#include "floodfill.h"

// Declaraciones externas de sim_api.c
extern void sim_api_set_maze_size(int w, int h);
extern void sim_api_set_maze_cell(int i, int16_t v);
extern void sim_api_reset_position(void);
extern void sim_api_print_stats(void);
extern char sim_api_get_cell_color(int x, int y);
extern void sim_api_print_times_maze(void);
extern void sim_api_print_path_maze(void);

// Variables globales para floodfill_type y explore_type (usadas por menu_run.c cuando MMSIM_ENABLED)
int MMSIM_FLOODFILL_TYPE = 3; // Default: FLOODFILL_TYPE_TIMEv2
int MMSIM_EXPLORE_TYPE = 2;   // Default: EXPLORE_COMPLETE

static void print_usage(const char *prog) {
  fprintf(stderr, "Uso: %s [-floodfill-type=F] [-explore-type=E] maze_file.map\n", prog);
  fprintf(stderr, "  F: 0=BASIC, 1=DIAGONAL, 2=TIME (default), 3=TIMEv2\n");
  fprintf(stderr, "  E: 0=SIMPLE, 1=HOME, 2=COMPLETE (default)\n");
  fprintf(stderr, "\nFormato del fichero:\n");
  fprintf(stderr, "  Laberinto ASCII seguido de valores entre []\n");
  fprintf(stderr, "  Ejemplo: [14,12,6,12,...]\n");
}

// Lee el fichero y extrae los valores entre []
static int load_maze_from_file(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Error: No se puede abrir '%s'\n", filename);
    return -1;
  }

  // Buscar el caracter '['
  int c;
  while ((c = fgetc(f)) != EOF) {
    if (c == '[')
      break;
  }

  if (c == EOF) {
    fprintf(stderr, "Error: No se encontró '[' en el fichero\n");
    fclose(f);
    return -1;
  }

  // Leer los 256 valores (16x16)
  sim_api_set_maze_size(16, 16);

  for (int i = 0; i < 256; i++) {
    int16_t value;
    if (fscanf(f, "%hd", &value) != 1) {
      fprintf(stderr, "Error leyendo celda %d\n", i);
      fclose(f);
      return -1;
    }
    sim_api_set_maze_cell(i, value);

    // Consumir coma o ]
    c = fgetc(f);
    while (c == ' ' || c == '\n' || c == '\r')
      c = fgetc(f);
    if (c == ']')
      break; // Fin de datos
    if (c != ',')
      ungetc(c, f);
  }

  fclose(f);
  return 0;
}

int main(int argc, char *argv[]) {
  const char *maze_file = NULL;

  // Parsear argumentos
  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], "-floodfill-type=", 16) == 0 ||
        strncmp(argv[i], "--floodfill-type=", 17) == 0) {
      const char *val = strchr(argv[i], '=') + 1;
      if (val) {
        MMSIM_FLOODFILL_TYPE = atoi(val);
      }
    } else if (strncmp(argv[i], "-explore-type=", 14) == 0 ||
               strncmp(argv[i], "--explore-type=", 15) == 0) {
      const char *val = strchr(argv[i], '=') + 1;
      if (val) {
        MMSIM_EXPLORE_TYPE = atoi(val);
      }
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (argv[i][0] != '-') {
      // Argumento sin guión = fichero de laberinto
      maze_file = argv[i];
    }
  }

  if (!maze_file) {
    fprintf(stderr, "Error: Debes especificar un fichero de laberinto\n");
    print_usage(argv[0]);
    return 1;
  }

  // Cargar laberinto desde fichero
  if (load_maze_from_file(maze_file) != 0) {
    return 1;
  }

  printf("=== ZoroBot3 Maze Simulator (Standalone) ===\n");
  printf("Laberinto: %s (16x16)\n", maze_file);
  printf("Floodfill type: %d\n\n", MMSIM_FLOODFILL_TYPE);

  // Ejecutar exploración (igual que mmsim.c)
  sim_api_reset_position();
  set_race_started(true);
  floodfill_start_explore();

  uint32_t loop_count = 0;
  while (is_race_started()) {
    floodfill_loop();
    loop_count++;
    if (loop_count > 3000) {
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
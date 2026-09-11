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
int MMSIM_FLOODFILL_TYPE = 2; // Default: FLOODFILL_TYPE_TIME
int MMSIM_EXPLORE_TYPE = 2;   // Default: EXPLORE_COMPLETE

// ============================================
// MANIFIESTO CLI (--describe)
// ============================================

enum option_type {
  OPT_ENUM,
};

struct option_choice {
  int value;
  const char *label;
  const char *description;
};

struct option_spec {
  const char *name;
  const char *alias;
  enum option_type type;
  int default_value;
  const char *value_name;
  const struct option_choice *choices;
  size_t choices_count;
  const char *description;
  int *target;
};

static const struct option_choice FLOODFILL_CHOICES[] = {
    {0, "BASIC", "Distancia Manhattan (1.0 por celda ortogonal)."},
    {1, "DIAGONAL", "Coste 1.0 ortogonal / 0.7 diagonal."},
    {2, "TIME", "Coste en tiempo real segun cinematica y penalizaciones de giro."},
};

static const struct option_choice EXPLORE_CHOICES[] = {
    {0, "SIMPLE", "Ir directo a la meta explorando en el camino."},
    {1, "HOME", "Explorar y volver al inicio."},
    {2, "COMPLETE", "Explorar todas las celdas no visitadas antes de volver."},
    {3, "INFINITE", "Exploracion continua (puede no terminar)."},
};

static const struct option_spec OPTION_SPECS[] = {
    {
        .name = "--floodfill-type",
        .alias = "-floodfill-type",
        .type = OPT_ENUM,
        .default_value = 2,
        .value_name = "F",
        .choices = FLOODFILL_CHOICES,
        .choices_count = sizeof(FLOODFILL_CHOICES) / sizeof(FLOODFILL_CHOICES[0]),
        .description = "Tipo de floodfill a usar durante la exploracion.",
        .target = &MMSIM_FLOODFILL_TYPE,
    },
    {
        .name = "--explore-type",
        .alias = "-explore-type",
        .type = OPT_ENUM,
        .default_value = 2,
        .value_name = "E",
        .choices = EXPLORE_CHOICES,
        .choices_count = sizeof(EXPLORE_CHOICES) / sizeof(EXPLORE_CHOICES[0]),
        .description = "Estrategia de exploracion a ejecutar.",
        .target = &MMSIM_EXPLORE_TYPE,
    },
};

#define OPTION_SPECS_COUNT (sizeof(OPTION_SPECS) / sizeof(OPTION_SPECS[0]))

static void json_print_string(const char *s) {
  putchar('"');
  for (; *s; s++) {
    switch (*s) {
      case '"': printf("\\\""); break;
      case '\\': printf("\\\\"); break;
      case '\n': printf("\\n"); break;
      case '\r': printf("\\r"); break;
      case '\t': printf("\\t"); break;
      default: putchar((unsigned char)*s);
    }
  }
  putchar('"');
}

static void print_manifest(void) {
  printf("{\n");
  printf("  \"schema_version\": 1,\n");
  printf("  \"program\": \"maze_sim\",\n");
  printf("  \"description\": \"Simulador standalone del firmware ZoroBot3 (Micromouse).\",\n");
  printf("  \"arg_style\": \"equals\",\n");
  printf("  \"positionals\": [\n");
  printf("    {\"name\": \"maze_file\", \"type\": \"path\", \"required\": true, "
         "\"description\": \"Ruta al fichero .map del laberinto\"}\n");
  printf("  ],\n");
  printf("  \"options\": [\n");
  for (size_t i = 0; i < OPTION_SPECS_COUNT; i++) {
    const struct option_spec *spec = &OPTION_SPECS[i];
    printf("    {\n");
    printf("      \"name\": ");
    json_print_string(spec->name);
    printf(",\n");
    printf("      \"aliases\": [");
    if (spec->alias) {
      json_print_string(spec->alias);
    }
    printf("],\n");
    printf("      \"type\": \"enum\",\n");
    printf("      \"default\": %d,\n", spec->default_value);
    printf("      \"required\": false,\n");
    printf("      \"repeatable\": false,\n");
    printf("      \"value_name\": ");
    json_print_string(spec->value_name);
    printf(",\n");
    printf("      \"choices\": [\n");
    for (size_t c = 0; c < spec->choices_count; c++) {
      printf("        {\"value\": %d, \"label\": ", spec->choices[c].value);
      json_print_string(spec->choices[c].label);
      printf(", \"description\": ");
      json_print_string(spec->choices[c].description);
      printf("}%s\n", c + 1 < spec->choices_count ? "," : "");
    }
    printf("      ],\n");
    printf("      \"description\": ");
    json_print_string(spec->description);
    printf("\n");
    printf("    }%s\n", i + 1 < OPTION_SPECS_COUNT ? "," : "");
  }
  printf("  ]\n");
  printf("}\n");
}

static void print_usage(const char *prog) {
  fprintf(stderr, "Uso: %s [-floodfill-type=F] [-explore-type=E] maze_file.map\n", prog);
  fprintf(stderr, "  F: 0=BASIC, 1=DIAGONAL, 2=TIME (default)\n");
  fprintf(stderr, "  E: 0=SIMPLE, 1=HOME, 2=COMPLETE (default)\n");
  fprintf(stderr, "  --describe: imprime el manifiesto JSON de opciones\n");
  fprintf(stderr, "\nFormato del fichero:\n");
  fprintf(stderr, "  Laberinto ASCII seguido de valores entre []\n");
  fprintf(stderr, "  Ejemplo: [14,12,6,12,...]\n");
}

// Busca una opción con formato nombre=valor y devuelve su descriptor
static const struct option_spec *find_option(const char *arg, const char **value) {
  for (size_t i = 0; i < OPTION_SPECS_COUNT; i++) {
    const struct option_spec *spec = &OPTION_SPECS[i];
    const char *names[2] = {spec->name, spec->alias};
    for (int n = 0; n < 2; n++) {
      if (!names[n]) {
        continue;
      }
      size_t len = strlen(names[n]);
      if (strncmp(arg, names[n], len) == 0 && arg[len] == '=') {
        *value = arg + len + 1;
        return spec;
      }
    }
  }
  return NULL;
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
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    }
    if (strcmp(argv[i], "--describe") == 0) {
      print_manifest();
      return 0;
    }

    const char *value = NULL;
    const struct option_spec *spec = find_option(argv[i], &value);
    if (spec) {
      *spec->target = (int)strtol(value, NULL, 10);
    } else if (argv[i][0] != '-') {
      // Argumento sin guión = fichero de laberinto
      maze_file = argv[i];
    } else {
      fprintf(stderr, "Opción desconocida: %s\n", argv[i]);
      print_usage(argv[0]);
      return 1;
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
  printf("Floodfill type: %d\t", MMSIM_FLOODFILL_TYPE);
  printf("Explore type: %d\n\n", MMSIM_EXPLORE_TYPE);

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
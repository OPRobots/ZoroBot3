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
static int16_t true_maze[256];  // Laberinto real (max 16x16)
static int maze_width = 16;
static int maze_height = 16;
static int current_x = 0;
static int current_y = 0;
static int current_dir = DIR_NORTH;  // 0=N, 1=E, 2=S, 3=W

// ============== Funciones de configuración (llamadas desde sim_main.c) ==============

void sim_api_set_maze_size(int width, int height) {
    maze_width = width;
    maze_height = height;
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
    
    // Mover en la dirección actual
    switch (current_dir) {
        case DIR_NORTH: current_y++; break;
        case DIR_EAST:  current_x++; break;
        case DIR_SOUTH: current_y--; break;
        case DIR_WEST:  current_x--; break;
    }
    return 1;
}

void API_turnRight(void) {
    current_dir = (current_dir + 1) % 4;
}

void API_turnLeft(void) {
    current_dir = (current_dir + 3) % 4;
}

void API_moveBack(void) {
    API_turnRight();
    API_turnRight();
    API_moveForward();
}

// ============== Funciones de UI (no-op en standalone) ==============

void API_setWall(int x, int y, char direction) {
    (void)x; (void)y; (void)direction;
}

void API_clearWall(int x, int y, char direction) {
    (void)x; (void)y; (void)direction;
}

void API_setColor(int x, int y, char color) {
    (void)x; (void)y; (void)color;
}

void API_clearColor(int x, int y) {
    (void)x; (void)y;
}

void API_clearAllColor(void) {
}

void API_setText(int x, int y, char *text) {
    (void)x; (void)y; (void)text;
}

void API_setFloodFill(int x, int y, float value) {
    (void)x; (void)y; (void)value;
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

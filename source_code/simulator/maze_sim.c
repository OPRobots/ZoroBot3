// maze_sim.c
// Implementación de funciones maze para el simulador standalone
// Reemplaza maze.c cuando se compila el simulador

#include "floodfill_sim.h"

static struct cells_stack goals;

// Variables para simulador
static uint16_t sim_rows = 16;
static uint16_t sim_cols = 16;

void maze_simulator_set_size(uint16_t rows, uint16_t cols) {
    sim_rows = rows;
    sim_cols = cols;
}

static void add_goal(uint8_t x, uint8_t y) {
    goals.stack[goals.size++] = (x - 1) + (y - 1) * maze_get_columns();
}

uint16_t maze_get_rows(void) {
    return sim_rows;
}

uint16_t maze_get_columns(void) {
    return sim_cols;
}

uint16_t maze_get_cells(void) {
    return maze_get_rows() * maze_get_columns();
}

struct cells_stack *maze_get_goals(void) {
    goals.size = 0;
    
    // Para 16x16 el centro es (8,8), (8,9), (9,8), (9,9)
    if (sim_rows == 16 && sim_cols == 16) {
        add_goal(8, 8);
        add_goal(8, 9);
        add_goal(9, 8);
        add_goal(9, 9);
    } else {
        // Centro genérico para otros tamaños
        add_goal(sim_cols / 2, sim_rows / 2);
        add_goal(sim_cols / 2 + 1, sim_rows / 2);
        add_goal(sim_cols / 2, sim_rows / 2 + 1);
        add_goal(sim_cols / 2 + 1, sim_rows / 2 + 1);
    }
    return &goals;
}

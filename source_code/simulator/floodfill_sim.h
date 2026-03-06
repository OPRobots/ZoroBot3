// floodfill_sim.h
// Header específico para el simulador standalone de ZoroBot3
// Contiene todas las definiciones necesarias que el robot obtiene de otros módulos

#ifndef FLOODFILL_SIM_H
#define FLOODFILL_SIM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============== Constantes del laberinto ==============
#define MAZE_COLUMNS 16
#define MAZE_CELLS 256
#define MAZE_MAX_DISTANCE 65535.0f
#define CELL_DIMENSION 180

// ============== Bits de celda ==============
#define VISITED_BIT 1
#define EAST_BIT 2
#define SOUTH_BIT 4
#define WEST_BIT 8
#define NORTH_BIT 16

#define MAX_TARGETS 10

// ============== Tipos de floodfill ==============
#define FLOODFILL_TYPE_BASIC 0
#define FLOODFILL_TYPE_DIAGONAL 1
#define FLOODFILL_TYPE_TIME 2
#define FLOODFILL_TYPE_TIMEv2 3
#define FLOODFILL_TYPE_MANHATTAN 0
#define FLOODFILL_TYPE_EUCLIDEAN 1
#define FLOODFILL_TYPE_DIAGONALS_TIME 3

// ============== Aceleración exploración ==============
#define ACCEL_EXPLORE_DISABLED 0
#define ACCEL_EXPLORE_ENABLED 1

// ============== EEPROM Indices (stub) ==============
#define DATA_INDEX_MAZE 0

// ============== Estructuras ==============
struct walls {
    bool front;
    bool left;
    bool right;
};

struct virtual_walls {
    uint8_t front;
    uint8_t left;
    uint8_t right;
    uint8_t back;
};

struct cell_weigth {
    uint16_t speed;
    float time;
    float total_time;
    float penalty;
};

struct kinematics {
    uint16_t linear_speed;
    float angular_speed;
};

// ============== Enumeraciones ==============
enum compass_direction {
    TARGET = 0,
    EAST = 1,
    SOUTH_EAST = 1 - MAZE_COLUMNS,
    SOUTH = -MAZE_COLUMNS,
    SOUTH_WEST = -1 - MAZE_COLUMNS,
    WEST = -1,
    NORTH_WEST = -1 + MAZE_COLUMNS,
    NORTH = MAZE_COLUMNS,
    NORTH_EAST = 1 + MAZE_COLUMNS,
};

struct compass_direction_values {
    int8_t EAST;
    int8_t SOUTH;
    int8_t WEST;
    int8_t NORTH;
};

enum step_direction {
    NONE = -1,
    FRONT = 0,
    LEFT = 1,
    RIGHT = 2,
    BACK = 3,
};

enum movement {
    MOVE_NONE = 0,
    MOVE_HOME,
    MOVE_START,
    MOVE_END,
    MOVE_FRONT,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_LEFT_90,
    MOVE_RIGHT_90,
    MOVE_LEFT_180,
    MOVE_RIGHT_180,
    MOVE_DIAGONAL,
    MOVE_LEFT_TO_45,
    MOVE_RIGHT_TO_45,
    MOVE_LEFT_TO_135,
    MOVE_RIGHT_TO_135,
    MOVE_LEFT_45_TO_45,
    MOVE_RIGHT_45_TO_45,
    MOVE_LEFT_FROM_45,
    MOVE_RIGHT_FROM_45,
    MOVE_LEFT_FROM_45_180,
    MOVE_RIGHT_FROM_45_180,
    MOVE_BACK,
    MOVE_BACK_WALL,
    MOVE_BACK_STOP,
    MOVE_MAX = 200
};

enum speed_strategy {
    SPEED_EXPLORE = 0,
    SPEED_NORMAL = 1,
    SPEED_SAFE = 1,
    SPEED_MEDIUM = 2,
    SPEED_FAST = 3,
    SPEED_SUPER = 4,
    SPEED_HAKI = 5
};

enum solve_strategy {
    SOLVE_STANDARD = 0,
    SOLVE_CLASSIC = 0,
    SOLVE_DIAGONALS = 1
};

enum maze_type {
    MAZE_COMPETITION = 0,
    MAZE_CLASSIC
};

// ============== Estructuras de cola/pila ==============
struct queue_cell {
    uint8_t cell;
    enum compass_direction direction;
    enum compass_direction last_step;
    uint8_t count;
};

struct cells_queue {
    struct queue_cell queue[MAZE_CELLS];
    uint8_t head;
    uint8_t tail;
};

struct cells_stack {
    uint8_t stack[MAX_TARGETS];
    uint8_t size;
};

// ============== Declaraciones de funciones del simulador ==============

// Funciones de configuración del simulador
void floodfill_simulator_set_maze_size(uint8_t rows, uint8_t cols);
void floodfill_simulator_set_true_maze(int16_t *maze_array, uint16_t size);
void maze_simulator_set_size(uint16_t rows, uint16_t cols);

// Funciones de utilidad del simulador
struct walls simulator_get_walls(void);
uint16_t floodfill_count_visited(void);
int16_t floodfill_get_maze_cell(uint8_t pos);

// Funciones principales de floodfill
void floodfill_set_time_limit(uint32_t ms);
void floodfill_set_reset_maze_on_start_explore(bool reset);
bool floodfill_is_reset_maze_on_start_explore(void);
void floodfill_maze_print(void);
void floodfill_load_maze(void);
void floodfill_start_explore(void);
void floodfill_start_run(void);
void floodfill_loop(void);

// Funciones de maze
uint16_t maze_get_rows(void);
uint16_t maze_get_columns(void);
uint16_t maze_get_cells(void);
struct cells_stack *maze_get_goals(void);

// ============== Forward declarations para stubs en maze_simulator.c ==============

// menu_run stubs
uint8_t menu_run_get_floodfill_type(void);
enum speed_strategy menu_run_get_speed(void);
uint8_t menu_run_get_accel_explore(void);
enum solve_strategy menu_run_get_solve_strategy(void);
enum maze_type menu_run_get_maze_type(void);

// kinematics stubs
void configure_kinematics(enum speed_strategy speed);
uint16_t get_floodfill_linear_speed(void);
uint16_t get_floodfill_max_linear_speed(void);
uint16_t get_floodfill_accel(void);
struct kinematics get_kinematics(void);

// floodfill_weights stubs
uint16_t floodfill_weights_cells_to_max_speed(float distance, uint16_t init_speed, uint16_t max_speed, uint16_t accel);
void floodfill_weights_table(float distance, uint16_t init_speed, uint16_t max_speed, uint16_t accel, uint16_t cells, struct cell_weigth *weights);

// eeprom stubs
void eeprom_set_data(uint16_t index, void *data, uint16_t size);
void eeprom_save(void);
int16_t* eeprom_get_data(void);

// timing stubs
uint32_t get_clock_ticks(void);

// control stubs
void set_target_linear_speed(int32_t speed);
void set_ideal_angular_speed(float speed);
int32_t get_ideal_linear_speed(void);
void set_target_fan_speed(int32_t speed, int32_t ms);
bool is_race_started(void);
void set_race_started(bool state);
bool is_motor_saturated(void);
bool is_race_auto_run(void);

// move stubs
void move(enum movement m);
void move_run_sequence(void *seq);
void run_straight(float dist, float a, float b, uint16_t cells, bool c, uint32_t timeout, uint16_t speed, int8_t turn);

// leds stubs
void clear_info_leds(void);
void set_RGB_color(uint8_t r, uint8_t g, uint8_t b);
void set_RGB_color_while(uint8_t r, uint8_t g, uint8_t b, uint32_t ms);
void set_RGB_rainbow(void);
void warning_status_led(uint32_t ms);
void set_status_led(bool state);

// delay stubs
void delay(uint32_t ms);

// sensors stubs
void side_sensors_calibration(bool keep);
void lsm6dsr_gyro_z_calibration(void);
struct walls get_walls(void);

// movement string
const char* get_movement_string(enum movement m);

#endif // FLOODFILL_SIM_H

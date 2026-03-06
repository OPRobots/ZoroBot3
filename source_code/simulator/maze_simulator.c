// maze_simulator.c
// Simulador standalone que usa las funciones reales de ZoroBot3 para mapear y resolver el laberinto
// Input: array de laberinto por consola. Output: solución y número de casillas visitadas.
// Compilar: gcc -I. maze_simulator.c floodfill_sim.c maze_sim.c -o maze_sim -lm
// Uso: maze_sim.exe [-floodfill-type=N]

#include "floodfill_sim.h"

// Variables globales para simular funciones
static uint8_t sim_floodfill_type = 0;
static bool sim_race_started = false;
static uint16_t sim_move_count = 0;

// ============== STUBS para funciones no necesarias en simulador ==============

// menu_run stubs - valores idénticos a MMS (ver src/menu_run.c con MMSIM_ENABLED)
uint8_t menu_run_get_floodfill_type(void) { return sim_floodfill_type; }
enum speed_strategy menu_run_get_speed(void) { return SPEED_HAKI; }
uint8_t menu_run_get_accel_explore(void) { return true; }  // MMS usa 'true' (=1)

// kinematics stubs
void configure_kinematics(enum speed_strategy speed) { (void)speed; }
uint16_t get_floodfill_linear_speed(void) { return 500; }
uint16_t get_floodfill_max_linear_speed(void) { return 1000; }
uint16_t get_floodfill_accel(void) { return 5000; }
struct kinematics get_kinematics(void) { struct kinematics k = {500, 0}; return k; }

// floodfill_weights stubs - replica la lógica real del robot
uint16_t floodfill_weights_cells_to_max_speed(float distance, uint16_t init_speed, uint16_t max_speed, uint16_t accel) {
    // Igual que la implementación real
    float time_to_max_speed = (float)(max_speed - init_speed) / (float)accel;
    float distance_to_max_speed = (init_speed * time_to_max_speed) + (0.5f * accel * time_to_max_speed * time_to_max_speed);
    return (uint16_t)roundf(distance_to_max_speed / distance) + 2 + 1;
}

void floodfill_weights_table(float distance, uint16_t init_speed, uint16_t max_speed, uint16_t accel, uint16_t cells, struct cell_weigth *weights) {
    // Calcular primera celda
    uint16_t speed = init_speed;
    float time = distance / (float)speed;
    float total_time = time;
    weights[0].speed = speed;
    weights[0].time = time;
    weights[0].total_time = total_time;
    weights[0].penalty = 0.0f;
    
    // Calcular celdas siguientes con aceleración
    for (uint16_t i = 1; i < cells && i < 15; i++) {
        // Calcular tiempo para recorrer distancia con aceleración
        if (speed < max_speed) {
            float time_to_max = (float)(max_speed - speed) / (float)accel;
            float time_to_dist = (-speed + sqrtf(speed * speed + 2.0f * accel * distance)) / accel;
            if (time_to_dist < time_to_max) {
                time = time_to_dist;
            } else {
                float dist_during_accel = speed * time_to_max + 0.5f * accel * time_to_max * time_to_max;
                float remaining_dist = distance - dist_during_accel;
                time = time_to_max + (remaining_dist / (float)max_speed);
            }
        } else {
            time = distance / (float)speed;
        }
        total_time += time;
        
        // Actualizar velocidad después de recorrer la celda
        if (speed < max_speed) {
            float new_speed = speed + accel * time;
            speed = (new_speed > max_speed) ? max_speed : (uint16_t)new_speed;
        }
        
        // Penalización: tiempo para volver a acelerar desde init_speed si hay giro
        float penalty = (init_speed < speed) ? (float)(speed - init_speed) / (float)accel : 0.0f;
        
        weights[i].speed = speed;
        weights[i].time = time;
        weights[i].total_time = total_time;
        weights[i].penalty = penalty;
    }
}

// eeprom stubs
void eeprom_set_data(uint16_t index, void *data, uint16_t size) { (void)index; (void)data; (void)size; }
void eeprom_save(void) {}

// timing stubs
uint32_t get_clock_ticks(void) { return 0; }

// control stubs
void set_target_linear_speed(int32_t speed) { (void)speed; }
void set_ideal_angular_speed(float speed) { (void)speed; }
int32_t get_ideal_linear_speed(void) { return 0; }
void set_target_fan_speed(int32_t speed, int32_t ms) { (void)speed; (void)ms; }
bool is_race_started(void) { return sim_race_started; }
void set_race_started(bool state) { sim_race_started = state; }
bool is_motor_saturated(void) { return false; }
bool is_race_auto_run(void) { return false; }

// move stubs - cuenta pasos para métricas
void move(enum movement m) { 
    (void)m; 
    sim_move_count++;
    if (sim_move_count > 10000) {
        fprintf(stderr, "ERROR: Demasiados pasos (%d), posible bucle infinito\n", sim_move_count);
        exit(1);
    }
}
void move_run_sequence(void *seq) { (void)seq; }
void run_straight(float dist, float a, float b, uint16_t cells, bool c, uint32_t timeout, uint16_t speed, int8_t turn) {
    (void)dist; (void)a; (void)b; (void)cells; (void)c; (void)timeout; (void)speed; (void)turn;
}

// leds stubs
void clear_info_leds(void) {}
void set_RGB_color(uint8_t r, uint8_t g, uint8_t b) { (void)r; (void)g; (void)b; }
void set_RGB_color_while(uint8_t r, uint8_t g, uint8_t b, uint32_t ms) { (void)r; (void)g; (void)b; (void)ms; }
void set_RGB_rainbow(void) {}
void warning_status_led(uint32_t ms) { (void)ms; }
void set_status_led(bool state) { (void)state; }

// delay stubs
void delay(uint32_t ms) { (void)ms; }

// sensors/calibration stubs
void side_sensors_calibration(bool keep) { (void)keep; }
void lsm6dsr_gyro_z_calibration(void) {}
// get_walls delega a simulator_get_walls() que lee de true_maze[]
struct walls get_walls(void) { return simulator_get_walls(); }

// menu_run additional stubs - valores idénticos a MMS (ver src/menu_run.c con MMSIM_ENABLED)
enum solve_strategy menu_run_get_solve_strategy(void) { return SOLVE_DIAGONALS; }
enum maze_type menu_run_get_maze_type(void) { return MAZE_COMPETITION; }

// eeprom additional stubs
static int16_t dummy_eeprom_data[512] = {0};
int16_t* eeprom_get_data(void) { return dummy_eeprom_data; }

// movement string stub
const char* get_movement_string(enum movement m) {
    switch(m) {
        case MOVE_START: return "START";
        case MOVE_END: return "END";
        case MOVE_HOME: return "HOME";
        case MOVE_BACK: return "BACK";
        case MOVE_NONE: return "NONE";
        case MOVE_LEFT_90: return "LEFT_90";
        case MOVE_RIGHT_90: return "RIGHT_90";
        case MOVE_DIAGONAL: return "DIAGONAL";
        default: return "?";
    }
}

// ============== Parsea argumentos de línea de comandos ==============
static void parse_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-floodfill-type=", 16) == 0) {
            sim_floodfill_type = atoi(argv[i] + 16);
            printf("Floodfill type: %d\n", sim_floodfill_type);
        }
    }
}

int main(int argc, char *argv[]) {
    printf("=== ZoroBot3 Maze Simulator ===\n\n");
    
    // Parsear argumentos
    parse_args(argc, argv);
    
    // Leer tamaño del laberinto
    printf("Tamaño del laberinto (ej: 16 para 16x16): ");
    int maze_size;
    if (scanf("%d", &maze_size) != 1 || maze_size <= 0 || maze_size > 32) {
        printf("Tamaño inválido.\n");
        return 1;
    }
    
    int maze_cells = maze_size * maze_size;
    int16_t maze_array[1024];  // Max 32x32
    
    printf("Introduce los %d valores de casillas (separados por coma o espacio):\n", maze_cells);
    
    // Leer valores (soporta separación por coma o espacio)
    for (int i = 0; i < maze_cells; i++) {
        if (scanf(" %hd", &maze_array[i]) != 1) {
            // Intentar con coma
            char comma;
            if (scanf(" %c %hd", &comma, &maze_array[i]) != 2) {
                printf("Error leyendo valor %d\n", i);
                return 1;
            }
        }
        // Consumir coma si existe
        char c = getchar();
        if (c != ',' && c != ' ' && c != '\n' && c != EOF) {
            ungetc(c, stdin);
        }
    }
    
    printf("\nLaberinto cargado: %dx%d (%d celdas)\n", maze_size, maze_size, maze_cells);
    fflush(stdout);
    
    printf("[DEBUG] Configurando simulador...\n"); fflush(stdout);
    // Configurar simulador (tanto maze.c como floodfill.c)
    maze_simulator_set_size(maze_size, maze_size);
    printf("[DEBUG] maze_simulator_set_size OK\n"); fflush(stdout);
    floodfill_simulator_set_maze_size(maze_size, maze_size);
    printf("[DEBUG] floodfill_simulator_set_maze_size OK\n"); fflush(stdout);
    floodfill_simulator_set_true_maze(maze_array, maze_cells);
    printf("[DEBUG] floodfill_simulator_set_true_maze OK\n"); fflush(stdout);
    
    // Ejecutar exploración usando MISMO flujo que MMS
    printf("Ejecutando exploración...\n\n");
    printf("[MAIN DEBUG] floodfill_type=%d\n", menu_run_get_floodfill_type());
    fflush(stdout);
    sim_move_count = 0;
    sim_race_started = true;
    uint32_t loop_counter = 0;
    printf("[MAIN DEBUG] Llamando a floodfill_start_explore()...\n");
    fflush(stdout);
    floodfill_start_explore();  // Mismo punto de entrada que MMS
    printf("[MAIN DEBUG] Entrando en bucle principal\n");
    fflush(stdout);
    while (sim_race_started) {
        floodfill_loop();  // Mismo bucle que MMS: loop_explore() -> go_to_target()
        loop_counter++;
        if (loop_counter > 100000) {
            fprintf(stderr, "ERROR: Bucle infinito detectado (%lu iteraciones)\n", (unsigned long)loop_counter);
            break;
        }
    }
    uint16_t steps = sim_move_count;
    
    // Contar casillas visitadas
    uint16_t visited = floodfill_count_visited();
    
    // Imprimir resultado
    printf("\n=== RESULTADO ===\n");
    printf("Pasos totales: %d\n", steps);
    printf("Casillas visitadas: %d\n", visited);
    printf("Casillas totales: %d\n", maze_cells);
    printf("Eficiencia: %.1f%%\n", (float)visited / maze_cells * 100.0f);
    
    // Imprimir laberinto resuelto
    printf("\n=== LABERINTO MAPEADO ===\n");
    floodfill_maze_print();
    
    return 0;
}

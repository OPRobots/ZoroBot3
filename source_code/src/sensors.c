#include <sensors.h>

static uint8_t sensores[NUM_SENSORES] = {
    ADC_CHANNEL10, // DETECTA SENSOR_SIDE_LEFT_ID - NO CAMBIAR
    ADC_CHANNEL13, // DETECTA SENSOR_SIDE_RIGHT_ID - NO CAMBIAR
    ADC_CHANNEL11, // DETECTA SENSOR_FRONT_LEFT_ID - NO CAMBIAR
    ADC_CHANNEL12, // DETECTA SENSOR_FRONT_RIGHT_ID - NO CAMBIAR
};

static volatile uint16_t sensores_raw[NUM_SENSORES];

static volatile uint16_t sensors_off[NUM_SENSORES];
static volatile uint16_t sensors_on[NUM_SENSORES];

/**
 * @brief Set an specific emitter ON.
 *
 * @param[in] emitter Emitter type.
 */
static void set_emitter_on(uint8_t emitter) {
  switch (emitter) {
    case SENSOR_SIDE_LEFT_ID:
      gpio_set(GPIOA, GPIO0);
      break;
    case SENSOR_SIDE_RIGHT_ID:
      gpio_set(GPIOA, GPIO3);
      break;
    case SENSOR_FRONT_LEFT_ID:
      gpio_set(GPIOA, GPIO1);
      break;
    case SENSOR_FRONT_RIGHT_ID:
      gpio_set(GPIOA, GPIO2);
      break;
    default:
      break;
  }
}

/**
 * @brief Set an specific emitter OFF.
 *
 * @param[in] emitter Emitter type.
 */
static void set_emitter_off(uint8_t emitter) {
  switch (emitter) {
    case SENSOR_SIDE_LEFT_ID:
      gpio_clear(GPIOA, GPIO0);
      break;
    case SENSOR_SIDE_RIGHT_ID:
      gpio_clear(GPIOA, GPIO3);
      break;
    case SENSOR_FRONT_LEFT_ID:
      gpio_clear(GPIOA, GPIO1);
      break;
    case SENSOR_FRONT_RIGHT_ID:
      gpio_clear(GPIOA, GPIO2);
      break;
    default:
      break;
  }
}

////////////////////
////   MANUEL   ////
////////////////////

#define MAGNITUD_FILTRO 20
#define UMBRAL_FILTRO 500
#define UMBRAL_LATERAL 650
#define UMBRAL_FRONTAL 2000
#define CONTADOR 3

static uint16_t contador_lateral_s1 = 0;
static uint16_t contador_lateral_s2 = 0;

static bool s0_bool = false;
static bool s1_bool = false;
static bool s2_bool = false;
static bool s3_bool = false;

static uint32_t s0 = 0; // detecta Pared frontal izquierda
static uint32_t s1 = 0; // detecta Pared Derecha
static uint32_t s2 = 0; // detecta Pared Izquierda
static uint32_t s3 = 0; // detecta Pared frontal derecha

static uint32_t s0_aux = 0;
static uint32_t s1_aux = 0;
static uint32_t s2_aux = 0;
static uint32_t s3_aux = 0;

static uint32_t Filtro_s0[MAGNITUD_FILTRO];
static uint32_t Filtro_s1[MAGNITUD_FILTRO];
static uint32_t Filtro_s2[MAGNITUD_FILTRO];
static uint32_t Filtro_s3[MAGNITUD_FILTRO];

int i_s = 0;

void filtro_sensores() {

  // Los indices se ponen directamente para ordenar segun la preferencia de Manuel En este caso S0 -> s3 Corresponden a:
  // los sensores de pared en orden de posicion en el robot de izquierda a derecha viendo el robot desde arriba con los sensores hacia adelante
  s0_aux = get_sensor_raw_filter(SENSOR_SIDE_LEFT_ID);   // detecta Pared frontal izquierda
  s1_aux = get_sensor_raw_filter(SENSOR_FRONT_LEFT_ID);  // detecta Pared Derecha
  s2_aux = get_sensor_raw_filter(SENSOR_FRONT_RIGHT_ID); // detecta Pared Izquierda
  s3_aux = get_sensor_raw_filter(SENSOR_SIDE_RIGHT_ID);  // detecta Pared frontal derecha

  if (s0_aux > UMBRAL_FILTRO) {
    Filtro_s0[i_s] = s0_aux;
  } else {
    Filtro_s0[i_s] = UMBRAL_FILTRO;
  }
  if (s1_aux > UMBRAL_FILTRO) {
    Filtro_s1[i_s] = s1_aux;
  } else {
    Filtro_s1[i_s] = UMBRAL_FILTRO;
  }
  if (s2_aux > UMBRAL_FILTRO) {
    Filtro_s2[i_s] = s2_aux;
  } else {
    Filtro_s2[i_s] = UMBRAL_FILTRO;
  }
  if (s3_aux > UMBRAL_FILTRO) {
    Filtro_s3[i_s] = s3_aux;
  } else {
    Filtro_s3[i_s] = UMBRAL_FILTRO;
  }

  i_s = (i_s + 1) % MAGNITUD_FILTRO; // Avanza el índice circularmente cuando supera MAGNITUD FILTRO vuelve a ser 0

  s0 = 0;
  s1 = 0;
  s2 = 0;
  s3 = 0;

  for (int i = 0; i < MAGNITUD_FILTRO; i++) {
    s0 += Filtro_s0[i];
    s1 += Filtro_s1[i];
    s2 += Filtro_s2[i];
    s3 += Filtro_s3[i];
  }

  s0 = s0 / MAGNITUD_FILTRO;
  s1 = s1 / MAGNITUD_FILTRO;
  s2 = s2 / MAGNITUD_FILTRO;
  s3 = s3 / MAGNITUD_FILTRO;

  s0_bool = s0 > UMBRAL_FRONTAL;
  s1_bool = s1 > UMBRAL_LATERAL;
  s2_bool = s2 > UMBRAL_LATERAL;
  s3_bool = s3 > UMBRAL_FRONTAL;

  s0 = map(s0, UMBRAL_FILTRO, 4000, 0, 1000);
  s1 = map(s1, UMBRAL_FILTRO, 4000, 0, 1000);
  s2 = map(s2, UMBRAL_FILTRO, 4000, 0, 1000);
  s3 = map(s3, UMBRAL_FILTRO, 4000, 0, 1000);
}



int sensor0_analog() {
  return s0;
}
int sensor1_analog() {
  return s1;
}
int sensor2_analog() {
  return s2;
}
int sensor3_analog() {
  return s3;
}

bool sensor0() {
  return s0_bool;
}
bool sensor1() {
  if (!s1_bool) {
    contador_lateral_s1++;
  } else {
    contador_lateral_s1 = 0;
  }
  if (contador_lateral_s1 >= CONTADOR) {
    return false;
  } else {
    return true;
  }
}
bool sensor2() {
  if (!s2_bool) {
    contador_lateral_s2++;
  } else {
    contador_lateral_s2 = 0;
  }
  if (contador_lateral_s2 >= CONTADOR) {
    return false;
  } else {
    return true;
  }
}
bool sensor3() {
  return s3_bool;
}

////////////////////
////   MANUEL   ////
////////////////////

void get_sensors_raw(uint16_t *on, uint16_t *off) {
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    on[i] = sensors_on[i];
    off[i] = sensors_off[i];
  }
}

uint8_t *get_sensors() {
  return sensores;
}

uint8_t get_sensors_num() {
  return NUM_SENSORES;
}

/**
 * @brief Máquina de estados de valores de sensores
 *
 */
void sm_emitter_adc(void) {
  static uint8_t emitter_status = 1;
  static uint8_t sensor_index = SENSOR_SIDE_LEFT_ID;

  switch (emitter_status) {
    case 1:
      sensors_off[sensor_index] = adc_read_injected(ADC1, (sensor_index + 1));
      set_emitter_on(sensor_index);
      emitter_status = 2;
      break;
    case 2:
      adc_start_conversion_injected(ADC1);
      emitter_status = 3;
      break;
    case 3:
      sensors_on[sensor_index] = adc_read_injected(ADC1, (sensor_index + 1));
      set_emitter_off(sensor_index);
      emitter_status = 4;
      break;
    case 4:
      adc_start_conversion_injected(ADC1);
      emitter_status = 1;
      if (sensor_index == (NUM_SENSORES - 1))
        sensor_index = 0;
      else
        sensor_index++;
      break;
    default:
      break;
  }
}

uint16_t get_sensor_raw(uint8_t pos, bool on) {
  if (pos < NUM_SENSORES) {
    return on ? sensors_on[pos] : sensors_off[pos];
  } else {
    return 0;
  }
}

uint16_t get_sensor_raw_filter(uint8_t pos) {
  if (pos < NUM_SENSORES) {
    if (sensors_on[pos] > sensors_off[pos]) {
      return sensors_on[pos] - sensors_off[pos];
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}
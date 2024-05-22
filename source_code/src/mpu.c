/*
    Reference: https://github.com/Bulebots/bulebule
*/

#include "mpu.h"

#define BYTE 8
#define MPU_CAL_SAMPLE_NUM 300
#define MPU_AVERAGE_FACTOR 2
#define MPU_COMPLEMENT_2_FACTOR 2
#define MPU_CAL_SAMPLE_US 1000

#define MPU_READ 0x80
#define MPU_SMPLRT_DIV 25
#define MPU_CONFIG 26
#define MPU_GYRO_CONFIG 27
#define MPU_SIGNAL_PATH_RESET 104
#define MPU_PWR_MGMT_1 107
#define MPU_USER_CTRL 106
#define MPU_WHOAMI 117

#define MPU_GYRO_ZOUT_H 71
#define MPU_GYRO_ZOUT_L 72
#define MPU_Z_OFFS_USR_H 23
#define MPU_Z_OFFS_USR_L 24

#define MPU_MASK_H 0xFF00
#define MPU_MASK_L 0x00FF

#define MPU_GYRO_SENSITIVITY_2000_DPS 16.4
#define MPU_DPS_TO_RADPS (PI / 180)

static volatile float deg_integ;
static volatile int16_t gyro_z_raw;
static bool mpu_updating = false;

/**
 * @brief Sets mpu updating.
 *
 */
void mpu_set_updating(bool updating) {
  mpu_updating = updating;
}

/**
 * @brief Read a MPU register.
 *
 * @param[in] address Register address.
 */
static uint8_t mpu_read_register(uint8_t address) {
  uint8_t reading;

  gpio_clear(GPIOA, GPIO15);
  spi_send(SPI3, (MPU_READ | address));
  spi_read(SPI3);
  spi_send(SPI3, 0x00);
  reading = spi_read(SPI3);
  gpio_set(GPIOA, GPIO15);
  //! Este delay_us(0) ralentiza lo suficiente la lectura de registros del MPU para que no se prenda fuego
  delay_us(0);

  return reading;
}

/**
 * @brief Write a MPU register with a given value.
 *
 * @param[in] address Register address.
 * @param[in] address Register value.
 */
static void mpu_write_register(uint8_t address, uint8_t value) {
  gpio_clear(GPIOA, GPIO15);
  spi_send(SPI3, address);
  spi_read(SPI3);
  spi_send(SPI3, value);
  spi_read(SPI3);
  gpio_set(GPIOA, GPIO15);
  delay(150);
}

/**
 * @brief Read the WHOAMI register value.
 *
 * This is a read-only register set to 0x70 after reset.
 */
uint8_t mpu_who_am_i(void) {
  return mpu_read_register(MPU_WHOAMI);
}

/**
 * @brief MPU-6500 board setup.
 *
 * MPU is configured as follows:
 *
 * - Configure SPI at low speed to write (less than 1MHz)
 * - Reset MPU restoring default settings and wait 100 ms
 * - Reset signal path and wait 100 ms
 * - Set SPI mode, reset I2C Slave module
 * - Sample Rate Divider (dix) equal to 0 where: SampleRate = InternalSample /
 *   (1 + div)
 * - Set DLPF (Dual Low Pass Filter) configuration to 0 with 250 Hz of
 *   bandwidth and InternalSample = 8 kHz
 * - Configure gyroscope's Z-axis with DLPF, -2000 dps and 16.4 LSB
 * - Configure SPI at high speed (less than 20MHz)
 * - Wait 100 ms
 */
void setup_mpu(void) {
  setup_spi_low_speed();
  mpu_write_register(MPU_PWR_MGMT_1, 0x80);
  mpu_write_register(MPU_SIGNAL_PATH_RESET, 0x07);
  mpu_write_register(MPU_USER_CTRL, 0x10);
  mpu_write_register(MPU_SMPLRT_DIV, 0x00);
  mpu_write_register(MPU_CONFIG, 0x00);
  mpu_write_register(MPU_GYRO_CONFIG, 0x18);
  setup_spi_high_speed();
}

/**
 * @brief Read gyroscope's Z-axis raw value from MPU.
 */
static int16_t mpu_read_gyro_z_raw(void) {

  return ((mpu_read_register(MPU_GYRO_ZOUT_H) << BYTE) |
          mpu_read_register(MPU_GYRO_ZOUT_L));
}

static void set_gyro_z_calibration(int16_t zout_c2) {
  set_info_leds();
  setup_spi_low_speed();
  mpu_write_register(MPU_Z_OFFS_USR_H, ((uint8_t)((zout_c2 & MPU_MASK_H) >> BYTE)));
  mpu_write_register(MPU_Z_OFFS_USR_L, (uint8_t)(zout_c2 & MPU_MASK_L));
  setup_spi_high_speed();

  mpu_set_updating(true);
  clear_info_leds();
}

/**
 * @brief Calibrate the gyroscope's Z axis.
 *
 * This function should be executed when the robot is stopped. The average of
 * gyroscope z output will be substracted from the gyro output from that
 * moment on. To write MPU registers, the SPI speed is changed to low speed.
 */
void gyro_z_calibration(void) {
  int16_t zout_c2;
  float zout_av = 0;
  int16_t i;

  deg_integ = 0;
  for (i = 0; i < MPU_CAL_SAMPLE_NUM; i++) {
    set_leds_wave(35);
    zout_av = ((float)mpu_read_gyro_z_raw() + zout_av) /
              MPU_AVERAGE_FACTOR;
    delay_us(MPU_CAL_SAMPLE_US);
  }
  zout_c2 = -(int16_t)(zout_av * MPU_COMPLEMENT_2_FACTOR);

  int16_t eeprom_zout_c2;
  uint16_t *eeprom_stored_data = eeprom_get_data();
  eeprom_zout_c2 = eeprom_stored_data[1];
  if (eeprom_stored_data[0] == 0) {
    eeprom_zout_c2 = -eeprom_zout_c2;
  }
  zout_c2 += eeprom_zout_c2;

  set_gyro_z_calibration(zout_c2);

  uint16_t eeprom_data[2] = {zout_c2 >= 0 ? 1 : 0, abs(zout_c2)};
  eeprom_set_data(DATA_INDEX_GYRO_Z, eeprom_data, 2);
}

void mpu_load_eeprom(void) {
  int16_t zout_c2;
  uint16_t *eeprom_data = eeprom_get_data();
  zout_c2 = eeprom_data[1];
  if (eeprom_data[0] == 0) {
    zout_c2 = -zout_c2;
  }
  set_gyro_z_calibration(zout_c2);
  mpu_set_updating(true);
}

/**
 * @brief Update the static gyroscope's Z-axis variables.
 */
void update_gyro_readings(void) {
  if (mpu_updating) {
    // gyro_z_raw = mpu_read_gyro_z_raw();
    int16_t new_gyro_z_raw = mpu_read_gyro_z_raw();
    gyro_z_raw = 0.5 * new_gyro_z_raw + (1 - 0.5) * gyro_z_raw;
    deg_integ = deg_integ - get_gyro_z_dps() / SYSTICK_FREQUENCY_HZ;
  }
}

/**
 * @brief Get gyroscope's Z-axis degrees.
 */
float get_gyro_z_degrees(void) {
  return deg_integ;
}
/**
 * @brief Set gyroscope's Z-axis degrees.
 */
void set_gyro_z_degrees(float deg) {
  deg_integ = deg;
}

/**
 * @brief Get gyroscope's Z-axis angular speed in bits per second.
 */
int16_t get_gyro_z_raw(void) {
  return gyro_z_raw;
}

/**
 * @brief Get gyroscope's Z-axis angular speed in radians per second.
 */
float get_gyro_z_radps(void) {
  return ((float)gyro_z_raw * MPU_DPS_TO_RADPS /
          MPU_GYRO_SENSITIVITY_2000_DPS);
}

/**
 * @brief Get gyroscope's Z-axis angular speed in degrees per second.
 */
float get_gyro_z_dps(void) {
  return ((float)gyro_z_raw / MPU_GYRO_SENSITIVITY_2000_DPS);
}

#define KP_GYRO 10
#define KD_GYRO 30
#define KI_GYRO 5
static float sumError = 0;
static float errorAnterior = 0;

void set_z_angle(float angle) {
  if (!mpu_updating) {
    return;
  }
  float error = angle - deg_integ;
  float p = 0;
  float i = 0;
  float correccion = 0;
  if (abs(error) >= 1) {
    p = KP_GYRO * error;
    if (abs(sumError) < 10) {
      sumError += error;
      i = sumError * KI_GYRO;
    }
    correccion = p + i;
    if (correccion > 200) {
      correccion = 200;
    } else if (correccion < -200) {
      correccion = -200;
    }
    set_motors_speed(correccion, -(correccion));
  } else {
    sumError = 0;
    set_motors_speed(0, 0);
  }
}
void keep_z_angle(void) {
  if (!mpu_updating) {
    return;
  }
  float error = -deg_integ; //((int16_t)(deg_integ * -100)) / 100;
  float p = 0;
  float i = 0;
  float d = 0;
  float correccion = 0;

  if (abs(error) > 0.1) {
    p = KP_GYRO * error;
    d = KD_GYRO * (error - errorAnterior);
    errorAnterior = error;
    if (abs(sumError) < 10) {
      sumError += error;
      i = sumError * KI_GYRO;
    }
    // printf("%d \n", (int16_t)(sumError * 100));
    correccion = p + i + d;
    // correccion = constrain(correccion, -200, 200);
    set_motors_speed(/* 60+ */ correccion, /* 80 */ -correccion);
  } else {
    sumError = 0;
    errorAnterior = error;
    set_motors_speed(/* 6 */ 0, /* 8 */ 0);
  }
}
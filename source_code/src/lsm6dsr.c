#include "lsm6dsr.h"

#define MPU_READ 0x80
#define MPU_WHOAMI 0x0F

#define CTRL1_XL 0x10
#define CTRL2_G 0x11
#define CTRL3_C 0x12
#define CTRL6_C 0x15
#define CTRL7_G 0x16
#define CTRL8_XL 0x17
#define CTRL9_XL 0x18

#define OUTZ_L_G 0x26
#define OUTZ_H_G 0x27

#define GYRO_SENSITIVITY_2000DPS 70.0f
#define GYRO_SENSITIVITY_4000DPS 140.0f
#define MPU_DPS_TO_RADPS (PI / 180)

static stmdev_ctx_t dev_ctx;

static volatile float deg_integ;
static volatile float gyro_z_raw;
static bool mpu_updating = false;
static float offset_z = 0;

static uint8_t lsm6dsr_read_register(uint8_t address) {
  uint8_t reading;

  gpio_clear(GPIOA, GPIO15);
  spi_send(SPI3, (MPU_READ | address));
  spi_read(SPI3);
  spi_send(SPI3, 0x00);
  reading = spi_read(SPI3);
  gpio_set(GPIOA, GPIO15);

  return reading;
}

static void lsm6dsr_write_register(uint8_t address, uint8_t value) {
  gpio_clear(GPIOA, GPIO15);
  spi_send(SPI3, address);
  spi_read(SPI3);
  spi_send(SPI3, value);
  spi_read(SPI3);
  gpio_set(GPIOA, GPIO15);
}

/* #region Librería */

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
  handle = handle;
  len = len;
  lsm6dsr_write_register(reg, *bufp);
  return 0;
}
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
  handle = handle;
  len = len;
  *bufp = lsm6dsr_read_register(reg);
  return 0;
}
static void platform_delay(uint32_t ms) {
  delay(ms);
}

/* #endregion */

static int16_t lsm6dsr_read_gyro_z_raw(void) {
  uint8_t zl = lsm6dsr_read_register(OUTZ_L_G);
  uint8_t zh = lsm6dsr_read_register(OUTZ_H_G);
  return ((zh << 8) | zl);
}

void lsm6dsr_init(void) {

  /* Initialize mems driver interface */
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.mdelay = platform_delay;

  /* Restore default configuration */
  lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE);
  uint8_t rst;
  do {
    lsm6dsr_reset_get(&dev_ctx, &rst);
  } while (rst);

  /* Disable I3C interface */
  lsm6dsr_i3c_disable_set(&dev_ctx, LSM6DSR_I3C_DISABLE);
  /* Enable Block Data Update */
  lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_DISABLE);

  /* Set Output Data Rate */
  lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_12Hz5);
  lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_1666Hz);
  /* Set full scale */
  lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);
  lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_4000dps);
  /* Configure filtering chain(No aux interface)
   * Accelerometer - LPF1 + LPF2 path
   */
  lsm6dsr_xl_hp_path_on_out_set(&dev_ctx, LSM6DSR_LP_ODR_DIV_100);
  lsm6dsr_xl_filter_lp2_set(&dev_ctx, PROPERTY_ENABLE);

  delay(1000);
}

uint8_t lsm6dsr_who_am_i(void) {
  return lsm6dsr_read_register(MPU_WHOAMI);
  // uint8_t whoamI = 0;
  // lsm6dsr_device_id_get(&dev_ctx, &whoamI);
  // return whoamI;
}

void lsm6dsr_gyro_z_calibration(void) {
  mpu_updating = false;
  int32_t sum_z = 0;

  offset_z = 0;
  for (int i = 0; i < 1000; i++) {
    sum_z += lsm6dsr_read_gyro_z_raw();
    delay(10);
    set_info_leds();
  }
  clear_info_leds();

  offset_z = sum_z / 1000.0f;
  printf("Offset Z: %.4f\n", offset_z);

  int16_t eeprom_data[2] = {offset_z >= 0 ? 1 : 0, (int16_t)(abs(offset_z * 5000))};
  eeprom_set_data(DATA_INDEX_GYRO_Z, eeprom_data, 2);

  
  delay(100);
  mpu_updating = true;
}

void lsm6dsr_load_eeprom(void) {
  int16_t *eeprom_data = eeprom_get_data();
  offset_z = eeprom_data[DATA_INDEX_GYRO_Z + 1] / 5000.0f;
  if (eeprom_data[DATA_INDEX_GYRO_Z] == 0) {
    offset_z = -offset_z;
  }
  printf("Offset Z: %.4f\n", offset_z);
  mpu_updating = true;
}

void lsm6dsr_update(void) {
  if (mpu_updating) {
    float new_gyro_z_raw = lsm6dsr_read_gyro_z_raw();
    new_gyro_z_raw -= offset_z;
    gyro_z_raw = 0.4 * new_gyro_z_raw + (1 - 0.4) * gyro_z_raw;
    deg_integ = deg_integ - lsm6dsr_get_gyro_z_dps() / SYSTICK_FREQUENCY_HZ;
  }
}

float lsm6dsr_get_gyro_z_raw(void) {
  return gyro_z_raw;
}

float lsm6dsr_get_gyro_z_radps(void) {
  return (gyro_z_raw * GYRO_SENSITIVITY_4000DPS / 1000 * MPU_DPS_TO_RADPS);
}

float lsm6dsr_get_gyro_z_dps(void) {
  return (gyro_z_raw * GYRO_SENSITIVITY_4000DPS) / 1000;
}

float lsm6dsr_get_gyro_z_degrees(void) {
  return deg_integ;
}
void lsm6dsr_set_gyro_z_degrees(float deg) {
  deg_integ = deg;
}
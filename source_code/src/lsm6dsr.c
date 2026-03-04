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

#define MPU_DPS_TO_RADPS (PI / 180)

static stmdev_ctx_t dev_ctx;

static uint8_t full_scale_dps[] = {MPU_FULL_SCALE_1000DPS, MPU_FULL_SCALE_2000DPS, MPU_FULL_SCALE_4000DPS};
static uint8_t sensitivity_dps[] = {MPU_SENSITIVITY_1000DPS, MPU_SENSITIVITY_2000DPS, MPU_SENSITIVITY_4000DPS};

static uint8_t current_full_scale_dps;

static volatile float deg_integ;
static volatile float gyro_z_raw;
static bool mpu_updating = false;
static float offset_z[MPU_FULL_SCALE_COUNT];

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

static int get_sensitivity_dps(void) {
  switch (current_full_scale_dps) {
    case MPU_FULL_SCALE_1000DPS:
      return sensitivity_dps[0];
    case MPU_FULL_SCALE_2000DPS:
      return sensitivity_dps[1];
    case MPU_FULL_SCALE_4000DPS:
      return sensitivity_dps[2];
  }
  return 0;
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

  // /* Disable I3C interface */
  // lsm6dsr_i3c_disable_set(&dev_ctx, LSM6DSR_I3C_DISABLE);
  // /* Enable Block Data Update */
  // lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_DISABLE);

  // /* Set Output Data Rate */
  // lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_12Hz5);
  // lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_1666Hz);
  // /* Set full scale */
  // lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);
  // lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_4000dps);
  // /* Configure filtering chain(No aux interface)
  //  * Accelerometer - LPF1 + LPF2 path
  //  */
  // lsm6dsr_xl_hp_path_on_out_set(&dev_ctx, LSM6DSR_LP_ODR_DIV_100);
  // lsm6dsr_xl_filter_lp2_set(&dev_ctx, PROPERTY_ENABLE);

  /* Disable I3C */
  lsm6dsr_i3c_disable_set(&dev_ctx, LSM6DSR_I3C_DISABLE);

  /* IMPORTANT */
  lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

  /* Accelerometer (helper clock only) */
  lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_104Hz);
  lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);

  /* Gyro */
  lsm6dsr_gy_power_mode_set(&dev_ctx, LSM6DSR_GY_HIGH_PERFORMANCE);
  lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_1666Hz);
  lsm6dsr_gy_full_scale_set(&dev_ctx, get_kinematics().mpu.full_scale_dps);
  current_full_scale_dps = get_kinematics().mpu.full_scale_dps;

  /* Filters */
  lsm6dsr_gy_filter_lp1_set(&dev_ctx, PROPERTY_ENABLE);
  // lsm6dsr_gy_hp_path_internal_set(&dev_ctx, LSM6DSR_HP_FILTER_NONE);
  lsm6dsr_gy_lp1_bandwidth_set(&dev_ctx, LSM6DSR_LIGHT);

  delay(1000);
}

void lsm6dsr_reload_config(void) {
  if (current_full_scale_dps != get_kinematics().mpu.full_scale_dps) {
    lsm6dsr_gy_full_scale_set(&dev_ctx, get_kinematics().mpu.full_scale_dps);
    current_full_scale_dps = get_kinematics().mpu.full_scale_dps;
    delay(50);
    reset_control_all();
  }
}

uint8_t lsm6dsr_who_am_i(void) {
  return lsm6dsr_read_register(MPU_WHOAMI);
  // uint8_t whoamI = 0;
  // lsm6dsr_device_id_get(&dev_ctx, &whoamI);
  // return whoamI;
}

void lsm6dsr_gyro_z_calibration(void) {
  mpu_updating = false;
  int16_t eeprom_data[MPU_FULL_SCALE_COUNT];

  for (uint8_t i = 0; i < MPU_FULL_SCALE_COUNT; i++) {
    lsm6dsr_gy_full_scale_set(&dev_ctx, full_scale_dps[i]);
    delay(50);
    int32_t sum_z = 0;
    offset_z[i] = 0;
    for (int s = 0; s < 200; s++) {
      sum_z += lsm6dsr_read_gyro_z_raw();
      delay(5);
      set_info_leds();
    }
    clear_info_leds();

    offset_z[i] = sum_z / 200.0f;
    printf("Offset Z for full-scale %d dps: %.4f\n", full_scale_dps[i], offset_z[i]);
    eeprom_data[i] = offset_z[i] * 1000;
  }

  eeprom_set_data(DATA_INDEX_GYRO_Z, eeprom_data, MPU_DATA_LENGTH);

  lsm6dsr_gy_full_scale_set(&dev_ctx, current_full_scale_dps);
  delay(100);
  mpu_updating = true;
}

void lsm6dsr_load_eeprom(void) {
  int16_t *eeprom_data = eeprom_get_data();
  for (uint8_t i = 0; i < MPU_FULL_SCALE_COUNT; i++) {
    offset_z[i] = eeprom_data[DATA_INDEX_GYRO_Z + i] / 1000.0f;
    printf("Offset Z for full-scale %d dps: %.4f\n", full_scale_dps[i], offset_z[i]);
  }
  mpu_updating = true;
}

void lsm6dsr_update(void) {
  if (mpu_updating) {
    float new_gyro_z_raw = lsm6dsr_read_gyro_z_raw();
    new_gyro_z_raw -= get_offset_z();

    gyro_z_raw =
        get_kinematics().mpu.low_pass_filter_alpha * gyro_z_raw +
        (1 - get_kinematics().mpu.low_pass_filter_alpha) * new_gyro_z_raw;

    deg_integ = deg_integ - lsm6dsr_get_gyro_z_dps() / SYSTICK_FREQUENCY_HZ;
  }
}

float lsm6dsr_get_gyro_z_raw(void) {
  return gyro_z_raw;
}

float lsm6dsr_get_gyro_z_radps(void) {
  return (gyro_z_raw * get_sensitivity_dps() / 1000 * MPU_DPS_TO_RADPS);
}

float lsm6dsr_get_gyro_z_dps(void) {
  return (gyro_z_raw * get_sensitivity_dps()) / 1000;
}

float lsm6dsr_get_gyro_z_degrees(void) {
  return deg_integ;
}
void lsm6dsr_set_gyro_z_degrees(float deg) {
  deg_integ = deg;
}

float get_offset_z(void) {
  switch (current_full_scale_dps) {
    case MPU_FULL_SCALE_1000DPS:
      return offset_z[0];
    case MPU_FULL_SCALE_2000DPS:
      return offset_z[1];
    case MPU_FULL_SCALE_4000DPS:
      return offset_z[2];
  }
  return 0;
}

uint8_t get_current_full_scale_dps(void) {
  return current_full_scale_dps;
}
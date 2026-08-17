// mpu6500.cpp
#include "mpu6500.h"
#include <Wire.h>

// I2C address
static const uint8_t MPU_ADDR = 0x68;

// MPU6500 registers
static const uint8_t REG_WHO_AM_I    = 0x75;
static const uint8_t REG_PWR_MGMT_1  = 0x6B;
static const uint8_t REG_PWR_MGMT_2  = 0x6C;
static const uint8_t REG_SIGNAL_PATH_RESET = 0x68;
static const uint8_t REG_SMPLRT_DIV  = 0x19;
static const uint8_t REG_CONFIG      = 0x1A;
static const uint8_t REG_GYRO_CONFIG = 0x1B;
static const uint8_t REG_ACCEL_CONFIG= 0x1C;
static const uint8_t REG_ACCEL_CONFIG2 = 0x1D;
static const uint8_t REG_INT_ENABLE  = 0x38;
static const uint8_t REG_INT_STATUS  = 0x3A;
static const uint8_t REG_ACCEL_XOUT_H= 0x3B;
static const uint8_t REG_MOT_THR     = 0x1F; // Motion threshold (WOM)
static const uint8_t REG_MOT_DETECT_CTRL = 0x69;
static const uint8_t REG_USER_CTRL   = 0x6A;
static const uint8_t REG_INT_PIN_CFG = 0x37;

// Expected WHO_AM_I for MPU6500
static const uint8_t WHO_AM_I_EXPECT = 0x70;

// Conversion scales
// MPU6500: accel ±8g -> 4096 LSB/g ; gyro ±250 dps -> 131 LSB/(deg/s)
static const float ACCEL_SENS_8G = 4096.0f;
static const float GYRO_SENS_250DPS = 131.0f;

// Offsets (raw units)
static int32_t accel_offset_x_raw = 0;
static int32_t accel_offset_y_raw = 0;
static int32_t accel_offset_z_raw = 0;
static int32_t gyro_offset_x_raw  = 0;
static int32_t gyro_offset_y_raw  = 0;
static int32_t gyro_offset_z_raw  = 0;

// Internal ready flag
static bool mpu_ready = false;

// I2C helpers
static bool i2c_write_reg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return (Wire.endTransmission() == 0);
}

static bool i2c_read_regs(uint8_t reg, uint8_t *buf, size_t len) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false; // restart
  size_t toRead = len;
  Wire.requestFrom((int)MPU_ADDR, (int)toRead);
  size_t idx = 0;
  unsigned long start = millis();
  while (Wire.available() < (int)toRead) {
    if (millis() - start > 50) return false;
    delay(1);
  }
  while (Wire.available() && idx < len) {
    buf[idx++] = Wire.read();
  }
  return (idx == len);
}

static bool i2c_read_reg(uint8_t reg, uint8_t &val) {
  uint8_t b;
  if (!i2c_read_regs(reg, &b, 1)) return false;
  val = b;
  return true;
}

// Utility: combine high/low to int16
static inline int16_t toInt16(uint8_t hi, uint8_t lo) {
  return (int16_t)((hi << 8) | lo);
}

// Public API implementations

void mpu_init() {
  // Initialize I2C if not already
  //Wire.begin(); // ESP32-C3 default SDA/SCL pins; override if needed in main app

  // Reset device
  i2c_write_reg(REG_PWR_MGMT_1, 0x80); // reset
  delay(100);

  // Clear signal paths
  i2c_write_reg(REG_SIGNAL_PATH_RESET, 0x07);
  delay(50);

  // Wake up and select PLL X axis gyroscope as clock source (stable)
  // PWR_MGMT_1: set CLKSEL = 1 (PLL with X axis gyroscope)
  i2c_write_reg(REG_PWR_MGMT_1, 0x01);
  delay(10);

  // Disable standby for accel/gyro
  i2c_write_reg(REG_PWR_MGMT_2, 0x00);
  delay(10);

  // Sample rate divider: sample_rate = GyroOutputRate / (1 + SMPLRT_DIV)
  // Keep moderate sample rate (e.g., 200 Hz): GyroOutputRate typically 1kHz when DLPF enabled
  i2c_write_reg(REG_SMPLRT_DIV, 4); // -> ~200 Hz
  delay(5);

  // CONFIG: set DLPF_CFG = 3 -> ~44 Hz (reduces high-frequency noise)
  // Also set EXT_SYNC_SET = 0
  i2c_write_reg(REG_CONFIG, 0x03);
  delay(5);

  // GYRO_CONFIG: FS_SEL = 0 -> ±250 dps
  i2c_write_reg(REG_GYRO_CONFIG, 0x00);
  delay(5);

  // ACCEL_CONFIG: AFS_SEL = 2 -> ±8g (AFS_SEL bits are 4:3 -> value 2 -> 0x10)
  i2c_write_reg(REG_ACCEL_CONFIG, 0x10);
  delay(5);

  // ACCEL_CONFIG2: set accel DLPF to match ~44Hz (set A_DLPF_CFG = 3)
  i2c_write_reg(REG_ACCEL_CONFIG2, 0x03);
  delay(5);

  // INT_PIN_CFG: clear latch, set active high push-pull default
  i2c_write_reg(REG_INT_PIN_CFG, 0x00);
  delay(5);

  // Enable Data Ready interrupt (optional)
  i2c_write_reg(REG_INT_ENABLE, 0x01); // Data ready
  delay(5);

  // Mark ready if WHO_AM_I matches
  uint8_t who = 0;
  if (i2c_read_reg(REG_WHO_AM_I, who) && who == WHO_AM_I_EXPECT) {
    mpu_ready = true;
  } else {
    mpu_ready = false;
  }

  // Small delay to stabilize
  delay(20);
}

bool mpu_is_ready() {
  return mpu_ready;
}

uint8_t mpu_get_whoami() {
  uint8_t who = 0;
  if (!i2c_read_reg(REG_WHO_AM_I, who)) return 0xFF;
  return who;
}

void mpu_calibrate() {
  // Calibration: lấy trung bình nhiều mẫu khi thiết bị nằm yên
  // Số mẫu có thể điều chỉnh; 200-500 là hợp lý
  const int samples = 400;
  int64_t ax_sum = 0, ay_sum = 0, az_sum = 0;
  int64_t gx_sum = 0, gy_sum = 0, gz_sum = 0;

  // Ensure device ready
  if (!mpu_is_ready()) return;

  // Discard a few initial samples
  for (int i = 0; i < 20; ++i) {
    uint8_t tmp[14];
    i2c_read_regs(REG_ACCEL_XOUT_H, tmp, 14);
    delay(5);
  }

  for (int i = 0; i < samples; ++i) {
    uint8_t buf[14];
    if (!i2c_read_regs(REG_ACCEL_XOUT_H, buf, 14)) {
      delay(5);
      continue;
    }
    int16_t ax = toInt16(buf[0], buf[1]);
    int16_t ay = toInt16(buf[2], buf[3]);
    int16_t az = toInt16(buf[4], buf[5]);
    int16_t temp = toInt16(buf[6], buf[7]); (void)temp;
    int16_t gx = toInt16(buf[8], buf[9]);
    int16_t gy = toInt16(buf[10], buf[11]);
    int16_t gz = toInt16(buf[12], buf[13]);

    ax_sum += ax;
    ay_sum += ay;
    az_sum += az;
    gx_sum += gx;
    gy_sum += gy;
    gz_sum += gz;

    delay(5); // phù hợp với sample rate ~200Hz
  }

  // Tính trung bình raw
  int32_t ax_avg = (int32_t)(ax_sum / samples);
  int32_t ay_avg = (int32_t)(ay_sum / samples);
  int32_t az_avg = (int32_t)(az_sum / samples);
  int32_t gx_avg = (int32_t)(gx_sum / samples);
  int32_t gy_avg = (int32_t)(gy_sum / samples);
  int32_t gz_avg = (int32_t)(gz_sum / samples);

  // Với accel, khi nằm yên, Z ~ +1g (tùy orientation). Giả sử thiết bị đặt mặt trên (Z hướng +g).
  // Để tổng quát, ta tính offset sao cho |g| được bù về 0 trong trục Z:
  // offset_raw = avg_raw - expected_raw
  // expected_raw_z = +1g -> ACCEL_SENS_8G * 1.0
  accel_offset_x_raw = ax_avg;
  accel_offset_y_raw = ay_avg;
  accel_offset_z_raw = az_avg - (int32_t)ACCEL_SENS_8G; // trừ 1g trên Z

  gyro_offset_x_raw = gx_avg;
  gyro_offset_y_raw = gy_avg;
  gyro_offset_z_raw = gz_avg;
}

bool mpu_read_accel(float &ax, float &ay, float &az) {
  if (!mpu_is_ready()) return false;
  uint8_t buf[6];
  if (!i2c_read_regs(REG_ACCEL_XOUT_H, buf, 6)) return false;
  int16_t raw_ax = toInt16(buf[0], buf[1]);
  int16_t raw_ay = toInt16(buf[2], buf[3]);
  int16_t raw_az = toInt16(buf[4], buf[5]);

  // Bù offset (raw), sau đó chuyển sang g
  float ax_g = ((float)(raw_ax - accel_offset_x_raw)) / ACCEL_SENS_8G;
  float ay_g = ((float)(raw_ay - accel_offset_y_raw)) / ACCEL_SENS_8G;
  float az_g = ((float)(raw_az - accel_offset_z_raw)) / ACCEL_SENS_8G;

  ax = ax_g;
  ay = ay_g;
  az = az_g;
  return true;
}

bool mpu_read_gyro(float &gx, float &gy, float &gz) {
  if (!mpu_is_ready()) return false;
  uint8_t buf[6];
  // Gyro registers start at ACCEL_XOUT_H + 8 bytes offset
  if (!i2c_read_regs(REG_ACCEL_XOUT_H + 8, buf, 6)) return false;
  int16_t raw_gx = toInt16(buf[0], buf[1]);
  int16_t raw_gy = toInt16(buf[2], buf[3]);
  int16_t raw_gz = toInt16(buf[4], buf[5]);

  float gx_dps = ((float)(raw_gx - gyro_offset_x_raw)) / GYRO_SENS_250DPS;
  float gy_dps = ((float)(raw_gy - gyro_offset_y_raw)) / GYRO_SENS_250DPS;
  float gz_dps = ((float)(raw_gz - gyro_offset_z_raw)) / GYRO_SENS_250DPS;

  gx = gx_dps;
  gy = gy_dps;
  gz = gz_dps;
  return true;
}

bool mpu_read(
    float &ax,
    float &ay,
    float &az,
    float &gx,
    float &gy,
    float &gz
) {
  if (!mpu_is_ready()) return false;
  uint8_t buf[14];
  if (!i2c_read_regs(REG_ACCEL_XOUT_H, buf, 14)) return false;

  int16_t raw_ax = toInt16(buf[0], buf[1]);
  int16_t raw_ay = toInt16(buf[2], buf[3]);
  int16_t raw_az = toInt16(buf[4], buf[5]);
  // temp ignored: buf[6], buf[7]
  int16_t raw_gx = toInt16(buf[8], buf[9]);
  int16_t raw_gy = toInt16(buf[10], buf[11]);
  int16_t raw_gz = toInt16(buf[12], buf[13]);

  ax = ((float)(raw_ax - accel_offset_x_raw)) / ACCEL_SENS_8G;
  ay = ((float)(raw_ay - accel_offset_y_raw)) / ACCEL_SENS_8G;
  az = ((float)(raw_az - accel_offset_z_raw)) / ACCEL_SENS_8G;

  gx = ((float)(raw_gx - gyro_offset_x_raw)) / GYRO_SENS_250DPS;
  gy = ((float)(raw_gy - gyro_offset_y_raw)) / GYRO_SENS_250DPS;
  gz = ((float)(raw_gz - gyro_offset_z_raw)) / GYRO_SENS_250DPS;

  return true;
}

void mpu_setup_wom(uint8_t threshold) {
  // threshold: 1 LSB ≈ 4 mg (the datasheet indicates MOT_THR LSB ~ 4mg for some devices)
  // User expects threshold in LSB; typical desired 40-80 mg -> 10..20 LSB
  // We accept a raw threshold value; clamp to 0..255
  if (threshold > 255) threshold = 255;

  // Write motion threshold
  i2c_write_reg(REG_MOT_THR, threshold);

  // Configure MOT_DETECT_CTRL: set accelerometer hardware intelligence
  // Set WOM_EN (bit 7) and set wake frequency (bits 3:0) -> choose 20Hz (0x02) as reasonable
  // MOT_DETECT_CTRL layout differs across families; here we set a common pattern:
  // 0x80 -> enable WOM, lower bits choose ODR; using 0x80 | 0x02
  i2c_write_reg(REG_MOT_DETECT_CTRL, 0x80 | 0x02);

  // Enable cycle mode (low-power accel) and set accel LP mode for WOM:
  // Set PWR_MGMT_1: set CYCLE bit (bit 5) and keep clock source PLL (bits 2:0 = 1)
  uint8_t pwr1 = 0x01 | 0x20; // PLL_X (0x01) | CYCLE (0x20)
  i2c_write_reg(REG_PWR_MGMT_1, pwr1);

  // In PWR_MGMT_2, ensure accel is enabled (clear standby bits)
  i2c_write_reg(REG_PWR_MGMT_2, 0x00);

  // Enable WOM interrupt: INT_ENABLE bit for Motion (bit 6)
  // Note: INT_ENABLE layout: bit6 = WOM_INT_EN on many devices
  uint8_t int_en = 0;
  i2c_read_reg(REG_INT_ENABLE, int_en);
  int_en |= (1 << 6); // set WOM int enable
  i2c_write_reg(REG_INT_ENABLE, int_en);

  // Configure INT pin to latch until cleared (optional)
  // Set INT_PIN_CFG to latch until cleared (bit 5 = INT_LEVEL, bit 6 = INT_OPEN, bit 5 latch)
  // We'll set latch clear on any read of INT_STATUS by default (0x00)
  i2c_write_reg(REG_INT_PIN_CFG, 0x00);
}

void mpu_clear_interrupt() {
  // Read INT_STATUS to clear interrupt flags
  uint8_t st = 0;
  i2c_read_reg(REG_INT_STATUS, st);
  (void)st;
}

void mpu_motion_mode()
{
    if(!mpu_ready)
        return;

    //------------------------------------------------
    // Disable gyro
    //------------------------------------------------
    i2c_write_reg(
        REG_PWR_MGMT_2,
        0x07
    );

    //------------------------------------------------
    // WOM threshold
    //------------------------------------------------
    mpu_setup_wom(10);

    Serial.println(
        "[MPU] MOTION MODE"
    );
}
void mpu_normal_mode()
{
    if(!mpu_ready)
        return;

    //------------------------------------------------
    // PLL clock + wake
    //------------------------------------------------
    i2c_write_reg(
        REG_PWR_MGMT_1,
        0x01
    );

    //------------------------------------------------
    // Enable accel + gyro
    //------------------------------------------------
    i2c_write_reg(
        REG_PWR_MGMT_2,
        0x00
    );

    //------------------------------------------------
    // Data Ready interrupt
    //------------------------------------------------
    i2c_write_reg(
        REG_INT_ENABLE,
        0x01
    );

    Serial.println(
        "[MPU] NORMAL MODE"
    );
}
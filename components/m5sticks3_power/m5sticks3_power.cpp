#include "m5sticks3_power.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5sticks3_power {

static constexpr uint8_t STICKS3_PM1_REG_PWR_SRC = 0x04;
static constexpr uint8_t STICKS3_PM1_REG_PWR_CFG = 0x06;
static constexpr uint8_t STICKS3_PM1_REG_I2C_CFG = 0x09;
static constexpr uint8_t STICKS3_PM1_REG_GPIO_MODE = 0x10;
static constexpr uint8_t STICKS3_PM1_REG_GPIO_OUT = 0x11;
static constexpr uint8_t STICKS3_PM1_REG_GPIO_DRV = 0x13;
static constexpr uint8_t STICKS3_PM1_REG_GPIO_FUNC0 = 0x16;
static constexpr uint8_t STICKS3_PM1_REG_VBAT_L = 0x22;
static constexpr uint8_t STICKS3_PM1_REG_VIN_L = 0x24;
static constexpr uint8_t STICKS3_PM1_REG_5VINOUT_L = 0x26;
static constexpr uint8_t STICKS3_PM1_REG_AW8737A_PULSE = 0x53;

static constexpr uint8_t M5PM1_PWR_CFG_CHG_EN = 1 << 0;
static constexpr uint8_t M5PM1_PWR_CFG_DCDC_EN = 1 << 1;
static constexpr uint8_t M5PM1_PWR_CFG_LDO_EN = 1 << 2;
static constexpr uint8_t M5PM1_PWR_CFG_BOOST_EN = 1 << 3;
static constexpr uint8_t M5PM1_PWR_SRC_BAT = 2;
static constexpr uint8_t M5PM1_PWR_SRC_UNKNOWN = 3;
static constexpr uint8_t M5PM1_GPIO2_MASK = 1 << 2;
static constexpr uint8_t M5PM1_GPIO3_MASK = 1 << 3;
static constexpr int M5STICKS3_I2C_SDA = 47;
static constexpr int M5STICKS3_I2C_SCL = 48;
static constexpr uint8_t M5STICKS3_PMIC_ADDRESS = 0x6E;

bool M5StickS3Power::init_pmic_() {
  ESP_LOGI(TAG, "Initializing PMIC over ESPHome I2C bus");

  uint8_t power_config = 0;
  if (!this->read_byte(STICKS3_PM1_REG_PWR_CFG, &power_config)) {
    ESP_LOGE(TAG, "PMIC init failed: cannot read power config");
    this->pmic_ready_ = false;
    return false;
  }

  if (!this->write_byte(STICKS3_PM1_REG_PWR_CFG,
                        (power_config | M5PM1_PWR_CFG_CHG_EN | M5PM1_PWR_CFG_DCDC_EN | M5PM1_PWR_CFG_LDO_EN) &
                            ~M5PM1_PWR_CFG_BOOST_EN)) {
    ESP_LOGE(TAG, "PMIC init failed: cannot enable power rails");
    this->pmic_ready_ = false;
    return false;
  }
  this->boost_enabled_ = false;

  // Disable PMIC I2C auto-sleep and keep 100 kHz mode so later reads do not time out.
  if (!this->write_byte(STICKS3_PM1_REG_I2C_CFG, 0x00)) {
    ESP_LOGW(TAG, "Could not disable PMIC I2C sleep");
  }

  // PYG2_L3B_EN controls the LCD/audio rail on StickS3. GPIO mode, push-pull, low = enabled.
  this->update_bits_(STICKS3_PM1_REG_GPIO_FUNC0, 0b00110000, 0);
  this->update_bits_(STICKS3_PM1_REG_GPIO_MODE, M5PM1_GPIO2_MASK, M5PM1_GPIO2_MASK);
  this->update_bits_(STICKS3_PM1_REG_GPIO_DRV, M5PM1_GPIO2_MASK, 0);
  this->update_bits_(STICKS3_PM1_REG_GPIO_OUT, M5PM1_GPIO2_MASK, 0);

  if (!this->configure_audio_amp_()) {
    ESP_LOGW(TAG, "Audio amplifier pulse configuration failed");
  }

  this->pmic_ready_ = true;
  ESP_LOGI(TAG, "PMIC init complete");
  return true;
}

bool M5StickS3Power::init_pmic_with_library_() {
  ESP_LOGI(TAG, "Initializing PMIC before ESPHome I2C bus setup");

  Wire.begin(M5STICKS3_I2C_SDA, M5STICKS3_I2C_SCL, 100000U);
  delay(20);

  const m5pm1_err_t err = this->pm1_.begin(&Wire, M5STICKS3_PMIC_ADDRESS, M5STICKS3_I2C_SDA, M5STICKS3_I2C_SCL, 100000U);
  if (err != M5PM1_OK) {
    ESP_LOGE(TAG, "Early PMIC init failed: %d", err);
    Wire.end();
    this->pmic_ready_ = false;
    return false;
  }

  this->pm1_.setI2cConfig(0, M5PM1_I2C_SPEED_100K);
  this->pm1_.setDcdcEnable(true);
  delay(20);
  this->pm1_.setLdoEnable(true);
  delay(20);
  this->pm1_.setChargeEnable(true);
  delay(20);
  this->pm1_.setBoostEnable(false);
  this->boost_enabled_ = false;
  delay(20);

  this->pm1_.gpioSetFunc(M5PM1_GPIO_NUM_2, M5PM1_GPIO_FUNC_GPIO);
  this->pm1_.gpioSetMode(M5PM1_GPIO_NUM_2, M5PM1_GPIO_MODE_OUTPUT);
  this->pm1_.gpioSetDrive(M5PM1_GPIO_NUM_2, M5PM1_GPIO_DRIVE_PUSHPULL);
  this->pm1_.gpioSetOutput(M5PM1_GPIO_NUM_2, false);
  delay(100);

  this->pm1_.setAw8737aPulse(M5PM1_GPIO_NUM_3, M5PM1_AW8737A_PULSE_2, M5PM1_AW8737A_REFRESH_NOW);
  delay(20);

  Wire.end();

  this->pmic_ready_ = true;
  ESP_LOGI(TAG, "Early PMIC init complete");
  return true;
}

bool M5StickS3Power::read_voltage_(uint8_t low_reg, uint16_t *mv) {
  uint8_t data[2] = {0, 0};
  if (!this->read_bytes(low_reg, data, 2)) {
    return false;
  }
  *mv = data[0] | ((data[1] & 0x0F) << 8);
  return true;
}

bool M5StickS3Power::read_power_source_(uint8_t *source) {
  uint8_t raw = 0;
  if (!this->read_byte(STICKS3_PM1_REG_PWR_SRC, &raw)) {
    return false;
  }
  *source = raw & 0x07;
  return true;
}

bool M5StickS3Power::update_bits_(uint8_t reg, uint8_t mask, uint8_t value) {
  uint8_t raw = 0;
  if (!this->read_byte(reg, &raw)) {
    return false;
  }
  raw = (raw & ~mask) | (value & mask);
  return this->write_byte(reg, raw);
}

bool M5StickS3Power::configure_audio_amp_() {
  // StickS3 uses M5PM1 PYG3_SPK_Pulse for the AW8737A speaker amplifier.
  // Pulse count 2 gives a moderate gain; register bit 7 triggers the refresh.
  this->update_bits_(STICKS3_PM1_REG_GPIO_FUNC0, 0b11000000, 0);
  this->update_bits_(STICKS3_PM1_REG_GPIO_MODE, M5PM1_GPIO3_MASK, M5PM1_GPIO3_MASK);
  this->update_bits_(STICKS3_PM1_REG_GPIO_DRV, M5PM1_GPIO3_MASK, 0);

  const uint8_t pin = 3;
  const uint8_t pulse_count = 2;
  const uint8_t pulse_config = pin | (pulse_count << 5);
  if (!this->write_byte(STICKS3_PM1_REG_AW8737A_PULSE, pulse_config)) {
    return false;
  }
  return this->write_byte(STICKS3_PM1_REG_AW8737A_PULSE, pulse_config | 0x80);
}

float M5StickS3Power::estimate_battery_level_(uint16_t battery_mv) {
  if (battery_mv >= 4200) {
    return 100.0f;
  }
  if (battery_mv <= 3200) {
    return 0.0f;
  }
  return (battery_mv - 3200) * 100.0f / 1000.0f;
}

void M5StickS3Power::setup() {
  this->init_pmic_with_library_();
  this->publish_ext_5v_state_();
}

void M5StickS3Power::update() {
  if (!this->pmic_ready_ && !this->init_pmic_()) {
    ESP_LOGW(TAG, "Skipping PMIC sensor update");
    return;
  }

  uint16_t battery_mv = 0;
  uint16_t input_mv = 0;
  uint16_t five_volt_mv = 0;
  bool input_valid = false;

  if (this->read_voltage_(STICKS3_PM1_REG_VBAT_L, &battery_mv)) {
    if (this->battery_voltage_sensor_ != nullptr) {
      this->battery_voltage_sensor_->publish_state(battery_mv / 1000.0f);
    }
    if (this->battery_level_sensor_ != nullptr) {
      this->battery_level_sensor_->publish_state(this->estimate_battery_level_(battery_mv));
    }
  } else {
    ESP_LOGW(TAG, "readVbat failed");
  }

  if (this->read_voltage_(STICKS3_PM1_REG_VIN_L, &input_mv)) {
    input_valid = true;
    if (this->input_voltage_sensor_ != nullptr) {
      this->input_voltage_sensor_->publish_state(input_mv / 1000.0f);
    }
  } else {
    ESP_LOGW(TAG, "readVin failed");
  }

  if (this->read_voltage_(STICKS3_PM1_REG_5VINOUT_L, &five_volt_mv)) {
    if (this->five_volt_voltage_sensor_ != nullptr) {
      this->five_volt_voltage_sensor_->publish_state(five_volt_mv / 1000.0f);
    }
  } else {
    ESP_LOGW(TAG, "read5VInOut failed");
  }

  if (this->charging_binary_sensor_ != nullptr) {
    uint8_t power_source = M5PM1_PWR_SRC_UNKNOWN;
    const bool source_valid = this->read_power_source_(&power_source);
    const bool charging = (source_valid && power_source != M5PM1_PWR_SRC_BAT && power_source != M5PM1_PWR_SRC_UNKNOWN) ||
                          (input_valid && input_mv >= 4500);
    this->charging_binary_sensor_->publish_state(charging);
  }

  this->publish_ext_5v_state_();
}

void M5StickS3Power::set_ext_5v(bool state) {
  if (!this->pmic_ready_ && !this->init_pmic_()) {
    return;
  }

  if (!this->update_bits_(STICKS3_PM1_REG_PWR_CFG, M5PM1_PWR_CFG_BOOST_EN, state ? M5PM1_PWR_CFG_BOOST_EN : 0)) {
    ESP_LOGW(TAG, "set boost enable failed");
    return;
  }

  this->boost_enabled_ = state;
  this->publish_ext_5v_state_();
}

void M5StickS3Power::publish_ext_5v_state_() {
  if (this->ext_5v_switch_ != nullptr) {
    this->ext_5v_switch_->publish_state(this->boost_enabled_);
  }
}

void M5StickS3Power::dump_config() {
  ESP_LOGCONFIG(TAG, "M5StickS3 Power (M5PM1)");
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Battery level", this->battery_level_sensor_);
  LOG_SENSOR("  ", "Battery voltage", this->battery_voltage_sensor_);
  LOG_SENSOR("  ", "Input voltage", this->input_voltage_sensor_);
  LOG_SENSOR("  ", "5V rail voltage", this->five_volt_voltage_sensor_);
  LOG_BINARY_SENSOR("  ", "Charging", this->charging_binary_sensor_);
  LOG_SWITCH("  ", "EXT 5V", this->ext_5v_switch_);
}

}  // namespace m5sticks3_power
}  // namespace esphome

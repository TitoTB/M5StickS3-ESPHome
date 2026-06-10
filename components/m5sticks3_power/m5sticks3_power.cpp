#include "m5sticks3_power.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5sticks3_power {

static constexpr int M5STICKS3_I2C_SDA = 47;
static constexpr int M5STICKS3_I2C_SCL = 48;
static constexpr uint8_t M5STICKS3_PMIC_ADDRESS = 0x6E;

bool M5StickS3Power::init_pmic_() {
  ESP_LOGI(TAG, "Initializing PMIC with M5PM1/Wire");

  Wire.end();
  Wire.begin(M5STICKS3_I2C_SDA, M5STICKS3_I2C_SCL, 100000U);
  delay(20);

  const m5pm1_err_t err =
      this->pm1_.begin(&Wire, M5STICKS3_PMIC_ADDRESS, M5STICKS3_I2C_SDA, M5STICKS3_I2C_SCL, M5PM1_I2C_FREQ_100K);
  if (err != M5PM1_OK) {
    ESP_LOGE(TAG, "PMIC init failed: %d", err);
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

  // PYG2_L3B_EN controls the StickS3 LCD/audio rail. Low output enables it.
  this->pm1_.gpioSetFunc(M5PM1_GPIO_NUM_2, M5PM1_GPIO_FUNC_GPIO);
  this->pm1_.gpioSetMode(M5PM1_GPIO_NUM_2, M5PM1_GPIO_MODE_OUTPUT);
  this->pm1_.gpioSetDrive(M5PM1_GPIO_NUM_2, M5PM1_GPIO_DRIVE_PUSHPULL);
  this->pm1_.gpioSetOutput(M5PM1_GPIO_NUM_2, false);
  delay(100);

  if (!this->configure_audio_amp_()) {
    ESP_LOGW(TAG, "Audio amplifier pulse configuration failed");
  }

  this->pmic_ready_ = true;
  ESP_LOGI(TAG, "PMIC init complete");
  return true;
}

bool M5StickS3Power::configure_audio_amp_() {
  if (this->pm1_.gpioSetFunc(M5PM1_GPIO_NUM_3, M5PM1_GPIO_FUNC_GPIO) != M5PM1_OK) {
    return false;
  }
  if (this->pm1_.gpioSetMode(M5PM1_GPIO_NUM_3, M5PM1_GPIO_MODE_OUTPUT) != M5PM1_OK) {
    return false;
  }
  if (this->pm1_.gpioSetDrive(M5PM1_GPIO_NUM_3, M5PM1_GPIO_DRIVE_PUSHPULL) != M5PM1_OK) {
    return false;
  }
  if (this->pm1_.gpioSetOutput(M5PM1_GPIO_NUM_3, true) != M5PM1_OK) {
    return false;
  }
  ESP_LOGI(TAG, "Speaker amplifier enabled");
  return true;
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
  this->init_pmic_();
  this->publish_ext_5v_state_();
}

void M5StickS3Power::update() {
  if (!this->pmic_ready_ && !this->init_pmic_()) {
    ESP_LOGW(TAG, "Skipping PMIC sensor update");
    return;
  }

  if (this->battery_voltage_sensor_ == nullptr && this->battery_level_sensor_ == nullptr &&
      this->input_voltage_sensor_ == nullptr && this->five_volt_voltage_sensor_ == nullptr &&
      this->charging_binary_sensor_ == nullptr) {
    this->publish_ext_5v_state_();
    return;
  }

  uint16_t input_mv = 0;
  uint16_t five_volt_mv = 0;
  bool input_valid = false;

  if (this->battery_voltage_sensor_ != nullptr || this->battery_level_sensor_ != nullptr) {
    uint16_t battery_mv = 0;
    if (this->pm1_.readVbat(&battery_mv) == M5PM1_OK) {
      if (this->battery_voltage_sensor_ != nullptr) {
        this->battery_voltage_sensor_->publish_state(battery_mv / 1000.0f);
      }
      if (this->battery_level_sensor_ != nullptr) {
        this->battery_level_sensor_->publish_state(this->estimate_battery_level_(battery_mv));
      }
    } else {
      ESP_LOGW(TAG, "readVbat failed");
    }
  }

  if (this->input_voltage_sensor_ != nullptr || this->charging_binary_sensor_ != nullptr) {
    if (this->pm1_.readVin(&input_mv) == M5PM1_OK) {
      input_valid = true;
      if (this->input_voltage_sensor_ != nullptr) {
        this->input_voltage_sensor_->publish_state(input_mv / 1000.0f);
      }
    } else {
      ESP_LOGW(TAG, "readVin failed");
    }
  }

  if (this->five_volt_voltage_sensor_ != nullptr) {
    if (this->pm1_.read5VInOut(&five_volt_mv) == M5PM1_OK) {
      this->five_volt_voltage_sensor_->publish_state(five_volt_mv / 1000.0f);
    } else {
      ESP_LOGW(TAG, "read5VInOut failed");
    }
  }

  if (this->charging_binary_sensor_ != nullptr) {
    this->charging_binary_sensor_->publish_state(input_valid && input_mv >= 4500);
  }

  this->publish_ext_5v_state_();
}

void M5StickS3Power::set_ext_5v(bool state) {
  if (!this->pmic_ready_ && !this->init_pmic_()) {
    return;
  }

  if (this->pm1_.setBoostEnable(state) != M5PM1_OK) {
    ESP_LOGW(TAG, "set boost enable failed");
    return;
  }

  this->boost_enabled_ = state;
  this->publish_ext_5v_state_();
}

void M5StickS3Power::play_beep() {
  ESP_LOGI(TAG, "Playing confirmation beep");
  M5.Speaker.begin();
  delay(50);
  M5.Speaker.setVolume(150);
  M5.Speaker.tone(1800, 80);
  delay(120);
  M5.Speaker.tone(2400, 110);
  delay(150);
  M5.Speaker.stop();
  M5.Speaker.end();
}

void M5StickS3Power::publish_ext_5v_state_() {
  if (this->ext_5v_switch_ != nullptr) {
    this->ext_5v_switch_->publish_state(this->boost_enabled_);
  }
}

void M5StickS3Power::dump_config() {
  ESP_LOGCONFIG(TAG, "M5StickS3 Power (M5PM1/Wire)");
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

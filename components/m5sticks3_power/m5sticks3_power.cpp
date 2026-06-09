#include "m5sticks3_power.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5sticks3_power {

bool M5StickS3Power::init_pmic_() {
  Wire.begin(this->sda_pin_, this->scl_pin_, 100000U);
  delay(20);

  const m5pm1_err_t err = this->pm1_.begin(&Wire, this->address_, this->sda_pin_, this->scl_pin_, 100000U);
  if (err != M5PM1_OK) {
    ESP_LOGE(TAG, "PMIC init failed: %d", err);
    this->pmic_ready_ = false;
    return false;
  }

  this->pm1_.setAutoWakeEnable(true);
  this->pm1_.setDcdcEnable(true);
  delay(20);
  this->pm1_.setLdoEnable(true);
  delay(20);
  this->pm1_.setChargeEnable(true);
  delay(20);
  this->pm1_.setBoostEnable(true);
  this->boost_enabled_ = true;
  delay(20);

  this->pm1_.gpioSetFunc(M5PM1_GPIO_NUM_2, M5PM1_GPIO_FUNC_GPIO);
  this->pm1_.gpioSetMode(M5PM1_GPIO_NUM_2, M5PM1_GPIO_MODE_OUTPUT);
  this->pm1_.gpioSetDrive(M5PM1_GPIO_NUM_2, M5PM1_GPIO_DRIVE_PUSHPULL);
  this->pm1_.gpioSetOutput(M5PM1_GPIO_NUM_2, false);
  delay(100);

  this->pmic_ready_ = true;
  ESP_LOGI(TAG, "PMIC init complete");
  return true;
}

bool M5StickS3Power::resync_pmic_() {
  Wire.end();
  delay(20);
  Wire.begin(this->sda_pin_, this->scl_pin_, 100000U);
  delay(20);

  const m5pm1_err_t err = this->pm1_.begin(&Wire, this->address_, this->sda_pin_, this->scl_pin_, 100000U);
  if (err != M5PM1_OK) {
    ESP_LOGW(TAG, "PMIC re-init failed: %d", err);
    this->pmic_ready_ = false;
    return false;
  }

  this->pmic_ready_ = true;
  delay(50);
  return true;
}

bool M5StickS3Power::read_voltage_twice_(m5pm1_err_t (M5PM1::*reader)(uint16_t *), uint16_t *mv) {
  (this->pm1_.*reader)(mv);
  delay(20);
  const m5pm1_err_t err = (this->pm1_.*reader)(mv);
  return err == M5PM1_OK;
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
  if (!this->resync_pmic_()) {
    ESP_LOGW(TAG, "Skipping PMIC sensor update");
    return;
  }

  uint16_t battery_mv = 0;
  uint16_t input_mv = 0;
  uint16_t five_volt_mv = 0;
  bool input_valid = false;

  if (this->read_voltage_twice_(&M5PM1::readVbat, &battery_mv)) {
    if (this->battery_voltage_sensor_ != nullptr) {
      this->battery_voltage_sensor_->publish_state(battery_mv / 1000.0f);
    }
    if (this->battery_level_sensor_ != nullptr) {
      this->battery_level_sensor_->publish_state(this->estimate_battery_level_(battery_mv));
    }
  } else {
    ESP_LOGW(TAG, "readVbat failed");
  }

  if (this->read_voltage_twice_(&M5PM1::readVin, &input_mv)) {
    input_valid = true;
    if (this->input_voltage_sensor_ != nullptr) {
      this->input_voltage_sensor_->publish_state(input_mv / 1000.0f);
    }
  } else {
    ESP_LOGW(TAG, "readVin failed");
  }

  if (this->read_voltage_twice_(&M5PM1::read5VInOut, &five_volt_mv)) {
    if (this->five_volt_voltage_sensor_ != nullptr) {
      this->five_volt_voltage_sensor_->publish_state(five_volt_mv / 1000.0f);
    }
  } else {
    ESP_LOGW(TAG, "read5VInOut failed");
  }

  if (this->charging_binary_sensor_ != nullptr) {
    m5pm1_pwr_src_t power_source = M5PM1_PWR_SRC_UNKNOWN;
    const bool source_valid = this->pm1_.getPowerSource(&power_source) == M5PM1_OK;
    const bool charging = (source_valid && power_source != M5PM1_PWR_SRC_BAT && power_source != M5PM1_PWR_SRC_UNKNOWN) ||
                          (input_valid && input_mv >= 4500);
    this->charging_binary_sensor_->publish_state(charging);
  }

  this->publish_ext_5v_state_();
}

void M5StickS3Power::set_ext_5v(bool state) {
  if (!this->resync_pmic_()) {
    return;
  }

  const m5pm1_err_t err = this->pm1_.setBoostEnable(state);
  if (err != M5PM1_OK) {
    ESP_LOGW(TAG, "setBoostEnable(%s) failed: %d", state ? "true" : "false", err);
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

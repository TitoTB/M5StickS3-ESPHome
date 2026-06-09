#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "m5sticks3_power_common.h"

namespace esphome {
namespace m5sticks3_power {

class M5StickS3Power;

class M5StickS3Ext5VSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(M5StickS3Power *parent) { this->parent_ = parent; }
  void write_state(bool state) override;

 protected:
  M5StickS3Power *parent_{nullptr};
};

class M5StickS3Power : public PollingComponent {
 public:
  void set_i2c_pins(int sda, int scl) {
    this->sda_pin_ = sda;
    this->scl_pin_ = scl;
  }
  void set_battery_level_sensor(sensor::Sensor *sensor) { this->battery_level_sensor_ = sensor; }
  void set_battery_voltage_sensor(sensor::Sensor *sensor) { this->battery_voltage_sensor_ = sensor; }
  void set_battery_current_sensor(sensor::Sensor *sensor) { this->battery_current_sensor_ = sensor; }
  void set_charging_binary_sensor(binary_sensor::BinarySensor *sensor) { this->charging_binary_sensor_ = sensor; }
  void set_ext_5v_switch(M5StickS3Ext5VSwitch *sw) { this->ext_5v_switch_ = sw; }

  void setup() override {
    ensure_m5sticks3_power_begin(this->sda_pin_, this->scl_pin_);
    this->publish_ext_5v_state_();
  }

  void update() override {
    M5.update();

    const int32_t level = M5.Power.getBatteryLevel();
    if (this->battery_level_sensor_ != nullptr && level >= 0) {
      this->battery_level_sensor_->publish_state(level);
    }

    const int16_t battery_mv = M5.Power.getBatteryVoltage();
    if (this->battery_voltage_sensor_ != nullptr && battery_mv > 0) {
      this->battery_voltage_sensor_->publish_state(battery_mv / 1000.0f);
    }

    const int32_t battery_ma = M5.Power.getBatteryCurrent();
    if (this->battery_current_sensor_ != nullptr) {
      this->battery_current_sensor_->publish_state(battery_ma / 1000.0f);
    }

    if (this->charging_binary_sensor_ != nullptr) {
      this->charging_binary_sensor_->publish_state(static_cast<bool>(M5.Power.isCharging()));
    }

    this->publish_ext_5v_state_();
  }

  void set_ext_5v(bool state) {
    M5.Power.setExtOutput(state);
    this->publish_ext_5v_state_();
  }

  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void dump_config() override {
    ESP_LOGCONFIG(TAG, "M5StickS3 Power");
    LOG_SENSOR("  ", "Battery level", this->battery_level_sensor_);
    LOG_SENSOR("  ", "Battery voltage", this->battery_voltage_sensor_);
    LOG_SENSOR("  ", "Battery current", this->battery_current_sensor_);
    LOG_BINARY_SENSOR("  ", "Charging", this->charging_binary_sensor_);
    LOG_SWITCH("  ", "EXT 5V", this->ext_5v_switch_);
  }

 protected:
  void publish_ext_5v_state_() {
    if (this->ext_5v_switch_ != nullptr) {
      this->ext_5v_switch_->publish_state(M5.Power.getExtOutput());
    }
  }

  int sda_pin_{47};
  int scl_pin_{48};
  sensor::Sensor *battery_level_sensor_{nullptr};
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *battery_current_sensor_{nullptr};
  binary_sensor::BinarySensor *charging_binary_sensor_{nullptr};
  M5StickS3Ext5VSwitch *ext_5v_switch_{nullptr};
  static constexpr const char *TAG = "m5sticks3_power";
};

inline void M5StickS3Ext5VSwitch::write_state(bool state) {
  if (this->parent_ != nullptr) {
    this->parent_->set_ext_5v(state);
  } else {
    M5.Power.setExtOutput(state);
    this->publish_state(M5.Power.getExtOutput());
  }
}

}  // namespace m5sticks3_power
}  // namespace esphome

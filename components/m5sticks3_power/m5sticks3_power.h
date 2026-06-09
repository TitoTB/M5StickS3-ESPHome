#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include <M5PM1.h>
#include <Wire.h>

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
  void set_input_voltage_sensor(sensor::Sensor *sensor) { this->input_voltage_sensor_ = sensor; }
  void set_five_volt_voltage_sensor(sensor::Sensor *sensor) { this->five_volt_voltage_sensor_ = sensor; }
  void set_charging_binary_sensor(binary_sensor::BinarySensor *sensor) { this->charging_binary_sensor_ = sensor; }
  void set_ext_5v_switch(M5StickS3Ext5VSwitch *sw) { this->ext_5v_switch_ = sw; }

  void setup() override;
  void update() override;
  void set_ext_5v(bool state);

  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void dump_config() override;

 protected:
  bool init_pmic_();
  bool resync_pmic_();
  void publish_ext_5v_state_();
  float estimate_battery_level_(uint16_t battery_mv);
  bool read_voltage_twice_(m5pm1_err_t (M5PM1::*reader)(uint16_t *), uint16_t *mv);

  int sda_pin_{47};
  int scl_pin_{48};
  uint8_t address_{0x6E};
  bool pmic_ready_{false};
  bool boost_enabled_{false};
  M5PM1 pm1_;

  sensor::Sensor *battery_level_sensor_{nullptr};
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *input_voltage_sensor_{nullptr};
  sensor::Sensor *five_volt_voltage_sensor_{nullptr};
  binary_sensor::BinarySensor *charging_binary_sensor_{nullptr};
  M5StickS3Ext5VSwitch *ext_5v_switch_{nullptr};
  static constexpr const char *TAG = "m5sticks3_power";
};

inline void M5StickS3Ext5VSwitch::write_state(bool state) {
  if (this->parent_ != nullptr) {
    this->parent_->set_ext_5v(state);
  } else {
    this->publish_state(state);
  }
}

}  // namespace m5sticks3_power
}  // namespace esphome

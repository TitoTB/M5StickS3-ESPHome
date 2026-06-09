#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"

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

class M5StickS3Power : public PollingComponent, public i2c::I2CDevice {
 public:
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
  void publish_ext_5v_state_();
  float estimate_battery_level_(uint16_t battery_mv);
  bool read_voltage_(uint8_t low_reg, uint16_t *mv);
  bool read_power_source_(uint8_t *source);
  bool update_bits_(uint8_t reg, uint8_t mask, uint8_t value);

  bool pmic_ready_{false};
  bool boost_enabled_{false};

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

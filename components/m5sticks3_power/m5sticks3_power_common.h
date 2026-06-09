#pragma once

#include <M5Unified.h>
#include <Wire.h>

namespace esphome {
namespace m5sticks3_power {

inline void ensure_m5sticks3_power_begin(int sda, int scl) {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  auto config = M5.config();
  config.serial_baudrate = 0;
  config.clear_display = false;
  config.internal_imu = true;
  config.internal_rtc = false;
  config.output_power = true;
  M5.begin(config);

  Wire.end();
  Wire.begin(sda, scl, 400000U);
  M5.Power.begin();

  initialized = true;
}

}  // namespace m5sticks3_power
}  // namespace esphome

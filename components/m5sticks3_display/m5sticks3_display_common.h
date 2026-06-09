#pragma once

#include <M5Unified.h>
#include <Wire.h>

namespace esphome {
namespace m5sticks3_display {

inline void ensure_m5sticks3_display_begin(int sda, int scl) {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  auto config = M5.config();
  config.serial_baudrate = 0;
  config.clear_display = false;
  config.output_power = true;
  M5.begin(config);

  Wire.end();
  Wire.begin(sda, scl, 400000U);
  M5.Power.begin();

  initialized = true;
}

}  // namespace m5sticks3_display
}  // namespace esphome

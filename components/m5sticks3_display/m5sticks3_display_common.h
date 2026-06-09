#pragma once

#include <M5Unified.h>

namespace esphome {
namespace m5sticks3_display {

inline void ensure_m5sticks3_display_begin(int sda, int scl) {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  (void) sda;
  (void) scl;

  auto config = M5.config();
  config.serial_baudrate = 0;
  config.clear_display = false;
  config.internal_imu = false;
  config.internal_rtc = false;
  config.internal_mic = false;
  config.internal_spk = false;
  config.external_imu = false;
  config.external_rtc = false;
  config.output_power = false;
  M5.begin(config);

  initialized = true;
}

}  // namespace m5sticks3_display
}  // namespace esphome

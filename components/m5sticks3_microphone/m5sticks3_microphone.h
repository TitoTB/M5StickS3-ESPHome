#pragma once

#include "esphome/components/microphone/microphone.h"
#include "esphome/core/component.h"

#include <M5Unified.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <cstdint>

namespace esphome {
namespace m5sticks3_microphone {

class M5StickS3Microphone : public microphone::Microphone, public Component {
 public:
  void setup() override;
  void dump_config() override;
  void start() override;
  void stop() override;

  void set_sample_rate(uint32_t sample_rate) { this->sample_rate_ = sample_rate; }
  void set_buffer_duration(uint32_t buffer_duration_ms) { this->buffer_duration_ms_ = buffer_duration_ms; }

 protected:
  static void mic_task(void *params);
  void run_();

  uint32_t sample_rate_{16000};
  uint32_t buffer_duration_ms_{20};
  volatile bool stop_requested_{false};
  TaskHandle_t task_handle_{nullptr};
};

}  // namespace m5sticks3_microphone
}  // namespace esphome

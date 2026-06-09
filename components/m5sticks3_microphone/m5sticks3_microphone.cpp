#include "m5sticks3_microphone.h"
#include "esphome/core/log.h"

#include <algorithm>
#include <cinttypes>
#include <cstring>
#include <vector>

namespace esphome {
namespace m5sticks3_microphone {

static const char *const TAG = "m5sticks3_microphone";

void M5StickS3Microphone::setup() {
  this->audio_stream_info_ = audio::AudioStreamInfo(16, 1, this->sample_rate_);
}

void M5StickS3Microphone::dump_config() {
  ESP_LOGCONFIG(TAG, "M5StickS3 Microphone (M5Unified/M5.Mic)");
  ESP_LOGCONFIG(TAG, "  Sample rate: %" PRIu32 " Hz", this->sample_rate_);
  ESP_LOGCONFIG(TAG, "  Buffer duration: %" PRIu32 " ms", this->buffer_duration_ms_);
}

void M5StickS3Microphone::start() {
  if (this->state_ == microphone::STATE_RUNNING || this->state_ == microphone::STATE_STARTING) {
    return;
  }

  ESP_LOGI(TAG, "Starting M5.Mic capture");
  this->state_ = microphone::STATE_STARTING;
  this->stop_requested_ = false;

  // StickS3 audio cannot use speaker and microphone at the same time.
  M5.Speaker.end();

  if (!M5.Mic.begin()) {
    ESP_LOGE(TAG, "M5.Mic.begin() failed");
    this->state_ = microphone::STATE_STOPPED;
    this->status_set_error("M5.Mic.begin failed");
    return;
  }

  this->status_clear_error();
  const BaseType_t result = xTaskCreatePinnedToCore(
      M5StickS3Microphone::mic_task, "m5sticks3_mic", 4096, this, 5, &this->task_handle_, 1);
  if (result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create microphone task");
    M5.Mic.end();
    this->task_handle_ = nullptr;
    this->state_ = microphone::STATE_STOPPED;
    this->status_set_error("task create failed");
    return;
  }

  this->state_ = microphone::STATE_RUNNING;
}

void M5StickS3Microphone::stop() {
  if (this->state_ == microphone::STATE_STOPPED || this->state_ == microphone::STATE_STOPPING) {
    return;
  }

  ESP_LOGI(TAG, "Stopping M5.Mic capture");
  this->state_ = microphone::STATE_STOPPING;
  this->stop_requested_ = true;
}

void M5StickS3Microphone::mic_task(void *params) {
  auto *mic = static_cast<M5StickS3Microphone *>(params);
  mic->run_();
  vTaskDelete(nullptr);
}

void M5StickS3Microphone::run_() {
  const uint32_t samples_per_chunk = std::max<uint32_t>(1, this->sample_rate_ * this->buffer_duration_ms_ / 1000U);
  std::vector<int16_t> samples(samples_per_chunk);
  std::vector<uint8_t> bytes(samples_per_chunk * sizeof(int16_t));

  while (!this->stop_requested_) {
    if (!M5.Mic.record(samples.data(), samples.size(), this->sample_rate_)) {
      delay(1);
      continue;
    }

    std::memcpy(bytes.data(), samples.data(), bytes.size());
    this->data_callbacks_.call(bytes);
  }

  M5.Mic.end();
  this->task_handle_ = nullptr;
  this->state_ = microphone::STATE_STOPPED;
}

}  // namespace m5sticks3_microphone
}  // namespace esphome

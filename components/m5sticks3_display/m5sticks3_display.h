#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/core/hal.h"
#include "../m5sticks3_common/m5sticks3_common.h"

namespace esphome {
namespace m5sticks3_display {

class M5StickS3Display : public PollingComponent, public display::DisplayBuffer {
 public:
  void set_i2c_pins(GPIOPin *sda, GPIOPin *scl) {
    this->sda_pin_ = sda;
    this->scl_pin_ = scl;
  }

  void setup() override {
    const int sda = this->pin_number_(this->sda_pin_, 47);
    const int scl = this->pin_number_(this->scl_pin_, 48);
    m5sticks3_common::ensure_m5sticks3_begin(sda, scl);

    M5.Display.setRotation(1);
    M5.Display.setBrightness(96);
    M5.Display.fillScreen(TFT_BLACK);
  }

  void update() override {
    this->do_update_();
  }

  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void dump_config() override {
    ESP_LOGCONFIG(TAG, "M5StickS3 Display");
    ESP_LOGCONFIG(TAG, "  Size: %ux%u", this->get_width_internal(), this->get_height_internal());
  }

 protected:
  int get_width_internal() override { return 240; }
  int get_height_internal() override { return 135; }

  void draw_absolute_pixel_internal(int x, int y, Color color) override {
    if (x < 0 || y < 0 || x >= this->get_width_internal() || y >= this->get_height_internal()) {
      return;
    }
    M5.Display.drawPixel(x, y, color.to_rgb_565());
  }

  int pin_number_(GPIOPin *pin, int fallback) {
    if (pin == nullptr) {
      return fallback;
    }
    return pin->get_pin();
  }

  GPIOPin *sda_pin_{nullptr};
  GPIOPin *scl_pin_{nullptr};
  static constexpr const char *TAG = "m5sticks3_display";
};

}  // namespace m5sticks3_display
}  // namespace esphome

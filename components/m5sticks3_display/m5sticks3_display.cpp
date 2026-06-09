#include "m5sticks3_display.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5sticks3_display {

static const char *const TAG = "m5sticks3_display";

void M5StickS3Display::setup() {
  m5sticks3_common::ensure_m5sticks3_begin(this->sda_pin_, this->scl_pin_);
  delay(150);

  M5.Display.begin();
  delay(120);

  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_BLACK);

  const int width = M5.Display.width();
  const int height = M5.Display.height();

  this->canvas_.setColorDepth(16);
  if (!this->canvas_.createSprite(width, height)) {
    ESP_LOGE(TAG, "Failed to allocate canvas sprite %dx%d", width, height);
    return;
  }

  this->canvas_.fillScreen(TFT_BLACK);
  this->canvas_.pushSprite(0, 0);
  this->initialized_ = true;

  ESP_LOGI(TAG, "M5StickS3 display initialized: %dx%d", width, height);
}

void M5StickS3Display::update() {
  if (!this->initialized_) {
    return;
  }

  this->do_update_();
  this->canvas_.pushSprite(0, 0);
}

void M5StickS3Display::dump_config() {
  ESP_LOGCONFIG(TAG, "M5StickS3 Display (M5Unified/M5GFX)");
  LOG_UPDATE_INTERVAL(this);

  if (this->initialized_) {
    ESP_LOGCONFIG(TAG, "  Width: %d", M5.Display.width());
    ESP_LOGCONFIG(TAG, "  Height: %d", M5.Display.height());
  } else {
    ESP_LOGCONFIG(TAG, "  Not initialized");
  }
}

int M5StickS3Display::get_width_internal() {
  return this->initialized_ ? M5.Display.width() : 240;
}

int M5StickS3Display::get_height_internal() {
  return this->initialized_ ? M5.Display.height() : 135;
}

void M5StickS3Display::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (!this->initialized_) {
    return;
  }
  const uint16_t rgb565 = this->canvas_.color565(color.r, color.g, color.b);
  this->canvas_.drawPixel(x, y, rgb565);
}

}  // namespace m5sticks3_display
}  // namespace esphome

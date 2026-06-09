#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "m5sticks3_display_common.h"

namespace esphome {
namespace m5sticks3_display {

class M5StickS3Display : public display::DisplayBuffer {
 public:
  void set_i2c_pins(int sda, int scl) {
    this->sda_pin_ = sda;
    this->scl_pin_ = scl;
  }

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }

 protected:
  int get_width_internal() override;
  int get_height_internal() override;
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  int sda_pin_{47};
  int scl_pin_{48};
  bool initialized_{false};
  M5Canvas canvas_{&M5.Display};
};

}  // namespace m5sticks3_display
}  // namespace esphome

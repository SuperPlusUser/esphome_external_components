#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/display/display_buffer.h"

namespace esphome {
namespace s1d15721_spi {

enum s1d15721Model {
  s1d15721_MODEL_240_64 = 0,
};

class SPIs1d15721 : public PollingComponent, 
                    public display::DisplayBuffer,
                    public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH, 
                                          spi::CLOCK_PHASE_TRAILING, spi::DATA_RATE_8MHZ> {
public:
  void set_model(s1d15721Model model) { this->model_ = model; }
  void set_dc_pin(GPIOPin *dc_pin) { this->dc_pin_ = dc_pin; }
  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }
  void init_invert(bool invert) { this->invert_ = invert; }
  void set_invert(bool invert);
  
  bool is_inverted() { return this->invert_; }
  bool is_on() { return this->is_on_; }
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  void setup() override;
  void update() override;
  void dump_config() override;
  void turn_on();
  void turn_off();
  void fill(Color color) override;

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_GRAYSCALE; } // grayscale mode (even if there are only 4 levels...)

 protected:
  void command(uint8_t cmd); 
  void command_arg(uint8_t cmd, uint8_t arg);
  void command_2arg(uint8_t cmd, uint8_t arg1, uint8_t arg2);
  void data(uint8_t value);
  void write_display_data();
  void init_reset_();
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  int get_height_internal() override;
  int get_width_internal() override;
  int get_page_offset();
  int get_col_offset();
  size_t get_buffer_length_();
  const char *model_str_();

  s1d15721Model model_{s1d15721_MODEL_240_64};
  GPIOPin *dc_pin_;
  GPIOPin *reset_pin_{nullptr};
  bool is_on_{false};
  bool invert_{false};
};

}  // namespace s1d15721_spi
}  // namespace esphome

#include "s1d15721_spi.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace s1d15721_spi {

static const char *const TAG = "s1d15721_spi";

static const uint8_t s1d15721_PIXELSPERBYTE = 4; // in greyscale mode 4, in binary mode 8, we use grayscale mode here because we can :)

// Display Commands:
static const uint8_t s1d15721_DATA_WRITE         = 0x1D;  // Display Data Write (n-bytes)
static const uint8_t s1d15721_PAGE_ADDR          = 0xB1;  // Page Address Set (2 byte)
static const uint8_t s1d15721_COL_ADDR           = 0x13;  // Column Address Set (2 byte)
static const uint8_t s1d15721_DISPLAY_OFFON      = 0xAE;  // Display ON/OFF: 0=Off/1=On
static const uint8_t s1d15721_DISPLAY_REVERSE    = 0xA6;  // Display/Normal: 0=Normal/1=Reverse
static const uint8_t s1d15721_DISPLAY_ALL_LIGHT  = 0xA4;  // Display All Lighting: 0=Normal/1=All On
static const uint8_t s1d15721_DISPLAY_START_LINE = 0x8A;  // Display Start Line (2/3 byte)
static const uint8_t s1d15721_DISPLAY_MODE       = 0x66;  // Display Mode (2 byte): 0=4 gray-scale, 1=binary
static const uint8_t s1d15721_GRAY_SCALE_PATTERN = 0x39;  // Gray Scale Pattern (2 byte)
static const uint8_t s1d15721_COM_OUT_STATUS     = 0xC4;  // Common Output Status Select: 0=Normal/1=Reverse
static const uint8_t s1d15721_DUTY_SET           = 0x6D;  // Duty Set Command (3 byte)
static const uint8_t s1d15721_BUILTIN_OSC        = 0xAA;  // Built-in Oscillator Circuit: [0]:0=Off/1=On
static const uint8_t s1d15721_BUILTIN_OSC_FREQ   = 0x5F;  // Built-in Oscillator Circuit Frequency Select (2 byte)
static const uint8_t s1d15721_PWR_CONTROL        = 0x25;  // Power Control Set (2 byte)
static const uint8_t s1d15721_LC_DRIVE_VOLTAGE   = 0x2B;  // Liquid Crystal Drive Voltage Select (2 byte)
static const uint8_t s1d15721_EL_VOLUME          = 0x81;  // Electronic Volume Set (2 byte)
static const uint8_t s1d15721_RESET              = 0xE2;  // Software Reset

void SPIs1d15721::setup() {
  ESP_LOGCONFIG(TAG, "Setting up SPI display s1d15721...");
  this->init_internal_(this->get_buffer_length_());
  this->spi_setup();
  this->dc_pin_->setup();  // OUTPUT

  this->init_reset_();
  delay(100);

  //Display INIT-Sequence:
  this->command(s1d15721_COM_OUT_STATUS | 1);       	  /* (5) Common Output Status (Reverse) */
  this->command(s1d15721_DISPLAY_REVERSE | this->invert_); /* (3) Display Normal (0) / Reverse (1) */
  this->command(s1d15721_DISPLAY_ALL_LIGHT | 0); 	      /* (4) Display All Light (Normal) */
  this->command_2arg(s1d15721_DUTY_SET, 0x10, 0x02); 		/* (18) Duty Set Command */
  this->command_arg(s1d15721_DISPLAY_MODE, 0x00);       /* (14) Display Mode, Parameter 0 (0 = Gray Scale 1 = Binary) */
  this->command_arg(s1d15721_GRAY_SCALE_PATTERN, 0x36); /* (15) Gray Scale Pattern Set, Pattern */
  this->command_arg(s1d15721_LC_DRIVE_VOLTAGE, 0x07);		/* (27) LCD Drive Mode Voltage Select, Parameter */
  this->command_arg(s1d15721_EL_VOLUME, 0x0a);		      /* (28) Electronic Volume, Parameter */
  this->command_arg(s1d15721_BUILTIN_OSC_FREQ, 0x00);		/* (24) Built-in Oscillator Frequency, Parameter  */
  this->command(s1d15721_BUILTIN_OSC | 1);			        /* (23) Built-in OSC On */  
  this->command_arg(s1d15721_PWR_CONTROL, 0x1F);		    /* (25) Power Control Set, Parameter  */
  this->command_arg(s1d15721_DISPLAY_START_LINE, 0x00);	/* (6) Start Line Setup, Parameter  */
  this->command_arg(s1d15721_PAGE_ADDR, 0x00);		      /* (7) Page Address Set, Parameter  */
  this->command_arg(s1d15721_COL_ADDR, 0x00);		        /* (8) Column Address Set  */
  
  this->fill(Color::BLACK);      // clear display - ensures we do not see garbage at power-on
  this->write_display_data();    // ...write buffer, which actually clears the display's memory
  this->turn_on();               // display ON
}

void SPIs1d15721::update() {
  this->do_update_();
  this->write_display_data();
}

void SPIs1d15721::dump_config() {
  LOG_DISPLAY("", "SPI s1d15721", this);
  ESP_LOGCONFIG(TAG, "  Model: %s", this->model_str_());
  if (this->cs_)
    LOG_PIN("  CS Pin: ", this->cs_);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  if (this->reset_pin_)
    LOG_PIN("  Reset Pin: ", this->reset_pin_);
  //ESP_LOGCONFIG(TAG, "  Internal Buffer Length: %u", this->get_buffer_length_());
  LOG_UPDATE_INTERVAL(this);
}

void SPIs1d15721::turn_on() {
  this->command(s1d15721_DISPLAY_OFFON|1);
  this->is_on_ = true;
}

void SPIs1d15721::turn_off() {
  this->command(s1d15721_DISPLAY_OFFON|0);
  this->is_on_ = false;
}

void SPIs1d15721::fill(Color color) { 
  // grayscale mode:
  uint8_t gray_level = color.white >> 6;
  uint8_t gray_level_byte = gray_level | gray_level << 2 | gray_level << 4 | gray_level << 6;
  memset(this->buffer_, gray_level_byte, this->get_buffer_length_()); 
  
  // for binary mode this would be enough:
  //memset(this->buffer_, color.is_on() ? 0xFF : 0x00, this->get_buffer_length_());
}

void SPIs1d15721::command(uint8_t cmd) {
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(cmd);
  this->disable();
}

void SPIs1d15721::command_arg(uint8_t cmd, uint8_t arg) {
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(cmd);
  this->dc_pin_->digital_write(true);
  this->write_byte(arg);
  this->disable();
}

void SPIs1d15721::command_2arg(uint8_t cmd, uint8_t arg1, uint8_t arg2) {
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(cmd);
  this->dc_pin_->digital_write(true);
  this->write_byte(arg1);
  this->write_byte(arg2);
  this->disable();
}

void SPIs1d15721::data(uint8_t value) {
  this->dc_pin_->digital_write(true);
  this->enable();
  this->write_byte(value);
  this->disable();
}

void SPIs1d15721::set_invert(bool invert) {
  this->invert_ = invert;
  this->command(s1d15721_DISPLAY_REVERSE | this->invert_);
}

void HOT SPIs1d15721::write_display_data() {
  // write buffer to display ram
  // - binary mode: 8 Pages a 240 columns
  // - greyscale mode: 16 Pages a 240 columns
  int width = this->get_width_internal();
  int pages = this->get_height_internal() / s1d15721_PIXELSPERBYTE;

  this->enable();
  for(uint8_t page = 0; page < pages; page++) { 
    // set column
    this->dc_pin_->digital_write(false);
    this->write_byte(s1d15721_COL_ADDR);
    this->dc_pin_->digital_write(true);
    this->write_byte(this->get_col_offset());
    // set page
    this->dc_pin_->digital_write(false);
    this->write_byte(s1d15721_PAGE_ADDR);
    this->dc_pin_->digital_write(true);
    this->write_byte( (page + this->get_page_offset()) & 0x1F );
    // write data
    this->dc_pin_->digital_write(false);
    this->write_byte(s1d15721_DATA_WRITE);
    this->dc_pin_->digital_write(true);

    // write one page to the lcd ram
    for (uint8_t col = 0; col < width; col++) {
      this->write_byte(this->buffer_[page*width + col]);
    }
  }
  this->disable();
}

void SPIs1d15721::init_reset_() {
  // Hardware Reset:
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(true);
    delay(1);
    // Trigger Reset
    this->reset_pin_->digital_write(false);
    delay(10);
    // Wake up
    this->reset_pin_->digital_write(true);
  }
  // Software Reset:
  this->command(s1d15721_RESET);
  delay(1);
}

void HOT SPIs1d15721::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || x < 0 || y >= this->get_height_internal() || y < 0) {
    ESP_LOGD(TAG, "Position out of area: %dx%d", x, y);
    return;
  }
  int width = this->get_width_internal();
  
  // TODO: convert color to grayscale:
  //uint32_t color4 = display::ColorUtil::color_to_grayscale4(color);

  // color.white to 2 bit gray level -> only use the two most significant bits:
  uint8_t gray_level = color.white >> 6;
  uint16_t pos = (x + ((y>>2)*width));

  // set relevant 2 bits to zero first
  this->buffer_[pos] &= ~(  0x03      << ((y<<1)&7) );
  // then override with selected gray_level
  this->buffer_[pos] |=  ( gray_level << ((y<<1)&7) );
}

size_t SPIs1d15721::get_buffer_length_() {
  return size_t(this->get_width_internal()) * size_t(this->get_height_internal()) / s1d15721_PIXELSPERBYTE;
}

// the following 5 methods have to be modified to add additional display types controlled by this chip:

int SPIs1d15721::get_height_internal() {
  switch (this->model_) {
    case s1d15721_MODEL_240_64:
      return 64;
    default:
      return 0;
  }
}

int SPIs1d15721::get_width_internal() {
  switch (this->model_) {
    case s1d15721_MODEL_240_64:
      return 240;
    default:
      return 0;
  }
}

int SPIs1d15721::get_page_offset() {
  switch (this->model_) {
    case s1d15721_MODEL_240_64:
      return 0;
    default:
      return 0;
  }
}

int SPIs1d15721::get_col_offset() {
  switch (this->model_) {
    case s1d15721_MODEL_240_64:
      return 8;
    default:
      return 0;
  }
}

const char *SPIs1d15721::model_str_() {
  switch (this->model_) {
    case s1d15721_MODEL_240_64:
      return "S1D15721 240x64";
    default:
      return "Unknown";
  }
}

}  // namespace s1d15721_spi
}  // namespace esphome

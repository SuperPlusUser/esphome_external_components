# s1d15721_spi

This component allows you to use displays controlled by the S1D15721 chip in ESPHome. The display must be connected via 4-Wire SPI bus. 
It can display 4 gray levels.

## Supported displays

### VARITRONIX VL-FS-COG-VLGEM1277-01, 240x64 pixel (`S1D15721 240x64`)
At the moment this is the only supported display (and the only one I know of with this controller). It is currently available in the German online shop [Pollin.de](https://www.pollin.de/p/varitronix-lcd-cog-vlgem1277-01-240x64-pixel-121713) for less than €1. Unfortunately it's not really "plug and play" as it uses an FPC connector and requires some external capacitors.

[This](https://www.mikrocontroller.net/topic/472549) Thread on Mikrocontroller.net has some examples of how to connect the display (in German). I also found [this](https://www.shotech.de/de/lcd-vlgem1277-01-adapter-board-3-3v-vcc.html) pre-soldered adapter board for the display, which looks useful but I haven't tested it yet.

Note: The linked offers serve only as an example. I am not affiliated with the sellers or manufacturers.

## Usage

### Example configuration entry: 

```yaml
external_components:
  - source: github://SuperPlusUser/esphome_external_components
    components: [ s1d15721_spi ]

spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23

display:
  - platform: s1d15721_spi
    model: "S1D15721 240x64"
    dc_pin: GPIO17
    reset_pin: GPIO19
    cs_pin: GPIO5
    lambda: |-
        it.print(5, 0, id(font1), "Hello World!");

font:
  - file: 
      type: gfonts
      family: Ubuntu
      weight: 400
    id: font1
    size: 16
```

For a more complex example see [example.yaml](../../example.yaml)

### Configuration variables:

- **model** (Required, string): Defines the display model to use. See supported displays above.
- **dc_pin**  (Required, pin): The DC pin (=A0).
- **reset_pin**  (Optional, pin): The RESET pin of the display can also be permanently connected to 3.3V.
- **cs_pin**  (Optional, pin): The pin on the ESP that the CS line is connected to. The CS line can be connected to GND if this is the only device on the SPI bus.
- **lambda**  (Optional, lambda): The lambda to use for rendering the content on the display. See [Display Rendering Engine](https://esphome.io/components/display/index.html#display-engine) for more information.
- **update_interval** (Optional, Time): The interval to re-draw the screen. Defaults to 5s.
- **invert** (Optional, boolean): Invert all pixel state on the display. Defaults to false. You can also invert the display at runtime using the `.set_invert(true)` function in lambda (see [example.yaml](../../example.yaml)).
- **rotation** (Optional): Set the rotation of the display. Everything you draw in lambda: will be rotated by this option. One of 0° (default), 90°, 180°, 270°.
- **pages** (Optional, list): Show pages instead of a single lambda. See [Display Pages](https://esphome.io/components/display/index.html#display-pages).

### Gray levels

The controller supports 4 gray levels (2 bits): COLOR_OFF (= black), dark_gray, light_gray and COLOR_ON (= white). COLOR_ON and COLOR_OFF are predefined, the other 2 "colors" have to be defined in yaml like this:
```yaml
color:
  - id: light_gray
    white: 30% # anything between 25% and 50% results in light gray
  - id: dark_gray
    white: 70% # anything between 50% and 75% results in dark gray
```

Technically, the two most significant bits of the `color.white` value are used to determine the gray level. So in theory it should also be possible to display pictures on the display, but I have not testetd this and I would not expect good looking results.

## Requirements

- ESPHome 2023.12.0 or higher


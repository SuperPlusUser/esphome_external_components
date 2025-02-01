# My Custom Components for ESPHome

This repository is a collection of my custom components for [ESPHome](https://esphome.io/)

## Usage

To use the components provided by this repository, simply add this to your `.yaml` definition:

```yaml
external_components:
  - source: github://SuperPlusUser/esphome_external_components
```

More information can be found [here](https://esphome.io/components/external_components.html).

## Components

### [s1d15721_spi](./components/s1d15721_spi/)

Display component to use LC displays controlled by the S1D15721 controller IC in ESPHome. For Example, this IC is used in [this](https://www.pollin.de/p/varitronix-lcd-cog-vlgem1277-01-240x64-pixel-121713) cheap display from Pollin Electronic (VARITRONIX VL-FS-COG-VLGEM1277-01, 240x64 pixel).
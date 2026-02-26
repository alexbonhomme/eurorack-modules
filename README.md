# eurorack-modules

Open-source Eurorack module designs: firmware, schematics, and documentation. Projects use Arduino, PlatformIO, and various MCUs (Raspberry Pi Pico, Teensy, Daisy, etc.).

## Overview

This repository contains multiple standalone module projects. Each module lives in its own folder with its own build system (PlatformIO or Arduino). Build and upload from inside each module directory.

## Modules

### Pico-based (RP2040 / PlatformIO)

- **[Pico Scope](pico_scope/)** — Dual-channel oscilloscope. Raspberry Pi Pico + SH1106 128×64 display (SPI/I2C), U8g2. Two ADC inputs, time/div and volts/div pots. Eurorack power.
- **[Pico Drum](pico_drum/)** — Sample-playback drum voice. RP2040 Zero (or Pico), gate + pitch + decay CV, PWM audio out.
- **[Pico MIDI](pico_midi/)** — MIDI to CV. 4× 1V/oct CV/gate (MCP4822), clock outputs, accent/slide on channel 1. Pico or Pico 2.

### Other

- **[LFO/VCO](vco_lfo/)** — Arduino UNO + AD9833.
- **[VCO3](vco3/)** — Teensy + Audio Library.
- **[Saturator](saturator/)** — LED-based soft clipping.
- **[Stereo Filters](stereo_filters/)** — Daisy Seed.
- **[MS20 Low Pass Filter](ms20_lpf/)** — WIP.

## Build (PlatformIO modules)

From the repo root:

```bash
cd pico_scope && pio run && cd ..
cd pico_drum  && pio run && cd ..
cd pico_midi  && pio run && cd ..
```

Upload: `pio run --target upload` inside the module folder.

## License

This work is licensed under a [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](https://creativecommons.org/licenses/by-nc-sa/4.0/).

[![CC BY-NC-SA 4.0](https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png)](https://creativecommons.org/licenses/by-nc-sa/4.0/)

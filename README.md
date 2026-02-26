# eurorack-modules

Open-source Eurorack module designs: firmware, schematics, and documentation. Projects use Arduino, PlatformIO, and various MCUs (Raspberry Pi Pico, Teensy, Daisy, etc.).

## Overview

This repository contains multiple standalone module projects. Each module lives in its own folder with its own build system (PlatformIO or Arduino). Build and upload from inside each module directory.

## Modules

### Pico-based (RP2040 / PlatformIO)

- **[Pico Scope](pico_scope/)** — Dual-channel scope, Pico + SH1106, time/volts per div.
- **[Pico Drum](pico_drum/)** — Sample drum voice; gate, pitch, decay CV; PWM out.
- **[Pico MIDI](pico_midi/)** — MIDI to CV; 4× CV/gate, clocks, accent/slide on ch1.

### Other

- **[VCO/LFO](vco_lfo/)** — Arduino UNO + AD9833; sine/tri/square, 1V/oct.
- **[VCO3](vco3/)** — Triple VCO + envelope; Teensy + Audio Library.
- **[Saturator](saturator/)** — LED soft clipping + Baxandall tone (analog).
- **[Stereo Filters](stereo_filters/)** — Daisy Seed; stereo ladder filters.
- **[MS20 LPF](ms20_lpf/)** — MS20-style VCF (analog, WIP).

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

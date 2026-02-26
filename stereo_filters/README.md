# Stereo Filters

Stereo filter module based on Daisy Seed. Four ladder filters (Huovilainen-style, ported from Teensy Audio Library) run in stereo: two LP12 and two BP12, with shared cutoff and resonance and stereo spread from pots and optional modulation.

## Features

- **Daisy Seed** — Arduino/DaisyDuino environment; 48 kHz audio
- **Four filters** — Two LP12 and two BP12; L+R each get one LP and one BP (BP at 0.5 mix)
- **Controls** — Cutoff (POT_1), spread L (POT_2), spread R (POT_3), resonance (POT_4)
- **Modulation** — Smooth random generators can modulate filter frequencies for movement
- **I/O** — Stereo in/out via Daisy Seed audio pins

## Build

Use the Arduino IDE with [DaisyDuino](https://github.com/electro-smith/DaisyDuino) and select Daisy Seed. Open `stereo_filters.ino`, compile and upload.

## Pinout

| Function | Pin  |
|----------|------|
| POT_1    | A0 (cutoff) |
| POT_2    | A1 (spread L) |
| POT_3    | A2 (spread R) |
| POT_4    | A3 (resonance) |

Filter routing and coefficients are in `stereo_filters.ino` and `ladder.cpp` / `ladder.h`.

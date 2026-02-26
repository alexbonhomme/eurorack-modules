# Pico Drum

Sample-playback drum voice for Eurorack. Uses an RP2040-based board (RP2040 Zero or Raspberry Pi Pico). No SD card — the sample is compiled in from a C header. Gate input triggers playback; pitch and decay are controlled by CV/pots. Audio output is PWM.

## Features

- Single sample playback, triggered by gate
- Pitch control (pot/CV) for playback speed
- Decay envelope (pot) for amplitude fade
- PWM audio output; 44.1 kHz internal sample rate
- Builds with PlatformIO

## Build

Requires [PlatformIO](https://platformio.org/).

```bash
pio run
pio run --target upload
```

## Sample data

The waveform is stored in `include/sample_data.h`. Generate it from a WAV file:

1. Run `wav_converter.py` on your WAV (e.g. `python wav_converter.py kick.wav`)
2. Copy or move the generated `sample_data.h` into `include/`

## Pinout

| Function   | Pin        |
|-----------|------------|
| Audio out | GPIO 6 (PWM) |
| Gate in   | GPIO 0     |
| Pitch pot | A0         |
| Decay pot | A1         |

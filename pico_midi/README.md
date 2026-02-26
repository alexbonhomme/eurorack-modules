# Pico MIDI

MIDI to CV converter for Eurorack. Converts MIDI note and velocity to 1V/oct CV and gate. Built for Raspberry Pi Pico or Pico 2. Uses MCP4822 dual 12-bit DACs (two chips for four channels).

## Features

- **4 CV/gate channels** — 1V/octave output (C0 = 0V, 10 octave range), gate per channel
- **Dual clock outputs** — 24 PPQN and 1/16th note (configurable in `lib/midi_cv_core/src/config.h`)
- **Channel 1 extras** — Accent (velocity above threshold) and slide (legato) outputs
- **CC on channel 1** — CC#70 mapped to a CV output for expression

## Build

Requires [PlatformIO](https://platformio.org/).

```bash
pio run
pio run --target upload
```

Serial monitor (after enabling debug): `pio device monitor`. Enable debug by uncommenting `#define DEBUG` in `lib/midi_cv_core/src/config.h`.

## Pin mapping

| Function   | GPIO |
|-----------|------|
| MIDI RX   | GP1 (Serial1 / UART0) |
| DAC1–4 CS| GP2, GP3, GP4, GP5   |
| Gate 1    | GP6  |
| Slide 1   | GP7  |
| Accent 1  | GP8  |
| Gate 3    | GP10 |
| Gate 4    | GP11 |
| Clock 1   | GP12 |
| Clock 2   | GP13 |
| MIDI LED  | GP14 |
| Clock LED | GP15 |
| Gate LED 1–2 | GP16, GP17 |
| Gate LED 3–4 | GP20, GP21 |
| Gate 2    | GP22 |

## Schematic

Full circuit diagram: [schematic_midi_cv_pico.pdf](schematic_midi_cv_pico.pdf).

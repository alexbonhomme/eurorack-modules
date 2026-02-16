# MIDI to CV Converter - Raspberry Pi Pico 2

MIDI to CV converter for Eurorack synthesizers. Converts MIDI notes to 1V/oct CV outputs with gate triggers and clock synchronization.

## Features

- 4 independent CV/gate channels (MCP4822 DAC)
- 1V/octave output (C0 = 0V, 10 octave range)
- Dual clock outputs (24 PPQN and 1/16th note)
- Channel 1 includes accent and slide functionality
- CC control on channel 1 (CC#70)

## Build

Requires [PlatformIO](https://platformio.org/).

```bash
# Build
pio run

# Upload
pio run --target upload

# Serial monitor (debug mode)
pio device monitor
```

Enable debug output by uncommenting `#define DEBUG` in `src/config.h`.

## Pin mapping

| Function     | GPIO |
|-------------|------|
| MIDI RX     | GP1 (Serial1 / UART0) |
| DAC1-4 CS   | GP2-5 |
| Gate 1      | GP6  |
| Slide 1     | GP7  |
| Accent 1    | GP8  |
| Gate 3      | GP10 |
| Gate 4      | GP11 |
| Clock 1     | GP12 |
| Clock 2     | GP13 |
| MIDI LED    | GP14 |
| Clock LED   | GP15 |
| Gate LED 1-2| GP16-17 |
| Gate LED 3-4| GP20-21 |
| Gate 2      | GP22 |

## Schematic

See [schematic_midi_cv_pico.pdf](schematic_midi_cv_pico.pdf) for the complete circuit diagram.

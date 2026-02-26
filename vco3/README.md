# VCO3

Three-oscillator VCO module using a Teensy and the Teensy Audio Library. Three waveforms are mixed (with individual level controls), passed through an envelope, and output via MQS (quad DAC) for Eurorack-level audio.

## Features

- **Three oscillators** — Shared waveform type (saw, square, triangle, sine), separate frequency and level pots
- **1V/oct CV** — Single CV input (A9) driving all oscillators
- **Envelope** — Gate input (pin 22) triggers envelope; attack/hold/decay/sustain/release
- **Waveform switch** — Button on pin 0 cycles waveform type
- **MQS output** — Audio out via Teensy MQS (no external DAC required for basic use)

## Build

Use the Arduino IDE with Teensy support (and Teensyduino). Open `vco3.ino`, select the correct Teensy board, then build and upload.

## Pinout (from sketch)

| Function   | Pin / ADC |
|-----------|-----------|
| Waveform button | 0  |
| Gate in   | 22 |
| Freq pots | A4, A3, A2 (osc 1–3) |
| Level pots| A1, A0 (osc 2–3 level) |
| 1V/oct CV | A9 |

Exact pot-to-oscillator mapping and envelope defaults are in `vco3.ino`.

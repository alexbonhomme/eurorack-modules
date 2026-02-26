# Pico Scope

Dual-channel oscilloscope for Eurorack. Raspberry Pi Pico drives an SH1106 128×64 OLED (SPI or I2C) via U8g2. Two ADC inputs with optional op-amp conditioning for Eurorack levels; time/div and volts/div are set by potentiometers.

## Features

- Two analog inputs (Channel 1, Channel 2)
- Adjustable time base and vertical scale (time/div, volts/div pots)
- Triggering (rising edge)
- Eurorack power: +5V for Pico (VSYS), ±12V for TL072 input conditioning

## Build

Requires [PlatformIO](https://platformio.org/).

```bash
pio run
pio run --target upload
```

## Pinout

| Function     | GPIO / Pin |
|-------------|------------|
| **SH1106 SPI** | |
| SCK         | GP18       |
| MOSI        | GP19       |
| CS          | GP17       |
| DC          | GP16       |
| RST         | GP15       |
| **SH1106 I2C** | |
| SDA         | GP4        |
| SCL         | GP5        |
| **Inputs**  | |
| Channel 1   | GP26 (A0)  |
| Channel 2   | GP27 (A1)  |
| Time/Div pot| GP28 (A2)  |
| Volts/Div pot | GP29 (A3) |
| **Power**   | |
| Eurorack +5V  | Pico VSYS |
| Eurorack ±12V | TL072 (input conditioning) |

Full pinout and conditioning notes are in `src/main.cpp`.

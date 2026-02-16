# 909 OH (PlatformIO)

909-style open-hat sample player for **RP2040 Zero**. No SD card — sample is baked in from a header.

- **Build:** `pio run`
- **Upload:** `pio run -t upload`
- **Sample data:** Generate `include/sample_data.h` with `../909_oh/wav_converter.py`, then copy into `include/`.

Pins: audio out 6, gate in 1, pitch pot A0, decay pot A1.

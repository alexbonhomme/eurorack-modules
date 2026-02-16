# Pico Drum

Drum sample player for **RP2040 Zero**. No SD card — sample is baked in from a header.

- **Build:** `pio run`
- **Upload:** `pio run -t upload`
- **Sample data:** Generate `include/sample_data.h` with `wav_converter.py`, then copy into `include/`.

Pins: audio out 6, gate in 1, pitch pot A0, decay pot A1.

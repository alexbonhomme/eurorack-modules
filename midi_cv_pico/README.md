# MIDI to CV Converter - Raspberry Pi Pico 2 W

MIDI to CV converter for Eurorack synthesizers. Converts MIDI notes to 1V/oct CV outputs with gate triggers and clock synchronization.

Features:

- 4 independent CV/gate channels
- 1V/octave output (C0 = 0V, 10 octave range)
- Dual clock outputs (24 PPQN and 1/16th note)
- Channel 1 includes accent and slide functionality
- SPI DAC output for precise voltage control

## Schematic

See [schematic_midi_cv_pico.pdf](schematic_midi_cv_pico.pdf) for the complete circuit diagram.

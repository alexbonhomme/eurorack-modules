#ifndef CONFIG_H
#define CONFIG_H

// --- LED Pins ---
#define PIN_MIDI_LED 14
#define PIN_CLOCK_LED 15
#define PIN_GATE_LED_1 16
#define PIN_GATE_LED_2 17
#define PIN_GATE_LED_3 20
#define PIN_GATE_LED_4 21

// --- DAC Chip Select Pins (MCP4822) ---
#define PIN_DAC1 2
#define PIN_DAC2 3
#define PIN_DAC3 4
#define PIN_DAC4 5

// --- Gate / Control Output Pins ---
#define PIN_GATE_1 6
#define PIN_SLIDE_1 7
#define PIN_ACCENT_1 8
#define PIN_GATE_2 22
#define PIN_GATE_3 10
#define PIN_GATE_4 11

// --- Clock Output Pins ---
#define PIN_CLOCK_1 12
#define PIN_CLOCK_2 13

// --- MIDI Channels ---
#define MIDI_CH1 1
#define MIDI_CH2 2
#define MIDI_CH3 3
#define MIDI_CH4 4

// --- MIDI CC Numbers ---
#define CC_1 70

// --- Clock Settings ---
#define PPQN_CLOCK 6      // 1/16th note resolution (standard eurorack clock)
#define PPQN_CLOCK_2 24   // 1/4 note resolution
#define PPQN_CLOCK_LED 24 // 1/4 note resolution for clock LED visual feedback
#define CLOCK_PULSE_WIDTH_US 10000 // 10ms pulse width

// --- Accent Velocity Threshold ---
#define ACCENT_VELOCITY_THRESHOLD 80

#endif // CONFIG_H

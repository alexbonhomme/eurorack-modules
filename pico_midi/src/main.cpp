#include <Arduino.h>
#include <MIDI.h>

#include "config.h"
#include "dac.h"
#include "midi_clock.h"

// Using Serial2 (UART1: GP26 TX / GP9 RX) for MIDI input.
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

static uint8_t note_count_ch1 = 0;
static uint8_t active_note_count = 0;

// ---------------------------------------------------------------------------
// Pin initialisation
// ---------------------------------------------------------------------------
static void initPins() {
    // LED outputs
    const uint8_t led_pins[] = {
        PIN_MIDI_LED, PIN_CLOCK_LED,
        PIN_GATE_LED_1, PIN_GATE_LED_2, PIN_GATE_LED_3, PIN_GATE_LED_4
    };
    for (auto pin : led_pins) {
        pinMode(pin, OUTPUT);
    }

    // DAC chip-select pins (active LOW, idle HIGH)
    const uint8_t dac_pins[] = {PIN_DAC1, PIN_DAC2, PIN_DAC3, PIN_DAC4};
    for (auto pin : dac_pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }

    // Clock outputs
    const uint8_t clock_pins[] = {PIN_CLOCK_1, PIN_CLOCK_2};
    for (auto pin : clock_pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    // Gate / control outputs
    const uint8_t gate_pins[] = {
        PIN_GATE_1, PIN_SLIDE_1, PIN_ACCENT_1,
        PIN_GATE_2, PIN_GATE_3, PIN_GATE_4
    };
    for (auto pin : gate_pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
}

static void startupAnimation() {
    const uint8_t leds[] = {
        PIN_CLOCK_LED, PIN_MIDI_LED,
        PIN_GATE_LED_1, PIN_GATE_LED_2, PIN_GATE_LED_3, PIN_GATE_LED_4
    };
    for (auto pin : leds) {
        digitalWrite(pin, HIGH);
        delay(300);
    }
    delay(200);
    for (auto pin : leds) {
        digitalWrite(pin, LOW);
    }
}

// ---------------------------------------------------------------------------
// MIDI Handlers
// ---------------------------------------------------------------------------
static void onNoteOn(byte channel, byte pitch, byte velocity) {
    if (active_note_count < 255) {
        active_note_count++;
    }
    digitalWrite(PIN_MIDI_LED, HIGH);

    if (channel == MIDI_CH1) {
        if (note_count_ch1 < 255) {
            note_count_ch1++;
        }
        commandNote(PIN_DAC1, pitch);
        digitalWrite(PIN_GATE_LED_1, HIGH);

        if (velocity > ACCENT_VELOCITY_THRESHOLD) {
            digitalWrite(PIN_ACCENT_1, HIGH);
        } else {
            digitalWrite(PIN_ACCENT_1, LOW);
        }

        if (note_count_ch1 > 1) {
            digitalWrite(PIN_SLIDE_1, HIGH);
        }
        digitalWrite(PIN_GATE_1, HIGH);
    }

    if (channel == MIDI_CH2) {
        commandNote(PIN_DAC2, pitch);
        commandCV(PIN_DAC2, velocity);
        digitalWrite(PIN_GATE_2, HIGH);
        digitalWrite(PIN_GATE_LED_2, HIGH);
    }

    if (channel == MIDI_CH3) {
        commandNote(PIN_DAC3, pitch);
        commandCV(PIN_DAC3, velocity);
        digitalWrite(PIN_GATE_3, HIGH);
        digitalWrite(PIN_GATE_LED_3, HIGH);
    }

    if (channel == MIDI_CH4) {
        commandNote(PIN_DAC4, pitch);
        commandCV(PIN_DAC4, velocity);
        digitalWrite(PIN_GATE_4, HIGH);
        digitalWrite(PIN_GATE_LED_4, HIGH);
    }
}

static void onNoteOff(byte channel, byte pitch, byte velocity) {
    if (active_note_count > 0) {
        active_note_count--;
    }
    if (active_note_count == 0) {
        digitalWrite(PIN_MIDI_LED, LOW);
    }

    if (channel == MIDI_CH1) {
        if (note_count_ch1 > 0) {
            note_count_ch1--;
        }

        // End slide when no longer in legato (back to single note or none)
        if (note_count_ch1 <= 1) {
            digitalWrite(PIN_SLIDE_1, LOW);
        }

        if (note_count_ch1 == 0) {
            digitalWrite(PIN_GATE_1, LOW);
            digitalWrite(PIN_ACCENT_1, LOW);
            digitalWrite(PIN_GATE_LED_1, LOW);
        }
    }

    if (channel == MIDI_CH2) {
        commandCV(PIN_DAC2, 0);
        digitalWrite(PIN_GATE_2, LOW);
        digitalWrite(PIN_GATE_LED_2, LOW);
    }

    if (channel == MIDI_CH3) {
        commandCV(PIN_DAC3, 0);
        digitalWrite(PIN_GATE_3, LOW);
        digitalWrite(PIN_GATE_LED_3, LOW);
    }

    if (channel == MIDI_CH4) {
        commandCV(PIN_DAC4, 0);
        digitalWrite(PIN_GATE_4, LOW);
        digitalWrite(PIN_GATE_LED_4, LOW);
    }
}

static void onControlChange(byte channel, byte number, byte value) {
    if (number == CC_1) {
        commandCV(PIN_DAC1, value);
    }
}

// ---------------------------------------------------------------------------
// Setup & Loop
// ---------------------------------------------------------------------------
void setup() {
#ifdef DEBUG
    Serial.begin(115200);
    delay(300);
    Serial.println("=== MIDI to CV Converter Starting ===");
#endif

    Serial2.setRX(9);  // MIDI RX pin on board (GP9)
    Serial2.setTX(26); // set to free pin to avoid conflicts

    initPins();
    startupAnimation();
    dac_init();

    MIDI.turnThruOff();
    MIDI.setHandleNoteOn(onNoteOn);
    MIDI.setHandleNoteOff(onNoteOff);
    MIDI.setHandleControlChange(onControlChange);
    MIDI.setHandleClock(handleClock);
    MIDI.setHandleStart(handleStartAndContinue);
    MIDI.setHandleContinue(handleStartAndContinue);
    MIDI.setHandleStop(handleStop);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    // clear MIDI buffer
    while (Serial2.available()) Serial2.read();

#ifdef DEBUG
    Serial.println("=== Ready - Waiting for MIDI ===");
#endif
}

void loop() {
#ifdef DEBUG
    if (MIDI.read()) {
        Serial.print("MIDI - Type:");
        Serial.print(MIDI.getType());
        Serial.print(" Ch:");
        Serial.print(MIDI.getChannel());
        Serial.print(" D1:");
        Serial.print(MIDI.getData1());
        Serial.print(" D2:");
        Serial.println(MIDI.getData2());
    }
#else
    MIDI.read();
#endif

    updateClock();
}

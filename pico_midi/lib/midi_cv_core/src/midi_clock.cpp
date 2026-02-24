#include "midi_clock.h"
#include "config.h"

struct ClockState {
    bool pulse         = false;
    uint8_t count      = 0;
    unsigned long pulse_start = 0;
};

static bool midi_playing = false;
static ClockState clocks[2];
static ClockState led_clock;

static void setClockPulse(uint8_t idx, bool active) {
    uint8_t pin = (idx == 0) ? PIN_CLOCK_1 : PIN_CLOCK_2;

    if (active) {
        digitalWrite(pin, HIGH);
        clocks[idx].pulse_start = micros();
        clocks[idx].pulse = true;
        clocks[idx].count = 0;
    } else {
        digitalWrite(pin, LOW);
        clocks[idx].pulse = false;
    }
}

static void setLedPulse(bool active) {
    if (active) {
        digitalWrite(PIN_CLOCK_LED, HIGH);
        led_clock.pulse_start = micros();
        led_clock.pulse = true;
        led_clock.count = 0;
    } else {
        digitalWrite(PIN_CLOCK_LED, LOW);
        led_clock.pulse = false;
    }
}

void handleClock() {
    if (!midi_playing) return;

    clocks[0].count++;
    if (clocks[0].count >= PPQN_CLOCK) {
        setClockPulse(0, true);
    }

    clocks[1].count++;
    if (clocks[1].count >= PPQN_CLOCK_2) {
        setClockPulse(1, true);
    }

    led_clock.count++;
    if (led_clock.count >= PPQN_CLOCK_LED) {
        setLedPulse(true);
    }
}

void handleStartAndContinue() {
    midi_playing = true;
    setClockPulse(0, true);
    setClockPulse(1, true);
    setLedPulse(true);
}

void handleStop() {
    midi_playing = false;
    setClockPulse(0, false);
    setClockPulse(1, false);
    setLedPulse(false);
}

void updateClock() {
    for (uint8_t i = 0; i < 2; i++) {
        if (clocks[i].pulse && (micros() - clocks[i].pulse_start > CLOCK_PULSE_WIDTH_US)) {
            setClockPulse(i, false);
        }
    }
    if (led_clock.pulse && (micros() - led_clock.pulse_start > CLOCK_PULSE_WIDTH_US)) {
        setLedPulse(false);
    }
}

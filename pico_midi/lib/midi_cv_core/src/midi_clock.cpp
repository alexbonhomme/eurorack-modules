#include "midi_clock.h"
#include "config.h"

struct ClockState {
    bool pulse         = false;
    uint16_t count     = 0;
    unsigned long pulse_start = 0;
    uint8_t divIdx     = 0; // index into kClockDivisionTicks[]
};

static bool midi_playing = false;
// Default divisions: Clock 1 = 1/16 (idx 0), Clock 2 = 1/4 (idx 2)
static ClockState clocks[2] = {
    {false, 0, 0, 0}, // Clock 1: divIdx 0 = 1/16
    {false, 0, 0, 2}, // Clock 2: divIdx 2 = 1/4
};
static ClockState led_clock;

void setClockDivisor(uint8_t idx, uint8_t divIdx) {
    if (idx >= 2) return;
    if (divIdx >= kClockDivisionCount) divIdx = kClockDivisionCount - 1;
    clocks[idx].divIdx = divIdx;
    clocks[idx].count  = 0; // reset counter to avoid stale state
}

uint8_t getClockDivisorIndex(uint8_t idx) {
    if (idx >= 2) return 0;
    return clocks[idx].divIdx;
}

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

    for (uint8_t i = 0; i < 2; i++) {
        clocks[i].count++;
        if (clocks[i].count >= kClockDivisionTicks[clocks[i].divIdx]) {
            setClockPulse(i, true);
        }
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

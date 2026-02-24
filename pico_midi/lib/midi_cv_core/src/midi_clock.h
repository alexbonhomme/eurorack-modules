#ifndef MIDI_CLOCK_H
#define MIDI_CLOCK_H

#include <Arduino.h>

/**
 * Clock division table index (0–7).
 *
 * Maps to musical divisions relative to a quarter note (24 MIDI PPQN):
 *   0 = 1/16  (6 ticks)
 *   1 = 1/8   (12 ticks)
 *   2 = 1/4   (24 ticks)
 *   3 = 1/2   (48 ticks)
 *   4 = 1     (96 ticks  — one bar)
 *   5 = 2     (192 ticks — two bars)
 *   6 = 4     (384 ticks — four bars)
 *   7 = 8     (768 ticks — eight bars)
 *
 * Default: Clock 1 = 0 (1/16), Clock 2 = 2 (1/4).
 */
static constexpr uint8_t kClockDivisionCount = 8;
static constexpr uint16_t kClockDivisionTicks[kClockDivisionCount] = {
    6, 12, 24, 48, 96, 192, 384, 768
};

/**
 * Set the division for one of the two clock outputs at runtime.
 * @param idx   Clock index: 0 = Clock 1, 1 = Clock 2.
 * @param divIdx Division table index (0–7, clamped if out of range).
 */
void setClockDivisor(uint8_t idx, uint8_t divIdx);

/** Return the current division table index for a clock output (0 or 1). */
uint8_t getClockDivisorIndex(uint8_t idx);

void handleClock();
void handleStartAndContinue();
void handleStop();
void updateClock();

#endif // MIDI_CLOCK_H

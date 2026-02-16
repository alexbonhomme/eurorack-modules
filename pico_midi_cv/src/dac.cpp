#include "dac.h"
#include <SPI.h>

// V/OCT DAC lookup table: 121 values for C0-C10 (0-4095 for MCP4822)
static const uint16_t CV_TABLE[121] = {
    0,    34,   68,   102,  137,  171,  205,  239,  273,  307,  341,
    375,  410,  444,  478,  512,  546,  580,  614,  648,  683,  717,
    751,  785,  819,  853,  887,  921,  956,  990,  1024, 1058, 1092,
    1126, 1160, 1194, 1229, 1263, 1297, 1331, 1365, 1399, 1433, 1467,
    1502, 1536, 1570, 1604, 1638, 1672, 1706, 1740, 1775, 1809, 1843,
    1877, 1911, 1945, 1979, 2013, 2048, 2082, 2116, 2150, 2184, 2218,
    2252, 2286, 2321, 2355, 2389, 2423, 2457, 2491, 2525, 2559, 2594,
    2628, 2662, 2696, 2730, 2764, 2798, 2832, 2867, 2901, 2935, 2969,
    3003, 3037, 3071, 3105, 3140, 3174, 3208, 3242, 3276, 3310, 3344,
    3378, 3413, 3447, 3481, 3515, 3549, 3583, 3617, 3651, 3686, 3720,
    3754, 3788, 3822, 3856, 3890, 3924, 3959, 3993, 4027, 4061, 4095};

// Clamp MIDI note to valid CV range (0-120)
static uint8_t processNote(uint8_t note) {
  uint8_t val = note - 12; // MIDI note 12 = C0 = 0V

  if (val < 0) {
    return 0;
  }

  if (val > 120) {
    return 120;
  }

  return val;
}

// Write 12-bit value to MCP4822 DAC via SPI
// channel: 0=A (note), 1=B (velocity/CV) | gain: 2x | value: 0-4095
static void setVoltage(uint8_t dac_pin, bool channel, uint16_t mV) {
  uint16_t command = channel ? 0x9000 : 0x1000;
  command |= 0x2000; // Gain 2x
  command |= (mV & 0x0FFF);

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(dac_pin, LOW);
  SPI.transfer(command >> 8);
  SPI.transfer(command & 0xFF);
  digitalWrite(dac_pin, HIGH);
  SPI.endTransaction();
}

void dac_init() {
  SPI.setRX(255); // don't claim any pin for MISO (not needed)
  SPI.begin();
}

void commandNote(uint8_t dac_pin, uint8_t pitch) {
  uint8_t note = processNote(pitch);
  setVoltage(dac_pin, 0, CV_TABLE[note]);
}

void commandCV(uint8_t dac_pin, uint8_t value) {
  uint16_t mV = map(value, 0, 127, 0, 4095);
  setVoltage(dac_pin, 1, mV);
}

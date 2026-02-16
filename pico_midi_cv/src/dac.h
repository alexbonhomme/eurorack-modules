#ifndef DAC_H
#define DAC_H

#include <Arduino.h>

void dac_init();
void commandNote(uint8_t dac_pin, uint8_t pitch);
void commandCV(uint8_t dac_pin, uint8_t value);

#endif // DAC_H

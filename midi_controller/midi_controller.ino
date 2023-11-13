#include <Control_Surface.h>

// TODO : USB + Serial midi with Teensy
//USBMIDI_Interface midi;

// Instantiate a hardware MIDI interface with Serial as its UART
HardwareSerialMIDI_Interface midi {Serial};
 
// If the jog wheels or other encoders are too slow in your software, increase
// this value.
// (It will be multiplied with the actual speed of the encoder, as the name
// implies.) Default is 1.
const int speedMultiplier = 5;
 
CCPotentiometer potars[] {
  {A0, MIDI_CC::General_Purpose_Controller_1},
  {A1, MIDI_CC::General_Purpose_Controller_2},
  {A2, MIDI_CC::General_Purpose_Controller_3},
  {A3, MIDI_CC::General_Purpose_Controller_4},
  
  // fader
  {A5, { MIDI_CC::Channel_Volume, Channel_1 }},
};

CCRotaryEncoder enc {
  {4, 3},                                // pins
  MIDI_CC::General_Purpose_Controller_5, // address
  speedMultiplier,                       // multiplier
  4,                                     // pulses per click
};

// Instantiate a CCButton object
CCButton button {
  // Push button on pin 2:
  2,
  // General Purpose Controller #6:
  MIDI_CC::General_Purpose_Controller_6,
};

void setup() {
  Control_Surface.begin();
}
 
void loop() { // Refresh all inputs
  Control_Surface.loop();
}

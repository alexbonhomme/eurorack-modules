/*

 */

#include <MIDI.h>
#include <Adafruit_MCP4728.h>
#include <Wire.h>

#define GATE 8
#define SLIDE 9
#define ACCENT 10
// #define GATE_2 11
#define CLOCK 12

#define MIDI_CH1 1
#define MIDI_CH2 2

#define CC_1 70
#define CC_2 71

#define PPQN_CLOCK 24 // Clock resolution

MIDI_CREATE_DEFAULT_INSTANCE();

Adafruit_MCP4728 mcp;

// V/OCT LSB for DAC
const PROGMEM long cv[121] = {
    0, 34, 68, 102, 137, 171, 205, 239, 273, 307, 341, 375, 410,
    444, 478, 512, 546, 580, 614, 648, 683, 717, 751, 785, 819,
    853, 887, 921, 956, 990, 1024, 1058, 1092, 1126, 1160, 1194, 1229,
    1263, 1297, 1331, 1365, 1399, 1433, 1467, 1502, 1536, 1570, 1604, 1638,
    1672, 1706, 1740, 1775, 1809, 1843, 1877, 1911, 1945, 1979, 2013, 2048,
    2082, 2116, 2150, 2184, 2218, 2252, 2286, 2321, 2355, 2389, 2423, 2457,
    2491, 2525, 2559, 2594, 2628, 2662, 2696, 2730, 2764, 2798, 2832, 2867,
    2901, 2935, 2969, 3003, 3037, 3071, 3105, 3140, 3174, 3208, 3242, 3276,
    3310, 3344, 3378, 3413, 3447, 3481, 3515, 3549, 3583, 3617, 3651, 3686,
    3720, 3754, 3788, 3822, 3856, 3890, 3924, 3959, 3993, 4027, 4061, 4095};

void setup()
{
  pinMode(CLOCK, OUTPUT);
  pinMode(GATE, OUTPUT);
  pinMode(SLIDE, OUTPUT);
  pinMode(ACCENT, OUTPUT);

  digitalWrite(CLOCK, LOW);
  digitalWrite(GATE, LOW);
  digitalWrite(SLIDE, LOW);
  digitalWrite(ACCENT, LOW);

  mcp.begin();

  MIDI.turnThruOff();
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleClock(handleClock);
  MIDI.setHandleControlChange(handleControlChange);

  MIDI.begin(MIDI_CHANNEL_OMNI);
}

static int clock_count = 1;
int dac_channel, note_count = 0;

void loop()
{
  MIDI.read();
}

int isActiveChannel(byte channel)
{
  // Only 2 channels support
  return (MIDI_CH1 == channel || MIDI_CH2 == channel);
}

byte processNote(byte note)
{
  int val = note - 12; // C0 = 12

  if (val < 0)
  {
    return (0);
  }

  if (val > 120)
  {
    return (120); // max 10 oct. = 120 notes
  }

  return val;
}

// @todo handle channel 2 (gate, etc.)
void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  if (!isActiveChannel(channel))
  {
    return;
  }

  if (velocity == 0)
  {
    // This acts like a NoteOff.
    handleNoteOff(channel, pitch, velocity);
    return;
  }

  byte note = processNote(pitch);

  note_count++;

  // CV
  commandNote(channel, note);

  // Gate
  digitalWrite(GATE, HIGH);

  // Accent
  if (velocity > 80)
  {
    digitalWrite(ACCENT, HIGH);
  }
  else
  {
    digitalWrite(ACCENT, LOW);
  }

  // Slide
  if (note_count > 1)
  {
    digitalWrite(SLIDE, HIGH);
  }
}

// @todo handle channel 2 (gate, etc.)
void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  if (!isActiveChannel(channel))
  {
    return;
  }

  byte note = processNote(pitch);

  note_count--;

  if (note_count < 0)
  {
    note_count = 0;
  }

  if (note_count == 0)
  {
    // Gate
    digitalWrite(GATE, LOW);

    // Accent
    digitalWrite(ACCENT, LOW);

    // Slide
    digitalWrite(SLIDE, LOW);
  }
}

void handleClock(void)
{
  // Clock out everytime receives clock in normal mode
  if (clock_count < PPQN_CLOCK)
  {
    clock_count++;

    digitalWrite(CLOCK, LOW);
  }
  else
  {
    clock_count = 1;

    // Send trigger to CLOCK port
    digitalWrite(CLOCK, HIGH);
  }
}

void handleControlChange(byte channel, byte number, byte value)
{
  if (!isActiveChannel(channel))
  {
    return;
  }

  if (number == CC_1 || number == CC_2)
  {
    commandCC(number, value);
  }
}

/**
 * @brief Send CV to DAC
 */
void commandNote(byte channel, byte note)
{
  uint16_t mV = pgm_read_dword_near(cv + note);

  mcp.setChannelValue(channel == MIDI_CH1 ? MCP4728_CHANNEL_C : MCP4728_CHANNEL_D, mV, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X);
}

/**
 * @brief Send CC to DAC
 */
void commandCC(byte number, byte value)
{
  uint16_t mV = map(value, 0, 127, 0, 4095);

  mcp.setChannelValue(number == CC_1 ? MCP4728_CHANNEL_A : MCP4728_CHANNEL_B, mV, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X);
}
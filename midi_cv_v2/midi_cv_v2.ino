/*

 */

#include <MIDI.h>
#include <Adafruit_MCP4728.h>
#include <Wire.h>

#define GATE 2
#define SLIDE 3
#define ACCENT 4
#define CLOCK 5

#define MIDI_CH1 1
#define MIDI_CH2 2

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

  MIDI.turnThruOff();                   // Thru off disconnected
  MIDI.setHandleNoteOn(handleNoteOn);   // Handle for NoteOn
  MIDI.setHandleNoteOff(handleNoteOff); // Handle for NoteOff
  MIDI.setHandleClock(handleClock);

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

  // @todo
  // dac_channel = MIDI_CH1 == channel ? 0 : 1;

  byte note = processNote(pitch);

  note_count++;

  // CV
  commandNote(note);

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
  }
  else
  {
    clock_count = 1;

    // Send trigger to CLOCK port
    digitalWrite(CLOCK, HIGH);
  }
}

// @todo multi channel
void commandNote(int note)
{
  long mV = pgm_read_dword_near(cv + note);

  mcp.setChannelValue(MCP4728_CHANNEL_A, mV, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X);
}

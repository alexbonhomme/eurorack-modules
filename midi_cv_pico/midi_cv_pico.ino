#include <MIDI.h>
#include <SPI.h>

// #define DEBUG

#define MIDI_LED 14
#define CLOCK_LED 15
#define GATE_LED_1 16
#define GATE_LED_2 17
#define GATE_LED_3 20
#define GATE_LED_4 21

#define DAC1 2
#define DAC2 3
#define DAC3 4
#define DAC4 5

#define GATE_1 6
#define SLIDE_1 7
#define ACCENT_1 8
#define GATE_2 22
#define GATE_3 10
#define GATE_4 11

#define CLOCK 12
#define CLOCK_2 13

#define MIDI_CH1 1
#define MIDI_CH2 2
#define MIDI_CH3 3
#define MIDI_CH4 4

#define CC_1 70

#define PPQN_CLOCK 24              // Clock resolution
#define PPQN_CLOCK_2 6             // 1/16th Clock resolution
#define CLOCK_PULSE_WIDTH_US 10000 // 10ms pulse width

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

// V/OCT LSB for DAC - 1V/octave conversion table
const long cv[121] = {
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

bool midi_playing = false;
unsigned char note_count_ch1 = 0;

struct clock
{
  bool pulse = false;
  unsigned char count = 0; // Start at 0, not 1
  unsigned long pulse_start = 0;
};

struct clock clocks[2];

/**
 * Setup function for Raspberry Pi Pico 2
 */
void setup()
{
#ifdef DEBUG
  // Initialize serial communication for debugging
  Serial.begin(115200);
  delay(100);

  Serial.println("=== MIDI to CV Converter Starting ===");
  Serial.println("Pico 2 Initialization...");
#endif

  // Configure LED output pins
  pinMode(MIDI_LED, OUTPUT);
  pinMode(CLOCK_LED, OUTPUT);
  pinMode(GATE_LED_1, OUTPUT);
  pinMode(GATE_LED_2, OUTPUT);
  pinMode(GATE_LED_3, OUTPUT);
  pinMode(GATE_LED_4, OUTPUT);

  digitalWrite(MIDI_LED, HIGH); // Turn on LED to show code is running
  digitalWrite(CLOCK_LED, LOW);
  digitalWrite(GATE_LED_1, LOW);
  digitalWrite(GATE_LED_2, LOW);
  digitalWrite(GATE_LED_3, LOW);
  digitalWrite(GATE_LED_4, LOW);

  // Configure gate output pin
  pinMode(DAC1, OUTPUT);
  pinMode(DAC2, OUTPUT);
  pinMode(DAC3, OUTPUT);
  pinMode(DAC4, OUTPUT);

  pinMode(CLOCK, OUTPUT);
  pinMode(CLOCK_2, OUTPUT);

  pinMode(GATE_1, OUTPUT);
  pinMode(SLIDE_1, OUTPUT);
  pinMode(ACCENT_1, OUTPUT);
  pinMode(GATE_2, OUTPUT);
  pinMode(GATE_3, OUTPUT);
  pinMode(GATE_4, OUTPUT);

  digitalWrite(CLOCK, LOW);
  digitalWrite(CLOCK_2, LOW);

  digitalWrite(GATE_1, LOW);
  digitalWrite(SLIDE_1, LOW);
  digitalWrite(ACCENT_1, LOW);
  digitalWrite(GATE_2, LOW);
  digitalWrite(GATE_3, LOW);
  digitalWrite(GATE_4, LOW);

  digitalWrite(DAC1, HIGH);
  digitalWrite(DAC2, HIGH);
  digitalWrite(DAC3, HIGH);
  digitalWrite(DAC4, HIGH);

  // Initialize SPI
  SPI.begin();

  // Configure MIDI
  MIDI.turnThruOff();
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleClock(handleClock);
  MIDI.setHandleStart(handleStartAndContinue);
  MIDI.setHandleContinue(handleStartAndContinue);
  MIDI.setHandleStop(handleStop);

  MIDI.begin(MIDI_CHANNEL_OMNI);

#ifdef DEBUG
  Serial.println("=== MIDI to CV Converter - Pico 2 W Ready ===");
  Serial.println("Waiting for MIDI input...");
#endif

  delay(300);
  digitalWrite(MIDI_LED, LOW);
}

/**
 * Main loop for MIDI processing
 */
void loop()
{
#ifdef DEBUG
  if (MIDI.read())
  {
    Serial.println("Midi received");
    Serial.print("Type: ");
    Serial.println(MIDI.getType());
    Serial.print("Channel: ");
    Serial.println(MIDI.getChannel());
    Serial.print("Data1: ");
    Serial.println(MIDI.getData1());
    Serial.print("Data2: ");
    Serial.println(MIDI.getData2());
  }
#else
  MIDI.read();
#endif

  updateClock();
}

/**
 * Handle MIDI Note On messages
 */
void handleNoteOn(byte channel, byte pitch, byte velocity)
{
#ifdef DEBUG
  Serial.print("Note On - Channel: ");
  Serial.print(channel);
  Serial.print(", Pitch: ");
  Serial.print(pitch);
  Serial.print(", Velocity: ");
  Serial.println(velocity);
#endif

  digitalWrite(MIDI_LED, HIGH);

  if (channel == MIDI_CH1)
  {
    note_count_ch1++;

    // Convert MIDI note to CV voltage
    commandNote(DAC1, pitch);

    // Gate LED HIGH
    digitalWrite(GATE_LED_1, HIGH);

    // Accent
    if (velocity > 80)
    {
      digitalWrite(ACCENT_1, HIGH);
    }
    else
    {
      digitalWrite(ACCENT_1, LOW);
    }

    // Slide
    if (note_count_ch1 > 1)
    {
      digitalWrite(SLIDE_1, HIGH);
    }

    // Gate HIGH
    digitalWrite(GATE_1, HIGH);
  }

  if (channel == MIDI_CH2)
  {
    // Convert MIDI note to CV voltage
    commandNote(DAC2, pitch);
    commandCV(DAC2, velocity);

    // Gate HIGH
    digitalWrite(GATE_2, HIGH);

    // Gate LED HIGH
    digitalWrite(GATE_LED_2, HIGH);
  }

  if (channel == MIDI_CH3)
  {
    // Convert MIDI note to CV voltage
    commandNote(DAC3, pitch);
    commandCV(DAC3, velocity);

    // Gate HIGH
    digitalWrite(GATE_3, HIGH);

    // Gate LED HIGH
    digitalWrite(GATE_LED_3, HIGH);
  }

  if (channel == MIDI_CH4)
  {
    // Convert MIDI note to CV voltage
    commandNote(DAC4, pitch);
    commandCV(DAC4, velocity);

    // Gate HIGH
    digitalWrite(GATE_4, HIGH);

    // Gate LED HIGH
    digitalWrite(GATE_LED_4, HIGH);
  }
}

/**
 * Handle MIDI Note Off messages
 */
void handleNoteOff(byte channel, byte pitch, byte velocity)
{
#ifdef DEBUG
  Serial.print("Note Off - Channel: ");
  Serial.print(channel);
  Serial.print(", Pitch: ");
  Serial.println(pitch);
#endif

  digitalWrite(MIDI_LED, LOW);

  if (channel == MIDI_CH1)
  {
    note_count_ch1--;

    if (note_count_ch1 < 0)
    {
      note_count_ch1 = 0;
    }

    if (note_count_ch1 == 0)
    {
      // Gate LOW
      digitalWrite(GATE_1, LOW);
      digitalWrite(ACCENT_1, LOW);
      digitalWrite(SLIDE_1, LOW);

      // Gate LED LOW
      digitalWrite(GATE_LED_1, LOW);
    }
  }

  if (channel == MIDI_CH2)
  {
    // Velocity LOW
    commandCV(DAC2, 0);

    // Gate LOW
    digitalWrite(GATE_2, LOW);

    // Gate LED LOW
    digitalWrite(GATE_LED_2, LOW);
  }

  if (channel == MIDI_CH3)
  {
    // Velocity LOW
    commandCV(DAC3, 0);

    // Gate LOW
    digitalWrite(GATE_3, LOW);

    // Gate LED LOW
    digitalWrite(GATE_LED_3, LOW);
  }

  if (channel == MIDI_CH4)
  {
    // Velocity LOW
    commandCV(DAC4, 0);

    // Gate LOW
    digitalWrite(GATE_4, LOW);

    // Gate LED LOW
    digitalWrite(GATE_LED_4, LOW);
  }
}

/**
 * Handle MIDI Control Change messages
 */
void handleControlChange(byte channel, byte number, byte value)
{
#ifdef DEBUG
  Serial.print("Control Change - Channel: ");
  Serial.print(channel);
  Serial.print(", Number: ");
  Serial.println(number);
  Serial.print("Value: ");
  Serial.println(value);
#endif

  if (number == CC_1)
  {
    commandCV(DAC1, value);
  }
}

/**
 *
 */
void handleClock(void)
{
  // Only process clock if MIDI is playing
  if (!midi_playing)
  {
    return;
  }

  clocks[0].count++;
  if (clocks[0].count >= PPQN_CLOCK)
  {
    setClockPulse(0, true);
  }

  clocks[1].count++;
  if (clocks[1].count >= PPQN_CLOCK_2)
  {
    setClockPulse(1, true);
  }
}

void handleStartAndContinue()
{
  midi_playing = true;
  setClockPulse(0, true);
  setClockPulse(1, true);
}

void handleStop()
{
  midi_playing = false;
  setClockPulse(0, false);
  setClockPulse(1, false);
}

void setClockPulse(unsigned char clock, bool active)
{
  if (active)
  {
    if (clock == 0)
    {
      digitalWrite(CLOCK, HIGH); // CLOCK HIGH
      digitalWrite(CLOCK_LED, HIGH);
    }
    else
    {
      digitalWrite(CLOCK_2, HIGH); // CLOCK_2 HIGH
    }

    clocks[clock].pulse_start = micros();
    clocks[clock].pulse = true;
    clocks[clock].count = 0;
  }
  else
  {
    if (clock == 0)
    {
      digitalWrite(CLOCK, LOW); // CLOCK LOW
      digitalWrite(CLOCK_LED, LOW);
    }
    else
    {
      digitalWrite(CLOCK_2, LOW); // CLOCK_2 LOW
    }

    clocks[clock].pulse = false;
  }
}

void updateClock()
{
  // Handle clock pulse width timing
  if (clocks[0].pulse && (micros() - clocks[0].pulse_start > CLOCK_PULSE_WIDTH_US))
  {
    setClockPulse(0, false);
  }

  if (clocks[1].pulse && (micros() - clocks[1].pulse_start > CLOCK_PULSE_WIDTH_US))
  {
    setClockPulse(1, false);
  }
}

/**
 *
 */
void commandNote(byte dac, byte pitch)
{
  byte note = processNote(pitch);
  uint16_t mV = cv[note];

#ifdef DEBUG
  Serial.print("Setting Note to DAC ");
  Serial.print(dac);
  Serial.print("Setting Note CV: ");
  Serial.print(mV);
  Serial.print(" mV for note ");
  Serial.println(note);
#endif

  setVoltage(dac, 0, mV);
}

/**
 *
 */
void commandCV(int dac, byte value)
{
  // will generate 0 to 10v
  uint16_t mV = map(value, 0, 127, 0, 4095);

#ifdef DEBUG
  Serial.print("Setting CV to DAC ");
  Serial.print(dac);
  Serial.print("Setting CV: ");
  Serial.print(mV);
  Serial.print(" mV for CV ");
  Serial.println(value);
#endif

  setVoltage(dac, 1, mV);
}

/**
 * Process MIDI note and convert to CV voltage
 */
byte processNote(byte note)
{
  int val = note - 12; // C0 = 12

  if (val < 0)
  {
    return 0;
  }

  if (val > 120)
  {
    return 120; // max 10 oct. = 120 notes
  }

  return val;
}

// setVoltage -- Set DAC voltage output
// dacpin: chip select pin for DAC.  Note and velocity on DAC1, pitch bend and CC on DAC2
// channel: 0 (A) or 1 (B).  Note and pitch bend on 0, velocity and CC on 2.
// gain: 0 = 1X, 1 = 2X.
// mV: integer 0 to 4095.  If gain is 1X, mV is in units of half mV (i.e., 0 to 2048 mV).
// If gain is 2X, mV is in units of mV
void setVoltage(int dacpin, bool channel, unsigned int mV)
{
  unsigned int command = channel ? 0x9000 : 0x1000;

  command |= 0x2000;
  command |= (mV & 0x0FFF);

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(dacpin, LOW);
  SPI.transfer(command >> 8);
  SPI.transfer(command & 0xFF);
  digitalWrite(dacpin, HIGH);
  SPI.endTransaction();
}
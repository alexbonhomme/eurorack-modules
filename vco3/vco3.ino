#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>

// GUItool: begin automatically generated code
AudioSynthWaveform waveform2;  // xy=450,435
AudioSynthWaveform waveform3;  // xy=450,484
AudioSynthWaveform waveform1;  // xy=451,387
AudioMixer4 mixer1;            // xy=700,421
AudioEffectEnvelope envelope1; // xy=882,417
AudioOutputMQS mqs1;           // xy=1078,416
AudioConnection patchCord1(waveform2, 0, mixer1, 1);
AudioConnection patchCord2(waveform3, 0, mixer1, 2);
AudioConnection patchCord3(waveform1, 0, mixer1, 0);
AudioConnection patchCord4(mixer1, envelope1);
AudioConnection patchCord5(envelope1, 0, mqs1, 0);
// GUItool: end automatically generated code

const int BT_1 = 0;
const int GATE_IN = 22;

Bounce button_1 = Bounce(BT_1, 15);
Bounce gate_in = Bounce(GATE_IN, 1);

int current_waveform = WAVEFORM_SAWTOOTH_REVERSE;

void setup()
{
  // Serial.begin(9600);

  pinMode(BT_1, INPUT_PULLUP);
  pinMode(GATE_IN, INPUT);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(10);

  waveform1.begin(current_waveform);
  waveform2.begin(current_waveform);
  waveform3.begin(current_waveform);

  waveform1.frequency(440);
  waveform2.frequency(440);
  waveform3.frequency(440);

  waveform1.amplitude(0);
  waveform2.amplitude(0);
  waveform3.amplitude(0);

  envelope1.attack(5);
  envelope1.hold(0);
  envelope1.decay(0);
  envelope1.sustain(1);
  envelope1.release(30);

  mixer1.gain(0, 0.33);
  mixer1.gain(1, 0.33);
  mixer1.gain(2, 0.33);
}

void loop()
{
  // Read the buttons and knobs, scale knobs to 0-1.0
  button_1.update();
  gate_in.update();

  // Frequency
  float pot_1 = (float)analogRead(A4) / 1023.0;
  float pot_2 = (float)analogRead(A3) / 1023.0;
  float pot_3 = (float)analogRead(A2) / 1023.0;

  // Gain
  float pot_4 = (float)analogRead(A1) / 1023.0;
  float pot_5 = (float)analogRead(A0) / 1023.0;

  // 1v/Oct (0 - 10v)
  float cv = (float)analogRead(A9) / 102.3;

  float freq_1 = computeFrequency(pot_1, cv);
  float freq_2 = computeFrequency(pot_2, cv);
  float freq_3 = computeFrequency(pot_3, cv);

  AudioNoInterrupts();

  waveform1.frequency(freq_1);
  waveform2.frequency(freq_2);
  waveform3.frequency(freq_3);

  waveform1.amplitude(1);
  waveform2.amplitude(pot_4);
  waveform3.amplitude(pot_5);

  AudioInterrupts();

  // Gate (inverted gate signal)
  if (gate_in.risingEdge())
  {
    envelope1.noteOff();
  }
  else if (gate_in.fallingEdge())
  {
    envelope1.noteOn();
  }

  // Button 1 changes the waveform type
  if (button_1.fallingEdge())
  {
    switch (current_waveform)
    {
    case WAVEFORM_SINE:
      current_waveform = WAVEFORM_SAWTOOTH_REVERSE;
      // Serial.println("Sawtooth Reverse");
      break;
    case WAVEFORM_SAWTOOTH_REVERSE:
      current_waveform = WAVEFORM_SQUARE;
      // Serial.println("Square");
      break;
    case WAVEFORM_SQUARE:
      current_waveform = WAVEFORM_TRIANGLE;
      // Serial.println("Triangle");
      break;
    case WAVEFORM_TRIANGLE:
      current_waveform = WAVEFORM_SINE;
      // Serial.println("Sine");
      break;
    }

    AudioNoInterrupts();

    waveform1.begin(current_waveform);
    waveform2.begin(current_waveform);

    AudioInterrupts();
  }
}

/**
 * Compute the frequency of the waveform based on the pot and cv input.
 *
 * The pot input is should be from 0 to 1, and the cv input from 0 to 10.
 *
 * @see https://vcvrack.com/manual/VoltageStandards#Pitch-and-Frequencies
 */
float computeFrequency(float pot, float cv)
{
  float offset = pot * 2.0 - 1.0; // -1 to 1
  float cv_offset = cv + offset;

  if (cv_offset < 0)
  {
    cv_offset = 0.0;
  }
  else if (cv_offset > 10)
  {
    cv_offset = 10.0;
  }

  return 32.7032 * pow(2, cv_offset); // f0 = C0 = 32.7032
}

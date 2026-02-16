#include <Arduino.h>
#include "hardware/timer.h"
#include "sample_data.h"

const int AUDIO_PIN = 6;
const int GATE_PIN = 1;
const int PITCH_PIN = A0;
const int DECAY_PIN = A1;
const uint32_t SAMPLE_RATE_HZ = 44100;

volatile bool playing = false;
volatile float volume = 0.0f;
volatile float decay = 0.99f;
volatile float pos = 0.0f;
volatile float pitch = 1.0f;

struct repeating_timer sampleTimer;

static int16_t u8_to_s16(uint8_t u)
{
  return (int16_t)((u - 128) * 256);
}

bool onTimer(struct repeating_timer*)
{
  if (!playing)
  {
    analogWrite(AUDIO_PIN, 32767);
    return true;
  }

  int idx = (int)pos;
  if (idx >= SAMPLE_LEN)
  {
    playing = false;
    analogWrite(AUDIO_PIN, 32767);
    return true;
  }

  float frac = pos - idx;
  int16_t s0 = u8_to_s16(sampleData[idx]);
  int16_t s1 = (idx + 1 < SAMPLE_LEN) ? u8_to_s16(sampleData[idx + 1]) : 0;
  float sample = s0 + frac * (s1 - s0);
  sample *= volume;

  if (sample > 32767.0f) sample = 32767.0f;
  if (sample < -32768.0f) sample = -32768.0f;

  uint16_t pwmValue = (uint16_t)((int32_t)sample + 32768);
  analogWrite(AUDIO_PIN, pwmValue);

  volume *= decay;
  pos += pitch;
  return true;
}

void setup()
{
  pinMode(GATE_PIN, INPUT_PULLDOWN);
  analogReadResolution(12);

  analogWriteFreq(200000);
  analogWriteRange(65535);
  analogWrite(AUDIO_PIN, 32767);

  int intervalUs = 1000000 / SAMPLE_RATE_HZ;
  add_repeating_timer_us(-intervalUs, onTimer, NULL, &sampleTimer);
}

void loop()
{
  int rawPitch = analogRead(PITCH_PIN);
  int rawDecay = analogRead(DECAY_PIN);

  noInterrupts();
  pitch = 0.5f + (rawPitch / 4095.0f) * 1.5f;
  decay = 0.998f + (rawDecay / 4095.0f) * 0.002f;
  interrupts();

  static bool lastGate = false;
  bool gate = digitalRead(GATE_PIN);
  if (gate && !lastGate)
  {
    noInterrupts();
    playing = true;
    pos = 0.0f;
    volume = 1.0f;
    interrupts();
  }
  lastGate = gate;
}

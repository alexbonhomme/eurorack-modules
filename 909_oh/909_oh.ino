#include <Arduino.h>
#include "sample_data.h" // Include your converted sample

// === Pin configuration ===
const int AUDIO_PIN = 6;  // PWM audio output
const int GATE_PIN = 2;   // External gate input
const int PITCH_PIN = A0; // Pitch potentiometer
const int DECAY_PIN = A1; // Decay potentiometer

// === Playback state ===
bool playing = false;
float volume = 0.0f;
float decay = 0.99f;
float pos = 0.0f;
float pitch = 1.0f;

void setup()
{
  pinMode(GATE_PIN, INPUT_PULLDOWN);
  analogReadResolution(12);

  // Configure PWM for audio output
  analogWriteFreq(200000);       // ~200 kHz sample rate
  analogWriteRange(65535);       // 16-bit resolution
  analogWrite(AUDIO_PIN, 32767); // mid-level idle voltage
}

void loop()
{
  // === Read potentiometers ===
  int rawPitch = analogRead(PITCH_PIN);
  int rawDecay = analogRead(DECAY_PIN);

  // Map pot values to useful ranges
  pitch = 0.5f + (rawPitch / 4095.0f) * 1.5f;   // 0.25× to 2× playback rate
  decay = 0.999f + (rawDecay / 4095.0f) * 0.001f; // 0.999–1.00 (fast to slow decay)

  // === Detect trigger edge ===
  static bool lastGate = false;
  bool gate = digitalRead(GATE_PIN);
  if (gate && !lastGate)
  {
    playing = true;
    pos = 0.0f;
    volume = 1.0f;
  }
  lastGate = gate;

  // === Playback ===
  if (playing)
  {
    int idx = (int)pos;
    if (idx >= SAMPLE_LEN)
    {
      playing = false;
    }
    else
    {
      // Linear interpolation for smoother pitch
      float frac = pos - idx;
      float s0 = sampleData[idx];
      float s1 = (idx + 1 < SAMPLE_LEN) ? sampleData[idx + 1] : 128;
      float sample = s0 + frac * (s1 - s0);

      // Apply volume envelope
      sample = (sample - 128.0f) * volume + 128.0f;

      // Output to PWM
      analogWrite(AUDIO_PIN, (int)(sample / 255.0f * 65535.0f));

      // Update envelope and position
      volume *= decay;
      pos += pitch;
    }
  }
  else
  {
    analogWrite(AUDIO_PIN, 32767); // keep at mid-level when idle
  }

  // Simple timing for ~22 kHz playback
  delayMicroseconds(45);
}
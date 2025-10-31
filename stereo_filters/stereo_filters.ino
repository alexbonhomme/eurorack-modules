
#include "DaisyDuino.h"
#include "ladder.h"

#define POT_1 A0
#define POT_2 A1
#define POT_3 A2
#define POT_4 A3

DaisyHardware hw;

SmoothRandomGenerator smooth[3];
LadderFilter filters[4];
DelayLine<float, 24000> del_left, del_right;

float GetCtrl(uint8_t pin)
{
  return analogRead(pin) / 1023.f;
}

void AudioCallback(float **in, float **out, size_t size)
{
  for (size_t i = 0; i < size; i++)
  {
    float dry_left, dry_right, wet_left, wet_right;

    // Process Filters
    dry_left = filters[0].Process(IN_L[i]) + filters[2].Process(IN_L[i]) * 0.5;
    dry_right = filters[1].Process(IN_R[i]) + filters[3].Process(IN_R[i]) * 0.5;

    OUT_L[i] = dry_left;
    OUT_R[i] = dry_right;

    // // Read Wet from Delay Lines
    // wet_left = del_left.Read();
    // wet_right = del_right.Read();

    // // Write to Delay with some feedback
    // del_left.Write((wet_left * 0.5) + dry_left);
    // del_right.Write((wet_right * 0.5) + dry_right);

    // // Mix Dry and Wet and send to I/O
    // OUT_L[i] = wet_left * 0.707 + dry_left * 0.707;
    // OUT_R[i] = wet_right * 0.707 + dry_right * 0.707;
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(POT_1, INPUT);
  pinMode(POT_2, INPUT);
  pinMode(POT_3, INPUT);
  pinMode(POT_4, INPUT);

  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);

  float sample_rate = DAISY.get_samplerate();

  filters[0].Init(sample_rate);
  filters[1].Init(sample_rate);
  filters[2].Init(sample_rate);
  filters[3].Init(sample_rate);

  filters[0].SetInputDrive(1.f);
  filters[1].SetInputDrive(1.f);
  // filters[2].SetInputDrive(1.f);
  // filters[3].SetInputDrive(1.f);

  filters[0].SetFilterMode(LadderFilter::FilterMode::LP12);
  filters[1].SetFilterMode(LadderFilter::FilterMode::LP12);
  filters[2].SetFilterMode(LadderFilter::FilterMode::BP12);
  filters[3].SetFilterMode(LadderFilter::FilterMode::BP12);

  smooth[0].Init(sample_rate);
  smooth[1].Init(sample_rate);
  smooth[2].Init(sample_rate);

  smooth[0].SetFreq(10.f);
  smooth[1].SetFreq(12.f);
  smooth[2].SetFreq(11.f);

  // Init Delay Lines
  del_left.Init();
  del_right.Init();

  // Set Delay Times in Samples
  del_left.SetDelay(12000.0f);
  del_right.SetDelay(8000.0f);

  DAISY.begin(AudioCallback);
}

void loop()
{
  float cutoff = (GetCtrl(POT_1) * 15000.f);

  float spread_left = (GetCtrl(POT_2) * 5000.f) + (smooth[0].Process() * 1000.f);
  float spread_right = (GetCtrl(POT_3) * 5000.f) + (smooth[1].Process() * 1000.f);
  float resonance = fmap(GetCtrl(POT_4), 0.f, 0.9f);

  filters[0].SetFreq(cutoff);
  filters[1].SetFreq(cutoff);
  filters[2].SetFreq(fabsf(cutoff - spread_left));
  filters[3].SetFreq(fabsf(cutoff - spread_right));

  filters[0].SetRes(resonance);
  filters[1].SetRes(resonance);
  filters[2].SetRes(resonance);
  filters[3].SetRes(resonance);

  Serial.println(cutoff);
  Serial.println(fabsf(cutoff - spread_left));
  Serial.println(fabsf(cutoff - spread_right));
}

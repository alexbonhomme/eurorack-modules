// ============================================================
//  Eurorack Dual-Channel Oscilloscope
//  Hardware : Raspberry Pi Pico + SH1106 128x64 (SPI)
//  Library  : U8g2 (install via Arduino Library Manager)
// ============================================================
//
//  PINOUT
//  ─────────────────────────────────────────────────────────
//  SH1106 SPI
//    SCK   → GP18
//    MOSI  → GP19
//    CS    → GP17
//    DC    → GP16
//    RST   → GP15
//
//  SH1106 I2C
//    SDA   → GP4
//    SCL   → GP5
//
//  Inputs
//    Channel 1  → GP26 (A0)
//    Channel 2  → GP27 (A1)
//    Time/Div   → GP28 (A2)   (pot wiper, 0–3.3V)
//    Volts/Div  → GP29 (A3)   (pot wiper, 0–3.3V)
//
//  Power
//    Eurorack +5V  → Pico VSYS
//    Eurorack GND  → GND
//    Eurorack +12V → TL072 V+
//    Eurorack -12V → TL072 V-
// ============================================================

#define VERSION "0.1.0"

#include <Arduino.h>
// #include <SPI.h>
#include <U8g2lib.h>

// ── Display constructor (SH1106, hardware SPI/I2C) ──────────────
// U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI
// display(U8G2_R0, /* cs= */ 17, /* dc= */ 16, /* rst= */ 15);
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset= */ U8X8_PIN_NONE);

// ── ADC pins ────────────────────────────────────────────────
#define PIN_CH1 A2     // GP28
#define PIN_CH2 A3     // GP29
#define PIN_TIMEDIV A0 // GP26
#define PIN_VOLTDIV A1 // GP27

// ── Oscilloscope settings ───────────────────────────────────
#define SCREEN_W 128
#define SCREEN_H 64
#define STATUS_Y 5                           // Y position for T/V text at top
#define WAVEFORM_TOP 8                       // top of waveform area (pixels)
#define WAVEFORM_H (SCREEN_H - WAVEFORM_TOP) // waveform height in pixels
#define SAMPLES SCREEN_W                     // one sample per horizontal pixel

// Vertical scaling: assume at vdiv_scale = 1.0 the screen shows 10 Vpp.
// With 8 vertical divisions, that means 10/8 V per division at vdiv_scale = 1.0.
// At other scales, V/div is (BASE_VPP_AT_VSCALE_1 * vdiv_scale) / VERTICAL_DIVS.
#define VERTICAL_DIVS 8
#define BASE_VPP_AT_VSCALE_1 10.0f

// ADC mid-scale (Pico = 12-bit → 0–4095, mid = ~2048)
// After op-amp conditioning a 0V eurorack signal should land here
#define ADC_MID 2048
#define ADC_MAX 4095

// Time/Div range : min and max microseconds between samples
#define TDIV_MIN_US 20
#define TDIV_MAX_US 5000

// Trigger: rising edge, search window in samples
#define TRIGGER_SEARCH 512 // max samples to scan for trigger

// ── Sample buffers ──────────────────────────────────────────
int16_t buf1[SAMPLES];
int16_t buf2[SAMPLES];

// ── Helpers ─────────────────────────────────────────────────

// Map an ADC value to a Y pixel coordinate inside the waveform area.
// vdiv_scale: 1.0 = full range fills screen, 0.5 = zoomed in x2, etc.
// midADC: per-channel DC offset (average ADC value for this frame).
int16_t adcToY(int16_t adcVal, float vdiv_scale, int16_t midADC) {
  // Centre around midADC, scale, then flip (Y=0 is top)
  float normalized = (adcVal - midADC) / (float)(ADC_MAX / 2); // -1.0 .. +1.0
  normalized /= vdiv_scale;                                    // apply zoom
  normalized = constrain(normalized, -1.0f, 1.0f);
  return (int16_t)(WAVEFORM_TOP + (WAVEFORM_H / 2) -
                   normalized * (WAVEFORM_H / 2));
}

// Find a rising-edge trigger point in a pre-filled oversize buffer.
// Returns the starting index or 0 if not found.
int findTrigger(int16_t *buf, int bufLen) {
  int midADC = ADC_MID;
  for (int i = 1; i < bufLen - SAMPLES; i++) {
    if (buf[i - 1] < midADC && buf[i] >= midADC) {
      return i;
    }
  }
  return 0; // no trigger found, display from start
}

// ── Setup ────────────────────────────────────────────────────
void setup() {
  analogReadResolution(12); // Pico supports 12-bit ADC

  display.begin();
  display.setFont(u8g2_font_4x6_tr); // tiny font for status bar
  display.setDrawColor(1);

  // Brief splash (centered)
  display.clearBuffer();

  const char *title = "PICO SCOPE";
  display.setFont(u8g2_font_6x10_tr);
  int titleW = display.getStrWidth(title);
  int titleX = (SCREEN_W - titleW) / 2;
  int titleY = (SCREEN_H / 2) - 2;
  display.drawStr(titleX, titleY, title);

  display.setFont(u8g2_font_4x6_tr);
  int verW = display.getStrWidth(VERSION);
  int verX = (SCREEN_W - verW) / 2;
  int verY = titleY + 10;
  display.drawStr(verX, verY, VERSION);

  display.sendBuffer();
  delay(2000);
}

// ── Main loop ────────────────────────────────────────────────
void loop() {
  // 1. Read pots ─────────────────────────────────────────────
  int timePotRaw = analogRead(PIN_TIMEDIV); // 0–4095
  int voltPotRaw = analogRead(PIN_VOLTDIV); // 0–4095

  // Time/Div: map pot to sample interval in microseconds
  uint32_t sampleDelayUs = map(timePotRaw, 0, 4095, TDIV_MIN_US, TDIV_MAX_US);

  // Volts/Div: map pot to a vertical scale factor
  //   pot=0   → vscale=0.25 (zoomed in, small signals fill screen)
  //   pot=max → vscale=2.0  (zoomed out, ±10V fits)
  float vdiv_scale = 0.25f + (voltPotRaw / 4095.0f) * 1.75f;

  // 2. Capture with trigger ──────────────────────────────────
  // Oversample into a larger buffer so we can search for trigger
  int16_t captureBuf1[SAMPLES + TRIGGER_SEARCH];
  int16_t captureBuf2[SAMPLES + TRIGGER_SEARCH];

  for (int i = 0; i < SAMPLES + TRIGGER_SEARCH; i++) {
    // CH1 (hardware inverted)
    captureBuf1[i] = ADC_MAX - analogRead(PIN_CH1);

    // CH2 (hardware not implemented yet)
    // captureBuf2[i] = ADC_MAX - analogRead(PIN_CH2);
    captureBuf2[i] = 0.0f;

    if (sampleDelayUs > 0)
      delayMicroseconds(sampleDelayUs);
  }

  // Find rising-edge trigger on CH1
  int trigIdx = findTrigger(captureBuf1, SAMPLES + TRIGGER_SEARCH);

  // Copy triggered window into display buffers
  for (int i = 0; i < SAMPLES; i++) {
    buf1[i] = captureBuf1[trigIdx + i];
    buf2[i] = captureBuf2[trigIdx + i];
  }

  // Compute per-channel DC offsets so each waveform is vertically centered
  long sum1 = 0;
  long sum2 = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum1 += buf1[i];
    sum2 += buf2[i];
  }
  int16_t mid1 = (int16_t)(sum1 / SAMPLES);
  int16_t mid2 = (int16_t)(sum2 / SAMPLES);

  // 3. Draw ──────────────────────────────────────────────────
  display.clearBuffer();

  // Centre line (ground reference)
  int midY = WAVEFORM_TOP + WAVEFORM_H / 2;
  for (int x = 0; x < SCREEN_W; x += 4) {
    display.drawPixel(x, midY); // dotted centre line
  }

  // Waveforms
  for (int x = 0; x < SAMPLES - 1; x++) {
    int y1a = adcToY(buf1[x], vdiv_scale, mid1);
    int y1b = adcToY(buf1[x + 1], vdiv_scale, mid1);
    int y2a = adcToY(buf2[x], vdiv_scale, mid2);
    int y2b = adcToY(buf2[x + 1], vdiv_scale, mid2);

    // CH1 — solid line
    display.drawLine(x, y1a, x + 1, y1b);

    // CH2 — draw only on even pixels to visually distinguish channels
    if (x % 2 == 0) {
      display.drawPixel(x, y2a);
    }
  }

  // CH1 / CH2 labels on left edge (font reused for status text)
  display.setFont(u8g2_font_4x6_tr);

  // Status text: Time/Div on top-left, Volts/Div on top-right
  char timeStr[12];
  if (sampleDelayUs < 1000) {
    snprintf(timeStr, sizeof(timeStr), "%luus", (unsigned long)sampleDelayUs);
  } else {
    snprintf(timeStr, sizeof(timeStr), "%.1fms", sampleDelayUs / 1000.0f);
  }

  char tLabel[16];
  snprintf(tLabel, sizeof(tLabel), "%s", timeStr);
  display.drawStr(2, STATUS_Y, tLabel);

  // Compute and display actual Volts/Div instead of raw scale factor
  float vpp = BASE_VPP_AT_VSCALE_1 * vdiv_scale;
  float vPerDiv = vpp / VERTICAL_DIVS;

  char vLabel[16];
  snprintf(vLabel, sizeof(vLabel), "%.2fv/div", vPerDiv);
  int vX = SCREEN_W - display.getStrWidth(vLabel) - 2;
  if (vX < 0) {
    vX = 0;
  }
  display.drawStr(vX, STATUS_Y, vLabel);

  display.sendBuffer();

  // 4. No fixed delay — loop rate is determined by capture time
}


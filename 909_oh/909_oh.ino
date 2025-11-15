#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <string.h>
#include "hardware/timer.h"

// === Pin configuration ===
const int AUDIO_PIN = 6;  // PWM audio output
const int GATE_PIN = 1;   // External gate input
const int PITCH_PIN = A0; // Pitch potentiometer
const int DECAY_PIN = A1; // Decay potentiometer
const int SD_CS_PIN = 5;  // SD card chip select (adjust as needed)

// === Sample data ===
int16_t *sampleData = nullptr;            // Dynamically allocated sample buffer (16-bit signed)
volatile int SAMPLE_LEN = 0;              // Sample length (volatile for ISR access)
volatile uint32_t SAMPLE_RATE_HZ = 44100; // Sample rate from WAV file (volatile for ISR access)

// === Playback state (volatile for ISR access) ===
volatile bool playing = false;
volatile float volume = 0.0f;
volatile float decay = 0.99f;
volatile float pos = 0.0f;
volatile float pitch = 1.0f;

// === Timer configuration ===
// Using hardware timer for precise timing
// Timer interval will be set based on RAW_SAMPLE_RATE
struct repeating_timer sampleTimer;

// === SD card sample loading ===
const char *SAMPLE_FILENAME_OH = "909_oh.raw";
const char *SAMPLE_FILENAME_CH = "909_ch.raw";
// const char *SAMPLE_FILENAME = "909_oh_original.raw";

const uint32_t RAW_SAMPLE_RATE = 44100; // Sample rate for raw files (adjust as needed)
const int RAW_BIT_DEPTH = 16;           // Bit depth: 8 for 8-bit unsigned (0-255), 16 for 16-bit
const bool RAW_BIG_ENDIAN = true;       // true for big-endian, false for little-endian (most files are little-endian)
const bool RAW_16BIT_SIGNED = true;     // true for signed 16-bit (-32768 to 32767), false for unsigned (0 to 65535)

// === Interrupt Service Routine for sample playback ===
bool onTimer(struct repeating_timer *t)
{
  // Check if sample data is loaded
  if (sampleData == nullptr || SAMPLE_LEN == 0)
  {
    analogWrite(AUDIO_PIN, 32767); // mid-level if no sample loaded
    return true;
  }

  if (playing)
  {
    int idx = (int)pos;
    int sampleLen = SAMPLE_LEN; // Local copy for safety

    if (idx >= sampleLen)
    {
      playing = false;
      analogWrite(AUDIO_PIN, 32767); // mid-level when done
    }
    else
    {
      // Linear interpolation for smoother pitch
      float frac = pos - idx;
      int16_t s0 = sampleData[idx];
      int16_t s1 = (idx + 1 < sampleLen) ? sampleData[idx + 1] : 0;
      float sample = s0 + frac * (s1 - s0);

      // Apply volume envelope
      sample = sample * volume;

      // Clamp sample to 16-bit signed range
      if (sample > 32767.0f)
        sample = 32767.0f;
      if (sample < -32768.0f)
        sample = -32768.0f;

      // Convert from 16-bit signed (-32768 to 32767) to PWM range (0 to 65535)
      // Center (0) maps to 32767 (mid-level)
      int32_t sampleInt = (int32_t)sample;
      uint16_t pwmValue = (uint16_t)(sampleInt + 32768);

      // Output to PWM
      analogWrite(AUDIO_PIN, pwmValue);

      // Update envelope and position
      volume *= decay;
      pos += pitch;
    }
  }
  else
  {
    analogWrite(AUDIO_PIN, 32767); // keep at mid-level when idle
  }

  return true; // Return true to keep repeating
}

bool loadSampleFromSD(const char *sampleFilename)
{
  File sampleFile = SD.open(sampleFilename, FILE_READ);
  if (!sampleFile)
  {
    Serial.print("ERROR: Failed to open file '");
    Serial.print(sampleFilename);
    Serial.println("'");
    return false;
  }

  // Get file size
  size_t fileSize = sampleFile.size();
  Serial.print("File opened successfully, size: ");
  Serial.print(fileSize);
  Serial.println(" bytes");

  // Ensure we're at the beginning of the file
  sampleFile.seek(0);

  // Calculate number of samples based on bit depth
  int numSamples;
  if (RAW_BIT_DEPTH == 8)
  {
    numSamples = fileSize; // 1 byte per sample
  }
  else if (RAW_BIT_DEPTH == 16)
  {
    if (fileSize % 2 != 0)
    {
      sampleFile.close();
      Serial.println("ERROR: 16-bit file size must be even (file size must be multiple of 2)");
      return false;
    }
    numSamples = fileSize / 2; // 2 bytes per sample
  }
  else
  {
    sampleFile.close();
    Serial.print("ERROR: Unsupported bit depth: ");
    Serial.println(RAW_BIT_DEPTH);
    return false;
  }

  // Limit sample count (max ~100KB output = 100000 samples for 8-bit)
  if (numSamples == 0 || numSamples > 100000)
  {
    sampleFile.close();
    Serial.println("ERROR: Sample count out of range (1-100000 samples)");
    return false;
  }

  Serial.print("Bit depth: ");
  Serial.print(RAW_BIT_DEPTH);
  Serial.print("-bit, Samples: ");
  Serial.println(numSamples);

  // Allocate buffer for raw file data
  uint8_t *rawData = (uint8_t *)malloc(fileSize);
  if (rawData == nullptr)
  {
    sampleFile.close();
    Serial.println("ERROR: Failed to allocate memory for raw file data");
    return false;
  }

  // Read file in chunks (some SD libraries have issues with large single reads)
  const size_t CHUNK_SIZE = 512; // Read 512 bytes at a time
  size_t totalBytesRead = 0;
  size_t bytesRemaining = fileSize;

  Serial.print("Reading file in chunks... ");

  while (bytesRemaining > 0 && sampleFile.available())
  {
    size_t chunkSize = (bytesRemaining > CHUNK_SIZE) ? CHUNK_SIZE : bytesRemaining;
    int bytesRead = sampleFile.read(&rawData[totalBytesRead], chunkSize);

    if (bytesRead <= 0)
    {
      // Error reading
      sampleFile.close();
      free(rawData);
      Serial.println();
      Serial.print("ERROR: Read failed at byte ");
      Serial.print(totalBytesRead);
      Serial.print(" of ");
      Serial.println(fileSize);
      return false;
    }

    totalBytesRead += bytesRead;
    bytesRemaining -= bytesRead;
  }

  sampleFile.close();

  if (totalBytesRead != fileSize)
  {
    free(rawData);
    Serial.println();
    Serial.print("ERROR: Read ");
    Serial.print(totalBytesRead);
    Serial.print(" bytes, expected ");
    Serial.println(fileSize);
    return false;
  }

  Serial.print(totalBytesRead);
  Serial.println(" bytes read successfully");

  // Allocate memory for 16-bit sample data
  sampleData = (int16_t *)malloc(numSamples * sizeof(int16_t));
  if (sampleData == nullptr)
  {
    free(rawData);
    Serial.println("ERROR: Failed to allocate memory for sample data");
    return false;
  }

  // Convert to 16-bit signed samples
  if (RAW_BIT_DEPTH == 8)
  {
    // 8-bit: convert unsigned (0-255, center at 128) to signed 16-bit (-32768 to 32767)
    for (int i = 0; i < numSamples; i++)
    {
      // Convert from 8-bit unsigned (0-255, center at 128) to 16-bit signed
      int16_t sample8 = (int16_t)rawData[i] - 128; // Now -128 to 127
      sampleData[i] = sample8 * 256;               // Scale to 16-bit range (-32768 to 32512)
    }
  }
  else if (RAW_BIT_DEPTH == 16)
  {
    // 16-bit: read with correct endianness and signed/unsigned format
    for (int i = 0; i < numSamples; i++)
    {
      uint16_t sample16;
      if (RAW_BIG_ENDIAN)
      {
        // Big-endian: MSB first
        sample16 = (rawData[i * 2] << 8) | rawData[i * 2 + 1];
      }
      else
      {
        // Little-endian: LSB first (most common)
        sample16 = rawData[i * 2] | (rawData[i * 2 + 1] << 8);
      }

      if (RAW_16BIT_SIGNED)
      {
        // Signed: convert from unsigned to signed
        sampleData[i] = (int16_t)sample16;
      }
      else
      {
        // Unsigned: convert from 0-65535 to -32768 to 32767
        sampleData[i] = (int16_t)(sample16 - 32768);
      }
    }
  }

  // Free raw data buffer
  free(rawData);

  // Set sample length and rate (disable interrupts briefly for atomic update)
  noInterrupts();
  SAMPLE_LEN = numSamples;
  SAMPLE_RATE_HZ = RAW_SAMPLE_RATE;
  interrupts();

  Serial.println("Sample loaded successfully!");
  return true;
}

void setup()
{
  // Initialize serial for debugging
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }

  Serial.println("Serial initialized");

  pinMode(GATE_PIN, INPUT_PULLDOWN);
  analogReadResolution(12);

  // Initialize SPI for SD card
  // Adjust these pin numbers to match your hardware configuration
  // Default SPI pins for Pico: MISO=16, MOSI=19, SCK=18
  // You can also use SPI.begin() without setRX/setTX/setSCK to use defaults
  SPI.setRX(4);  // MISO - adjust pins as needed for your board
  SPI.setTX(3);  // MOSI
  SPI.setSCK(2); // SCK
  SPI.begin();

  // Initialize SD card with retry
  bool sdInitialized = false;
  for (int i = 0; i < 5; i++)
  {
    if (SD.begin(SD_CS_PIN))
    {
      sdInitialized = true;
      Serial.println("SD card initialized successfully!");
      delay(100); // Give SD card time to stabilize
      break;
    }

    Serial.print("SD card initialization attempt ");
    Serial.print(i + 1);
    Serial.println(" failed, retrying...");
    delay(500);
  }

  if (!sdInitialized)
  {
    Serial.println("ERROR: SD card initialization failed after 5 attempts!");
    Serial.println("Check: CS pin, wiring, card format (FAT32), power supply");
    return; // Don't continue if SD card failed
  }

  // List files on SD card for debugging
  // Serial.println("\nFiles on SD card:");
  // File root = SD.open("/");
  // if (root)
  // {
  //   File entry = root.openNextFile();
  //   int fileCount = 0;
  //   while (entry)
  //   {
  //     fileCount++;
  //     Serial.print("  ");
  //     Serial.print(entry.name());
  //     if (!entry.isDirectory())
  //     {
  //       Serial.print(" (");
  //       Serial.print(entry.size());
  //       Serial.println(" bytes)");
  //     }
  //     else
  //     {
  //       Serial.println(" [DIR]");
  //     }
  //     entry.close();
  //     entry = root.openNextFile();
  //   }
  //   root.close();
  //   if (fileCount == 0)
  //   {
  //     Serial.println("  (no files found)");
  //   }
  //   Serial.println();
  // }
  // else
  // {
  //   Serial.println("  (could not open root directory)");
  // }

  // Configure PWM for audio output
  analogWriteFreq(200000);       // ~200 kHz sample rate
  analogWriteRange(65535);       // 16-bit resolution
  analogWrite(AUDIO_PIN, 32767); // mid-level idle voltage

  // Load sample from SD card
  Serial.print("Attempting to load: ");
  Serial.println(SAMPLE_FILENAME_OH);

  if (!loadSampleFromSD(SAMPLE_FILENAME_OH))
  {
    Serial.println("ERROR: Sample loading failed!");
  }
  else
  {
    // Sample loaded successfully - setup timer with the file's sample rate
    // Calculate timer interval based on loaded sample rate
    uint32_t sampleRate = SAMPLE_RATE_HZ; // Read once (atomic)
    int sampleIntervalUs = 1000000 / sampleRate;

    Serial.print("Sample loaded: ");
    Serial.print(SAMPLE_LEN);
    Serial.print(" samples at ");
    Serial.print(sampleRate);
    Serial.println(" Hz");

    // Setup hardware timer for sample playback interrupt
    // Negative interval means delay in microseconds
    add_repeating_timer_us(-sampleIntervalUs, onTimer, NULL, &sampleTimer);
  }
}

void loop()
{
  // === Read potentiometers ===
  int rawPitch = analogRead(PITCH_PIN);
  int rawDecay = analogRead(DECAY_PIN);

  // Map pot values to useful ranges
  // Use noInterrupts/interrupts to safely update shared variables
  noInterrupts();
  pitch = 0.5f + (rawPitch / 4095.0f) * 1.5f;     // 0.25× to 2× playback rate
  decay = 0.998f + (rawDecay / 4095.0f) * 0.002f; // 0.998–1.00 (fast to slow decay)
  interrupts();

  // === Detect trigger edge ===
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
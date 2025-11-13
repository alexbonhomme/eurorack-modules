import wave
import struct
import sys
import numpy as np

# === CONFIG ===
INPUT_FILE = "909_oh.wav"    # path to your sample
OUTPUT_FILE = "sample_data.h"
TARGET_SAMPLE_RATE = 22050   # Hz, to match Arduino playback rate
TARGET_BITS = 8              # output bit depth

# === Helper functions ===
def read_wav(filename):
    with wave.open(filename, 'rb') as wf:
        n_channels = wf.getnchannels()
        sample_width = wf.getsampwidth()
        framerate = wf.getframerate()
        n_frames = wf.getnframes()
        data = wf.readframes(n_frames)
    return data, n_channels, sample_width, framerate, n_frames

def convert_to_mono(samples, n_channels):
    if n_channels == 2:
        samples = samples.reshape(-1, 2)
        samples = samples.mean(axis=1)
    return samples

# === Read and process WAV ===
data, n_channels, width, rate, n_frames = read_wav(INPUT_FILE)

# Decode based on bit depth
if width == 2:
    fmt = "<{}h".format(n_frames * n_channels)
    samples = np.array(struct.unpack(fmt, data), dtype=np.float32)
    samples /= 32768.0
elif width == 1:
    fmt = "<{}B".format(n_frames * n_channels)
    samples = np.array(struct.unpack(fmt, data), dtype=np.float32)
    samples = (samples - 128.0) / 128.0
else:
    raise ValueError("Unsupported bit depth")

samples = convert_to_mono(samples, n_channels)

# Resample if needed
if rate != TARGET_SAMPLE_RATE:
    import scipy.signal
    samples = scipy.signal.resample_poly(samples, TARGET_SAMPLE_RATE, rate)

# Normalize and quantize to 8-bit unsigned
samples = np.clip(samples, -1.0, 1.0)
samples_u8 = ((samples + 1.0) * 127.5).astype(np.uint8)

# === Write C header ===
with open(OUTPUT_FILE, "w") as f:
    f.write("// Converted from {}\n".format(INPUT_FILE))
    f.write("const uint8_t sampleData[] = {\n  ")
    for i, s in enumerate(samples_u8):
        f.write("{:d},".format(s))
        if (i + 1) % 16 == 0:
            f.write("\n  ")
    f.write("\n};\n")
    f.write("const int SAMPLE_LEN = {};\n".format(len(samples_u8)))

print(f"✅ Conversion complete: {len(samples_u8)} samples written to {OUTPUT_FILE}")
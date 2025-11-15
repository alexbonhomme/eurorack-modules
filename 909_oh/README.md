# 909 hats

> WIP

Convert to 16 bits

```
ffmpeg -i input.wav -ac 1 -sample_fmt s16 output.wav
```

```
ffmpeg -i file.wav -f s16be -ar 44100 -acodec pcm_s16be file.raw
```

Convert sample to C array

```
python wav_converter.py
```

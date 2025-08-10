# wave.h
A minimal single-header C library for reading and writing **.wav** audio files in **PCM** format.

## Features
- **Load** `.wav` files (PCM audio format only)
- **Save** `.wav` files with custom sample rate, channels, and bit depth
- **Free** loaded audio memory with a single function
- Supports:
  - 8-bit unsigned integer samples
  - 16-bit unsigned integer samples
  - 32-bit floating-point samples
- Validates WAV file structure
- Single-header, no dependencies
- Cross-platform (standard C)

---

## Usage

### Include in Your Project
Just drop `wave.h` into your project and:
```c
#include "wave.h"
```

---

### Reading a WAV File
```c
#include "wave.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    wave_t wav = wave_open(L"audio.wav");

    if (wav.is_loaded) {
        printf("Channels: %d\n", wav.fmt_chunk.num_channels);
        printf("Sample rate: %d Hz\n", wav.fmt_chunk.sample_rate);
        printf("Bits per sample: %d\n", wav.fmt_chunk.bits_per_sample);

        // Raw PCM data
        uint8_t* data = wav.data_chunk.data;

        // Free memory when done
        wave_free(&wav);
    }

    return 0;
}
```

---

### Saving a WAV File
```c
#include "wave.h"
#include <stdint.h>

int main() {
    int16_t samples[44100]; // 1 second of mono 16-bit audio @ 44100Hz
    // Fill `samples` with your audio data here...

    wave_save(L"output.wav", (cint8_t*)samples,
              sizeof(samples) * sizeof(int16_t),
              1,               // num_channels
              44100,           // sample_rate
              suint16);        // sample_type

    return 0;
}
```

---

## API

### `wave_t wave_open(const wchar_t *filepath)`
Loads a `.wav` file from disk (PCM only).  
- **Parameters:**  
  - `filepath` → path to the `.wav` file (wide string, UTF-16 on Windows)
- **Returns:** `wave_t` struct containing:
  - `riff_chunk` → RIFF header info
  - `fmt_chunk` → audio format details
  - `data_chunk` → raw PCM audio data
  - `is_loaded` → 1 if successful
- **Notes:** Use `wave_free` to release allocated memory.

---

### `void wave_save(const wchar_t *filepath, uint8_t *data, uint32_t data_size, uint16_t num_channels, uint32_t sample_rate, sample_type_t sample_type)`
Writes raw PCM data to a `.wav` file.  
- **Parameters:**  
  - `filepath` → path to save file (wide string)
  - `data` → pointer to raw PCM audio
  - `data_size` → size of `data` in bytes
  - `num_channels` → number of channels (1 = mono, 2 = stereo, etc.)
  - `sample_rate` → samples per second (e.g., 44100)
  - `sample_type` → `suint8`, `suint16`, or `sfloat32`

---

### `void wave_free(wave_t *wave)`
Frees memory allocated by `wave_open`.  
- **Parameters:**  
  - `wave` → pointer to a `wave_t` struct returned by `wave_open`.

---

## Limitations
- PCM format only (no compressed formats)
- Little-endian `.wav` files only
- Loads full file into memory (no streaming)

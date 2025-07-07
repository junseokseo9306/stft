# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C project that uses the KISS FFT (Keep It Simple, Stupid Fast Fourier Transform) library. The codebase contains:

- **kiss_fft.h/kiss_fft.c**: The main KISS FFT library implementation
- **_kiss_fft_guts.h**: Internal FFT implementation details and macros
- **kiss_fft_log.h**: Logging utilities for the FFT library
- **example.c**: Example program demonstrating FFT usage
- **example**: Compiled binary from example.c

## Building and Running

### Compilation
```bash
# Compile the example program
gcc -o example example.c kiss_fft.c -lm

# Run the example
./example
```

### Key Build Notes
- Link with math library (`-lm`) for mathematical functions like `sin()`, `cos()`
- Include `kiss_fft.c` in compilation as it's not a pre-compiled library

## Code Architecture

### KISS FFT Library Structure
- **kiss_fft_cfg**: Configuration handle for FFT operations
- **kiss_fft_cpx**: Complex number structure with `.r` (real) and `.i` (imaginary) components
- **kiss_fft_alloc()**: Allocates FFT configuration for given size
- **kiss_fft()**: Performs the actual FFT transform
- **kiss_fft_free()**: Releases allocated configuration

### Typical Usage Pattern
```c
// Allocate FFT configuration
kiss_fft_cfg cfg = kiss_fft_alloc(N, 0, NULL, NULL);

// Initialize input array with signal data
kiss_fft_cpx in[N], out[N];
for(int i = 0; i < N; i++) {
    in[i].r = /* real signal value */;
    in[i].i = /* imaginary signal value */;
}

// Perform FFT
kiss_fft(cfg, in, out);

// Clean up
kiss_fft_free(cfg);
```

### Important Implementation Details
- Input arrays must be properly initialized before calling `kiss_fft()`
- The library supports both forward and inverse FFT (controlled by `inverse_fft` parameter in `kiss_fft_alloc()`)
- FFT size should preferably be a power of 2 for optimal performance
- The library can work with fixed-point or floating-point arithmetic (controlled by compile-time defines)

## STFT Implementation

### Building with STFT
```bash
# Compile STFT example
gcc -o stft_example stft_example.c stft.c kiss_fft.c -lm

# Run STFT tests
gcc -o test_stft test_stft.c stft.c kiss_fft.c -lm
./test_stft
```

### STFT Usage Pattern
```c
#include "stft.h"

// Create STFT parameters
STFTParameters params = stft_create_parameters(
    1024,           // window_size
    512,            // hop_size (50% overlap)
    44100.0,        // sample_rate
    WINDOW_HANN     // window_type
);

// Perform STFT
STFTResult *result = perform_stft(signal, signal_length, &params);

if (result && result->success) {
    // Extract magnitude spectrogram
    float **magnitude = stft_get_magnitude_spectrogram(result);
    
    // Extract phase spectrogram
    float **phase = stft_get_phase_spectrogram(result);
    
    // Extract power spectrogram (dB)
    float **power_db = stft_get_power_spectrogram_db(result);
    
    // Access spectrogram data: magnitude[frame][frequency_bin]
    printf("Frame count: %d\n", result->frame_count);
    printf("Frequency bins: %d\n", result->frequency_bin_count);
    
    // Clean up
    stft_free_2d_array(magnitude, result->frame_count);
    stft_free_2d_array(phase, result->frame_count);
    stft_free_2d_array(power_db, result->frame_count);
}

stft_free_result(result);
```

### STFT Architecture
- **STFTParameters**: Configuration (window size, hop size, sample rate, window type)
- **STFTResult**: Contains spectrogram data [frame][frequency_bin] and metadata
- **Window Functions**: Currently supports Hann window with proper normalization
- **Signal Generation**: Utilities for creating test signals (sine waves, multi-tone, time-varying)
- **Performance**: Sub-millisecond execution for typical audio frame sizes

### Key Features
- **Hann Window**: Proper window normalization by window sum
- **Frame Processing**: Configurable overlap with hop size
- **Frequency Domain**: Returns positive frequencies only (DC to Nyquist)
- **Memory Management**: Proper allocation/deallocation with helper functions
- **Error Handling**: Comprehensive parameter validation and error reporting
- **Performance Timing**: Built-in timing measurement capabilities
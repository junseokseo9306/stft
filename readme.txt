KISS FFT STFT Implementation
============================

This project implements Short-Time Fourier Transform (STFT) using the KISS FFT library.

Files Overview
--------------
Core STFT Library:
- stft.h          - STFT library header with function declarations
- stft.c          - STFT implementation with minimal required functions
- stft_example.c  - Example program demonstrating STFT usage

KISS FFT Library:
- kiss_fft.h      - KISS FFT library header
- kiss_fft.c      - KISS FFT library implementation
- _kiss_fft_guts.h - Internal FFT implementation details

Test and Comparison:
- generate_scipy_stft.py - Python script to generate reference STFT using scipy
- scipy_stft.csv        - Reference STFT output from scipy
- stft_result.csv       - STFT output from C implementation

Building and Running
--------------------
Compile the STFT example:
    gcc -o stft_example stft_example.c stft.c kiss_fft.c -lm

Run the example:
    ./stft_example

This will generate stft_result.csv containing the STFT power spectrogram in dB.

STFT Parameters
---------------
The example uses these parameters to match the scipy reference:
- Sample rate: 125 Hz
- Window size: 62 samples
- Hop size: 31 samples (50% overlap)
- Window type: Hann window with energy normalization
- Signal: sin(2π×10×t) + 0.5×sin(2π×20×t) for 2 seconds

Key Features
------------
- Hann window with proper energy normalization
- Configurable window size and overlap
- Power spectrogram output in dB
- Memory management with proper cleanup
- Parameter validation and error handling

Usage Pattern
-------------
    #include "stft.h"
    
    // Create STFT parameters
    STFTParameters params = stft_create_parameters(
        window_size, hop_size, sample_rate, WINDOW_HANN
    );
    
    // Perform STFT
    STFTResult *result = perform_stft(signal, signal_length, &params);
    
    if (result && result->success) {
        // Extract power spectrogram in dB
        float **power_db = stft_get_power_spectrogram_db(result);
        
        // Use the data...
        
        // Clean up
        stft_free_2d_array(power_db, result->frame_count);
    }
    
    stft_free_result(result);

Dependencies
------------
- Standard C library (math.h, stdlib.h, string.h)
- KISS FFT library (included)
- Math library (-lm flag when compiling)

Notes
-----
- The implementation is optimized for minimal memory usage
- Only essential functions are included (unused functions removed)
- Compatible with scipy.signal.stft for reference comparison
- Supports real-valued input signals only
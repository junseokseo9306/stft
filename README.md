# KISS FFT STFT Implementation

A C implementation of Short-Time Fourier Transform (STFT) using the KISS FFT library.

## Project Structure

```
├── src/                    # Source code
│   ├── stft.c             # STFT implementation
│   ├── kiss_fft.c         # KISS FFT library
│   ├── kiss_fft.h         # KISS FFT header
│   ├── _kiss_fft_guts.h   # FFT internals
│   └── kiss_fft_log.h     # FFT logging
├── include/               # Public headers
│   └── stft.h            # STFT API
├── examples/              # Example programs
│   ├── stft_example.c    # Main STFT example
│   ├── example.c         # Basic FFT example
│   ├── generate_scipy_stft.py # Python reference
│   └── stft_ctypes.py    # Python bindings
├── tests/                 # Test files
│   └── test_stft.c       # STFT tests
├── docs/                  # Documentation
│   ├── readme.txt        # Detailed documentation
│   ├── CLAUDE.md         # Development notes
│   └── STFT_LIBRARY_USAGE.md # Usage guide
├── data/                  # Test data
│   ├── scipy_stft.csv    # Reference output
│   ├── stft_result.csv   # C implementation output
│   └── stft_result_python.csv # Python output
├── binaries/             # Compiled executables
└── Makefile             # Build system
```

## Quick Start

### Build
```bash
make
```

### Run Example
```bash
make run-example
```

This generates `stft_result.csv` with the STFT power spectrogram.

## Features

- **Minimal Implementation**: Only essential functions included
- **Hann Window**: Proper energy normalization
- **Configurable Parameters**: Window size, overlap, sample rate
- **Memory Management**: Proper allocation and cleanup
- **Error Handling**: Parameter validation and error reporting
- **Scipy Compatible**: Matches scipy.signal.stft output

## Usage

```c
#include "stft.h"

// Create parameters
STFTParameters params = stft_create_parameters(
    1024,           // window_size
    512,            // hop_size (50% overlap)
    44100.0,        // sample_rate
    WINDOW_HANN     // window_type
);

// Perform STFT
STFTResult *result = perform_stft(signal, signal_length, &params);

if (result && result->success) {
    // Extract power spectrogram
    float **power_db = stft_get_power_spectrogram_db(result);
    
    // Use the data...
    
    // Clean up
    stft_free_2d_array(power_db, result->frame_count);
}

stft_free_result(result);
```

## Dependencies

- Standard C library
- Math library (`-lm`)
- KISS FFT (included)

## License

This project uses the KISS FFT library which has its own license terms.
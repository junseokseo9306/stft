# STFT Library Usage Instructions

## Overview
This guide shows how to use the STFT (Short-Time Fourier Transform) library with Python using ctypes and a shared library (.so file).

## Prerequisites
- Python 3.6+
- NumPy
- GCC compiler
- The C source files: `stft.c`, `kiss_fft.c`, `stft.h`

## Step 1: Compile the Shared Library

First, compile the C code into a shared library:

```bash
gcc -shared -fPIC -o libstft.so stft.c kiss_fft.c -lm
```

**Command breakdown:**
- `-shared`: Creates a shared library
- `-fPIC`: Position Independent Code (required for shared libraries)
- `-o libstft.so`: Output filename
- `stft.c kiss_fft.c`: Source files to compile
- `-lm`: Links the math library

## Step 2: Verify the Library

Check that the library was created successfully:

```bash
ls -la libstft.so
file libstft.so
```

You should see output like:
```
libstft.so: Mach-O 64-bit dynamically linked shared library arm64
```

## Step 3: Install Python Dependencies

Install NumPy if not already installed:

```bash
pip install numpy
```

## Step 4: Use the Python Wrapper

The `stft_ctypes.py` file provides a Python interface to the shared library.

### Basic Usage Example

```python
import numpy as np
from stft_ctypes import perform_stft

# Generate a test signal (10 Hz + 20 Hz sine waves)
fs = 125.0  # Sample rate
duration = 2.0  # Duration in seconds
t = np.arange(0, duration, 1/fs)
signal = np.sin(2 * np.pi * 10 * t) + 0.5 * np.sin(2 * np.pi * 20 * t)
signal = signal.astype(np.float32)

# Perform STFT
result = perform_stft(
    signal=signal,
    window_size=64,
    hop_size=32,
    sample_rate=fs,
    window_type=0  # 0 = Hann window
)

# Access results
spectrogram = result['spectrogram']  # Power spectrogram in dB
frame_count = result['frame_count']
frequency_bins = result['frequency_bin_count']
frame_time = result['frame_time']
frequency_resolution = result['frequency_resolution']

print(f"Spectrogram shape: {spectrogram.shape}")
print(f"Frame count: {frame_count}")
print(f"Frequency bins: {frequency_bins}")
```

### Advanced Usage with STFTWrapper Class

```python
from stft_ctypes import STFTWrapper
import numpy as np

# Initialize wrapper
stft = STFTWrapper("./libstft.so")

# Generate signal
fs = 44100.0
t = np.arange(0, 1.0, 1/fs, dtype=np.float32)
signal = np.sin(2 * np.pi * 440 * t)  # 440 Hz tone

# Perform STFT
result = stft.perform_stft(
    signal=signal,
    window_size=1024,
    hop_size=512,
    sample_rate=fs
)

# Use results
spectrogram = result['spectrogram']
print(f"Spectrogram shape: {spectrogram.shape}")
```

## Step 5: Test the Setup

Run the built-in test:

```bash
python3 stft_ctypes.py
```

Expected output:
```
Testing ctypes wrapper...
✓ STFT successful: (7, 33)
Frame count: 7
Frequency bins: 33
```

## Parameters

### STFT Parameters
- **window_size**: Size of the analysis window (samples)
- **hop_size**: Number of samples between successive frames
- **sample_rate**: Audio sample rate (Hz)
- **window_type**: Window function type (0 = Hann window)

### Return Values
- **spectrogram**: 2D NumPy array [frames × frequency_bins] with power in dB
- **frame_count**: Number of time frames
- **frequency_bin_count**: Number of frequency bins
- **frame_time**: Time duration of each frame (seconds)
- **frequency_resolution**: Frequency resolution (Hz/bin)

## File Structure

Your project should have these files:
```
├── stft.c                 # STFT implementation
├── kiss_fft.c            # KISS FFT library
├── stft.h                # Header file
├── libstft.so            # Compiled shared library
└── stft_ctypes.py        # Python wrapper
```

## Troubleshooting

### "Failed to load libstft.so"
- Make sure the shared library is compiled: `gcc -shared -fPIC -o libstft.so stft.c kiss_fft.c -lm`
- Check the library exists: `ls -la libstft.so`
- Verify the path in your Python code

### "No module named 'numpy'"
- Install NumPy: `pip install numpy`

### "Input array must be float32"
- Convert your signal: `signal = signal.astype(np.float32)`

### "Input array must be C-contiguous"
- Make array contiguous: `signal = np.ascontiguousarray(signal)`

## Performance Notes

- The library uses optimized C code with KISS FFT
- Typical performance: sub-millisecond execution for audio frame sizes
- Memory is automatically managed (allocated and freed)
- All arrays are properly cleaned up after use

## Example Applications

1. **Audio Analysis**: Analyze music or speech signals
2. **Signal Processing**: Real-time spectral analysis
3. **Scientific Computing**: Time-frequency analysis of signals
4. **Audio Visualization**: Create spectrograms for display

## Comparison with scipy.signal.stft

The library provides similar functionality to `scipy.signal.stft` but with:
- Faster execution (optimized C code)
- Lower memory overhead
- Simplified interface
- Compatible output format
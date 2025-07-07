#!/usr/bin/env python3
"""
Ctypes wrapper for STFT library
Simpler alternative that doesn't require compilation
"""

import ctypes
import numpy as np
from ctypes import Structure, POINTER, c_int, c_double, c_float, c_char_p, c_bool

# Load the shared library (you'll need to compile it first)
# gcc -shared -fPIC -o libstft.so stft.c kiss_fft.c -lm

class STFTParameters(Structure):
    _fields_ = [
        ("window_size", c_int),
        ("hop_size", c_int),
        ("sample_rate", c_double),
        ("window_type", c_int),
    ]

class ComplexFloat(Structure):
    _fields_ = [("r", c_float), ("i", c_float)]

class STFTResult(Structure):
    _fields_ = [
        ("success", c_bool),
        ("spectrogram_data", POINTER(POINTER(ComplexFloat))),
        ("frame_count", c_int),
        ("frequency_bin_count", c_int),
        ("frame_time", c_double),
        ("frequency_resolution", c_double),
        ("message", c_char_p),
    ]

class STFTWrapper:
    def __init__(self, lib_path="./libstft.so"):
        """Initialize the STFT wrapper"""
        try:
            self.lib = ctypes.CDLL(lib_path)
        except OSError:
            print(f"Failed to load {lib_path}")
            print("Please compile first: gcc -shared -fPIC -o libstft.so stft.c kiss_fft.c -lm")
            raise
        
        # Define function signatures
        self._setup_function_signatures()
    
    def _setup_function_signatures(self):
        """Setup C function signatures"""
        
        # perform_stft function
        self.lib.perform_stft.argtypes = [
            POINTER(c_float),  # input_data
            c_int,             # input_length
            POINTER(STFTParameters)  # params
        ]
        self.lib.perform_stft.restype = POINTER(STFTResult)
        
        # stft_get_power_spectrogram_db function
        self.lib.stft_get_power_spectrogram_db.argtypes = [POINTER(STFTResult)]
        self.lib.stft_get_power_spectrogram_db.restype = POINTER(POINTER(c_float))
        
        # cleanup functions
        self.lib.stft_free_result.argtypes = [POINTER(STFTResult)]
        self.lib.stft_free_result.restype = None
        
        self.lib.stft_free_2d_array.argtypes = [POINTER(POINTER(c_float)), c_int]
        self.lib.stft_free_2d_array.restype = None
    
    def perform_stft(self, signal, window_size, hop_size, sample_rate, window_type=0):
        """
        Perform STFT on input signal
        
        Args:
            signal (numpy.ndarray): Input signal (will be converted to float32)
            window_size (int): Window size in samples
            hop_size (int): Hop size in samples  
            sample_rate (float): Sample rate in Hz
            window_type (int): Window type (0=Hann)
            
        Returns:
            dict: Dictionary containing spectrogram and metadata
        """
        
        # Convert input to float32
        if not isinstance(signal, np.ndarray):
            signal = np.array(signal, dtype=np.float32)
        elif signal.dtype != np.float32:
            signal = signal.astype(np.float32)
        
        # Ensure C-contiguous
        if not signal.flags['C_CONTIGUOUS']:
            signal = np.ascontiguousarray(signal)
        
        # Create parameters
        params = STFTParameters(
            window_size=window_size,
            hop_size=hop_size,
            sample_rate=sample_rate,
            window_type=window_type
        )
        
        # Call C function
        input_data = signal.ctypes.data_as(POINTER(c_float))
        result_ptr = self.lib.perform_stft(input_data, len(signal), ctypes.byref(params))
        
        if not result_ptr:
            raise RuntimeError("STFT computation failed")
        
        result = result_ptr.contents
        
        if not result.success:
            error_msg = result.message.decode('utf-8') if result.message else "Unknown error"
            self.lib.stft_free_result(result_ptr)
            raise RuntimeError(f"STFT failed: {error_msg}")
        
        # Get power spectrogram
        power_ptr = self.lib.stft_get_power_spectrogram_db(result_ptr)
        if not power_ptr:
            self.lib.stft_free_result(result_ptr)
            raise RuntimeError("Failed to get power spectrogram")
        
        # Convert to numpy array
        spectrogram = np.zeros((result.frame_count, result.frequency_bin_count), dtype=np.float32)
        
        for frame in range(result.frame_count):
            for bin in range(result.frequency_bin_count):
                spectrogram[frame, bin] = power_ptr[frame][bin]
        
        # Create result dictionary
        result_dict = {
            'spectrogram': spectrogram,
            'frame_count': result.frame_count,
            'frequency_bin_count': result.frequency_bin_count,
            'frame_time': result.frame_time,
            'frequency_resolution': result.frequency_resolution
        }
        
        # Cleanup
        self.lib.stft_free_2d_array(power_ptr, result.frame_count)
        self.lib.stft_free_result(result_ptr)
        
        return result_dict

# Convenience function
def perform_stft(signal, window_size, hop_size, sample_rate, window_type=0, lib_path="./libstft.so"):
    """
    Convenience function to perform STFT
    
    Args:
        signal (numpy.ndarray): Input signal
        window_size (int): Window size in samples
        hop_size (int): Hop size in samples
        sample_rate (float): Sample rate in Hz
        window_type (int): Window type (0=Hann)
        lib_path (str): Path to shared library
        
    Returns:
        dict: STFT result dictionary
    """
    wrapper = STFTWrapper(lib_path)
    return wrapper.perform_stft(signal, window_size, hop_size, sample_rate, window_type)

if __name__ == "__main__":
    # Test the wrapper
    print("Testing ctypes wrapper...")
    
    # Generate test signal
    fs = 125.0
    duration = 2.0
    t = np.arange(0, duration, 1/fs)
    signal = np.sin(2 * np.pi * 10 * t) + 0.5 * np.sin(2 * np.pi * 20 * t)
    signal = signal.astype(np.float32)
    
    try:
        result = perform_stft(signal, 62, 31, fs)
        print(f"✓ STFT successful: {result['spectrogram'].shape}")
        print(f"Frame count: {result['frame_count']}")
        print(f"Frequency bins: {result['frequency_bin_count']}")
        
        # Save to CSV file
        np.savetxt('stft_result_python.csv', result['spectrogram'], delimiter=',', fmt='%.18e')
        print("✓ CSV file saved as stft_result_python.csv")
        
    except Exception as e:
        print(f"✗ STFT failed: {e}")
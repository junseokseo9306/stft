#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "stft.h"

#define EPSILON 1e-4

typedef struct {
    int total;
    int passed;
    int failed;
} TestResults;

static TestResults results = {0, 0, 0};

void test_assert(int condition, const char* test_name) {
    results.total++;
    if (condition) {
        results.passed++;
        printf("✓ %s\n", test_name);
    } else {
        results.failed++;
        printf("✗ %s\n", test_name);
    }
}

int float_equals(double a, double b, double epsilon) {
    return fabs(a - b) < epsilon;
}

void test_hann_window() {
    int window_size = 1024;
    float *window = generate_hann_window(window_size);
    
    test_assert(window != NULL, "Hann window generation");
    
    if (window) {
        test_assert(float_equals(window[0], 0.0, EPSILON), "Hann window starts at zero");
        test_assert(float_equals(window[window_size-1], 0.0, EPSILON), "Hann window ends at zero");
        test_assert(float_equals(window[window_size/2], 1.0, EPSILON), "Hann window peak at center");
        
        double sum = 0.0;
        for (int i = 0; i < window_size; i++) {
            sum += window[i];
        }
        test_assert(sum > 0.0, "Hann window has positive sum");
        
        free(window);
    }
}

void test_stft_parameters() {
    STFTParameters params = stft_create_parameters(1024, 512, 44100.0, WINDOW_HANN);
    
    test_assert(params.window_size == 1024, "Parameter window size");
    test_assert(params.hop_size == 512, "Parameter hop size");
    test_assert(params.sample_rate == 44100.0, "Parameter sample rate");
    test_assert(params.window_type == WINDOW_HANN, "Parameter window type");
    
    double overlap = stft_get_overlap_percentage(&params);
    test_assert(float_equals(overlap, 50.0, EPSILON), "50% overlap calculation");
    
    double frame_time = stft_get_frame_time(&params);
    test_assert(float_equals(frame_time, 512.0/44100.0, EPSILON), "Frame time calculation");
    
    double freq_res = stft_get_frequency_resolution(&params);
    test_assert(float_equals(freq_res, 44100.0/1024.0, EPSILON), "Frequency resolution calculation");
    
    char *error = stft_validate_parameters(&params);
    test_assert(error == NULL, "Valid parameters validation");
    
    params.window_size = 0;
    error = stft_validate_parameters(&params);
    test_assert(error != NULL, "Invalid window size validation");
    if (error) free(error);
}

void test_stft_sine_wave() {
    double frequency = 440.0;
    double amplitude = 1.0;
    double duration = 1.0;
    double sample_rate = 44100.0;
    int sample_count;
    
    float *signal = generate_sine_wave(frequency, amplitude, duration, sample_rate, &sample_count);
    test_assert(signal != NULL, "Sine wave generation");
    test_assert(sample_count == 44100, "Sine wave sample count");
    
    if (signal) {
        STFTParameters params = stft_create_parameters(1024, 512, sample_rate, WINDOW_HANN);
        STFTResult *result = perform_stft(signal, sample_count, &params);
        
        test_assert(result != NULL, "STFT result creation");
        test_assert(result->success, "STFT computation success");
        
        if (result && result->success) {
            int expected_frames = (sample_count - params.window_size) / params.hop_size + 1;
            test_assert(result->frame_count == expected_frames, "STFT frame count");
            test_assert(result->frequency_bin_count == params.window_size / 2 + 1, "STFT frequency bin count");
            
            float **magnitude = stft_get_magnitude_spectrogram(result);
            test_assert(magnitude != NULL, "Magnitude spectrogram extraction");
            
            if (magnitude) {
                int expected_bin = (int)(frequency * params.window_size / sample_rate + 0.5);
                double max_magnitude = 0.0;
                int max_bin = 0;
                
                for (int bin = 0; bin < result->frequency_bin_count; bin++) {
                    if (magnitude[0][bin] > max_magnitude) {
                        max_magnitude = magnitude[0][bin];
                        max_bin = bin;
                    }
                }
                
                test_assert(abs(max_bin - expected_bin) <= 1, "Sine wave peak at correct frequency");
                
                stft_free_2d_array(magnitude, result->frame_count);
            }
            
            stft_free_result(result);
        }
        
        free(signal);
    }
}

void test_stft_multi_tone() {
    double frequencies[] = {220.0, 440.0, 880.0};
    double amplitudes[] = {0.5, 0.7, 0.3};
    int tone_count = 3;
    double duration = 1.0;
    double sample_rate = 44100.0;
    int sample_count;
    
    float *signal = generate_multi_tone_sine_wave(frequencies, amplitudes, tone_count, duration, sample_rate, &sample_count);
    test_assert(signal != NULL, "Multi-tone sine wave generation");
    
    if (signal) {
        STFTParameters params = stft_create_parameters(2048, 1024, sample_rate, WINDOW_HANN);
        STFTResult *result = perform_stft(signal, sample_count, &params);
        
        test_assert(result != NULL && result->success, "Multi-tone STFT computation");
        
        if (result && result->success) {
            float **magnitude = stft_get_magnitude_spectrogram(result);
            test_assert(magnitude != NULL, "Multi-tone magnitude spectrogram");
            
            if (magnitude) {
                int peaks_found = 0;
                for (int i = 0; i < tone_count; i++) {
                    int expected_bin = (int)(frequencies[i] * params.window_size / sample_rate + 0.5);
                    if (expected_bin < result->frequency_bin_count && magnitude[0][expected_bin] > 0.1) {
                        peaks_found++;
                    }
                }
                
                test_assert(peaks_found >= 2, "Multi-tone peaks detection");
                
                stft_free_2d_array(magnitude, result->frame_count);
            }
            
            stft_free_result(result);
        }
        
        free(signal);
    }
}

void test_stft_timing() {
    double sample_rate = 44100.0;
    double duration = 0.1;
    int sample_count;
    
    float *signal = generate_sine_wave(1000.0, 1.0, duration, sample_rate, &sample_count);
    test_assert(signal != NULL, "Timing test signal generation");
    
    if (signal) {
        STFTParameters params = stft_create_parameters(1024, 512, sample_rate, WINDOW_HANN);
        STFTResult *result = NULL;
        TimingResult *timing = perform_stft_with_timing(signal, sample_count, &params, &result);
        
        test_assert(timing != NULL, "Timing result creation");
        test_assert(timing->success, "Timing measurement success");
        test_assert(timing->execution_time_ns > 0, "Positive execution time");
        
        printf("  STFT execution time: %.2f ms\n", timing->execution_time_ns / 1000000.0);
        
        stft_free_result(result);
        stft_free_timing_result(timing);
        free(signal);
    }
}

void test_stft_edge_cases() {
    STFTParameters params = stft_create_parameters(1024, 512, 44100.0, WINDOW_HANN);
    
    float short_signal[100] = {0};
    STFTResult *result = perform_stft(short_signal, 100, &params);
    test_assert(result != NULL && !result->success, "Short signal handling");
    if (result) stft_free_result(result);
    
    float *null_signal = NULL;
    result = perform_stft(null_signal, 1000, &params);
    test_assert(result != NULL && !result->success, "Null signal handling");
    if (result) stft_free_result(result);
    
    params.window_size = 0;
    float valid_signal[2048] = {0};
    result = perform_stft(valid_signal, 2048, &params);
    test_assert(result != NULL && !result->success, "Invalid parameters handling");
    if (result) stft_free_result(result);
}

void test_spectrogram_extraction() {
    double sample_rate = 44100.0;
    double duration = 0.1;
    int sample_count;
    
    float *signal = generate_sine_wave(1000.0, 1.0, duration, sample_rate, &sample_count);
    test_assert(signal != NULL, "Test signal for spectrogram extraction");
    
    if (signal) {
        STFTParameters params = stft_create_parameters(1024, 512, sample_rate, WINDOW_HANN);
        STFTResult *result = perform_stft(signal, sample_count, &params);
        
        if (result && result->success) {
            float **magnitude = stft_get_magnitude_spectrogram(result);
            float **phase = stft_get_phase_spectrogram(result);
            float **power_db = stft_get_power_spectrogram_db(result);
            
            test_assert(magnitude != NULL, "Magnitude spectrogram extraction");
            test_assert(phase != NULL, "Phase spectrogram extraction");
            test_assert(power_db != NULL, "Power spectrogram extraction");
            
            if (magnitude && phase && power_db) {
                test_assert(magnitude[0][0] >= 0.0, "Magnitude is non-negative");
                test_assert(phase[0][0] >= -M_PI-0.01 && phase[0][0] <= M_PI+0.01, "Phase in valid range");
                test_assert(power_db[0][0] <= 0.0 || power_db[0][0] >= -200.0, "Power in reasonable dB range");
            }
            
            stft_free_2d_array(magnitude, result->frame_count);
            stft_free_2d_array(phase, result->frame_count);
            stft_free_2d_array(power_db, result->frame_count);
        }
        
        stft_free_result(result);
        free(signal);
    }
}

void test_time_varying_signal() {
    double sample_rate = 44100.0;
    double duration = 0.5;
    int sample_count;
    
    float *signal = generate_time_varying_signal(sample_rate, duration, &sample_count);
    test_assert(signal != NULL, "Time-varying signal generation");
    
    if (signal) {
        STFTParameters params = stft_create_parameters(1024, 256, sample_rate, WINDOW_HANN);
        STFTResult *result = perform_stft(signal, sample_count, &params);
        
        test_assert(result != NULL && result->success, "Time-varying signal STFT");
        
        if (result && result->success) {
            test_assert(result->frame_count > 10, "Multiple frames for time-varying signal");
            
            float **magnitude = stft_get_magnitude_spectrogram(result);
            if (magnitude) {
                int non_zero_frames = 0;
                for (int frame = 0; frame < result->frame_count; frame++) {
                    double frame_energy = 0.0;
                    for (int bin = 0; bin < result->frequency_bin_count; bin++) {
                        frame_energy += magnitude[frame][bin] * magnitude[frame][bin];
                    }
                    if (frame_energy > 0.01) non_zero_frames++;
                }
                
                test_assert(non_zero_frames > result->frame_count / 2, "Time-varying signal has energy");
                
                stft_free_2d_array(magnitude, result->frame_count);
            }
            
            stft_free_result(result);
        }
        
        free(signal);
    }
}

int main() {
    printf("Running STFT Tests...\n");
    printf("=====================\n");
    
    test_hann_window();
    test_stft_parameters();
    test_stft_sine_wave();
    test_stft_multi_tone();
    test_stft_timing();
    test_stft_edge_cases();
    test_spectrogram_extraction();
    test_time_varying_signal();
    
    printf("\nTest Results:\n");
    printf("=============\n");
    printf("Total: %d\n", results.total);
    printf("Passed: %d\n", results.passed);
    printf("Failed: %d\n", results.failed);
    printf("Success Rate: %.1f%%\n", (double)results.passed / results.total * 100);
    
    return results.failed == 0 ? 0 : 1;
}
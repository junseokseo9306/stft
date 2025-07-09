#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../include/stft.h"

int main() {
    float fs = 125.0;
    float duration = 2.0;
    int N = (int)(fs * duration);
    
    // Generate composite signal: sin(2π*10*t) + 0.5*sin(2π*20*t)
    float *signal = (float*)malloc(N * sizeof(float));
    for(int i = 0; i < N; i++) {
        float t = (float)i / fs;
        signal[i] = sin(2 * M_PI * 10 * t) + 0.5 * sin(2 * M_PI * 20 * t);
    }
    
    // Create STFT parameters
    STFTParameters params = stft_create_parameters(
        62,              // window_size
        31,              // hop_size (50% overlap)
        fs,              // sample_rate
        WINDOW_HANN,     // window_type
        SCALING_SPECTRUM // scaling (use spectrum scaling like scipy default)
    );
    
    // Perform STFT
    STFTResult *result = perform_stft(signal, N, &params);
    
    if (result && result->success) {
        // Extract power spectrogram in dB
        float **power_db = stft_get_power_spectrogram_db(result);
        
        // Save to CSV file
        FILE *csv_file = fopen("data/stft_result.csv", "w");
        if (csv_file) {
            // Write power data in dB (no header, just values)
            for(int frame = 0; frame < result->frame_count; frame++) {
                for(int f = 0; f < result->frequency_bin_count; f++) {
                    fprintf(csv_file, "%.18e", power_db[frame][f]);
                    if (f < result->frequency_bin_count - 1) {
                        fprintf(csv_file, ",");
                    }
                }
                fprintf(csv_file, "\n");
            }
            
            fclose(csv_file);
        }
        
        // Clean up
        stft_free_2d_array(power_db, result->frame_count);
    }
    
    stft_free_result(result);
    free(signal);
    return 0;
}
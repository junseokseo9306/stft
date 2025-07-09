#ifndef STFT_H
#define STFT_H

#include <stdbool.h>
#include "../src/kiss_fft.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WINDOW_HANN
} WindowType;

typedef enum {
    SCALING_SPECTRUM,
    SCALING_PSD
} ScalingType;

typedef struct {
    int window_size;
    int hop_size;
    double sample_rate;
    WindowType window_type;
    ScalingType scaling;
} STFTParameters;

typedef struct {
    bool success;
    kiss_fft_cpx **spectrogram_data;  // [frame][frequency_bin]
    int frame_count;
    int frequency_bin_count;
    double frame_time;
    double frequency_resolution;
    char *message;
} STFTResult;


STFTParameters stft_create_parameters(int window_size, int hop_size, double sample_rate, WindowType window_type, ScalingType scaling);
char* stft_validate_parameters(const STFTParameters *params);
double stft_get_overlap_percentage(const STFTParameters *params);
double stft_get_frame_time(const STFTParameters *params);
double stft_get_frequency_resolution(const STFTParameters *params);

float* generate_hann_window(int window_size);
float* generate_window(WindowType window_type, int window_size);

STFTResult* perform_stft(const float *input_data, int input_length, const STFTParameters *params);

float** stft_get_magnitude_spectrogram(const STFTResult *result);
float** stft_get_phase_spectrogram(const STFTResult *result);
float** stft_get_power_spectrogram_db(const STFTResult *result);


void stft_free_result(STFTResult *result);
void stft_free_2d_array(float **array, int rows);

double cpx_magnitude(kiss_fft_cpx c);
double cpx_phase(kiss_fft_cpx c);
double cpx_power_db(kiss_fft_cpx c);


#ifdef __cplusplus
}
#endif

#endif // STFT_H
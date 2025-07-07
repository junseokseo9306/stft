#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../src/kiss_fft.h"

int main() {
    float fs = 125.0;
    float duration = 2.0;
    int N = (int)(fs * duration);
    
    printf("Sample rate: %.1f Hz\n", fs);
    printf("Duration: %.1f seconds\n", duration);
    printf("Number of samples: %d\n", N);
    
    kiss_fft_cfg cfg = kiss_fft_alloc(N, 0, NULL, NULL);
    kiss_fft_cpx *in = (kiss_fft_cpx*)malloc(N * sizeof(kiss_fft_cpx));
    kiss_fft_cpx *out = (kiss_fft_cpx*)malloc(N * sizeof(kiss_fft_cpx));
    
    for(int i = 0; i < N; i++) {
        float t = (float)i / fs;
        in[i].r = sin(2 * M_PI * 10 * t) + 0.5 * sin(2 * M_PI * 20 * t);
        in[i].i = 0;
    }
    
    kiss_fft(cfg, in, out);

    printf("FFT completed. First few output values:\n");
    for(int i = 0; i < 5; i++) {
        printf("out[%d] = %f + %fi\n", i, out[i].r, out[i].i);
    }

    free(in);
    free(out);
    kiss_fft_free(cfg);
    return 0;
}
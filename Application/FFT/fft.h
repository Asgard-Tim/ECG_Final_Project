#pragma once
#include "stdlib.h"
#include "stdint.h"
#include "arm_math.h"
#include "CONFIGE.h"

#define FFT_LENGTH 1024 // 设定FFT的点数

typedef struct {
    uint8_t is_ready;
    float *input_pointer;
    float input[FFT_LENGTH];
    float fft_input[FFT_LENGTH * 2];
    float output[FFT_LENGTH];
    arm_rfft_fast_instance_f32 rfft_instance;
    arm_cfft_instance_f32 cfft_instance;
    arm_cfft_radix4_instance_f32 cfft_radix4_instance;
} FFT_Instance;

FFT_Instance *FFT_Init(float *input_pointer);
void FFT_Task(void);
void Draw_Spectrum(void);
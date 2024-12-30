#include "fft.h"
#include "lcd.h"
#include "memory.h"
#include "key.h"
FFT_Instance *fft_instance;
extern float ecg_buffer[FFT_LENGTH]; // ECG数据缓冲区

static void FFT_Calculate(void);

FFT_Instance *FFT_Init(float *input)
{
    // Initialize the FFT module
    fft_instance = (FFT_Instance *)malloc(sizeof(FFT_Instance));
    memset(fft_instance, 0, sizeof(FFT_Instance));

    fft_instance->is_ready      = 0;
    fft_instance->input_pointer = input;

    arm_rfft_fast_init_f32(&fft_instance->rfft_instance, FFT_LENGTH);
    arm_cfft_init_f32(&fft_instance->cfft_instance, FFT_LENGTH);
    arm_cfft_radix4_init_f32(&fft_instance->cfft_radix4_instance, FFT_LENGTH, 0, 1);

    return fft_instance;
}

int count = 0;

void FFT_Task(void)
{
    // FFT Task

    if (fft_instance->is_ready == 0) {
        return;
    }
    count++;
    FFT_Calculate();
    fft_instance->is_ready = 0;
}

// FFT计算和绘制频谱图
static void FFT_Calculate(void)
{
    memcpy(fft_instance->input, fft_instance->input_pointer, FFT_LENGTH * sizeof(float));

    for (uint16_t i = 0; i < FFT_LENGTH; i++) {
        fft_instance->fft_input[i * 2]     = ecg_buffer[i];
        fft_instance->fft_input[i * 2 + 1] = 0;
    }

    // FFT计算
    arm_cfft_radix4_f32(&fft_instance->cfft_radix4_instance, fft_instance->fft_input);
    arm_cmplx_mag_f32(fft_instance->fft_input, fft_instance->output, FFT_LENGTH);
}

uint16_t test_X = 0;
uint16_t test_Y = 0;

void Draw_Spectrum(void)
{

    for (uint16_t i = 0; i < FFT_LENGTH / 2; i += 1) {
        if (fft_instance->output[i] > 150) {
            fft_instance->output[i] = 150;
        }

        if (fft_instance->output[i] < 0) {
            fft_instance->output[i] = 0;
        }

        uint16_t x = i * (LCD_WIDTH - 1) / (FFT_LENGTH / 2);
        uint16_t y = (LCD_HEIGHT - 1) - (uint16_t)(fft_instance->output[i] / 2);

        if (y < 150)
            y = 150;

        test_X = x;
        test_Y = y;

        lcd_draw_line(x, LCD_HEIGHT - 1, x, y, BLACK); // 绘制频谱图，从屏幕底部开始画线
    }
}

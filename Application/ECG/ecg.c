#include "ecg.h"
#include "ADS1292R.h"
#include "arm_math.h"
#include "lcd.h"
#include "FIR_filter.h"
#include "IIR.h"
#include "bsp_dwt.h"
#include "transfer_function.h"
#include "fft.h"
#include "key.h"

#define ECG_LENGTH FFT_LENGTH

float ecg_buffer[ECG_LENGTH];               // ECG数据缓冲区
static float squer_wave_buffer[ECG_LENGTH]; // 呼吸阻抗数据缓冲区
static uint32_t DWT_CNT_ECG        = 0;     // ECG数据计数
static float FIR_filtered_data     = 0;     // FIR滤波后的数据
static float IIR_filtered_data     = 0;     // IIR滤波后的数据
static uint16_t ecg_count          = 0;     // ECG数据计数
static uint16_t squer_wave_count   = 0;     // 呼吸阻抗数据计数
static int heart_break             = 0;     // 心跳检测
static double heart_rate           = 0;     // 心跳率
static double heart_rate_container = 0;     // 心跳率容器,记录上一次正常心跳率
static ADS1292_Instance *ads1292;
static FFT_Instance *fft;
static KEY_Instance *key0, *key1, *key2;

static float ecg_test          = 0;
static int32_t squer_wave_test = 0;
static float peak_to_peak      = 0;

static void Draw_ECG();
static void Plot_ECG(void);
static int32_t get_volt(uint32_t data);
static void ECG_process(void);
static void Plot_SquerWave(void);
static float Cal_PeakToPeak(float *samples, uint16_t sample_count);

void ECG_Init(void)
{
    // Initialize the ECG module
    // Initialize ADS1292
    FIRInit();
    ads1292 = ADS1292_Init();
    fft     = FFT_Init(ecg_buffer);

    KEY_Config_s KEY0 = {
        .gpio_config = {
            .GPIOx    = GPIOE,
            .GPIO_Pin = GPIO_PIN_4,
        },
    };

    key0 = KEYRegister(&KEY0);
}

static uint8_t display_mode = 0;
#define KEY_PRESS 0

void ECG_Task(void)
{
    key0->state = GPIORead(key0->gpio);
    if (key0->state == KEY_PRESS) {
        display_mode = (display_mode == 0) ? 1 : 0;
        while (key0->state == KEY_PRESS) {
            key0->state = GPIORead(key0->gpio);
        }
    }

    if (display_mode == 0) {
        Plot_ECG();
    } else {
        Plot_SquerWave();
    }
}

static void Draw_ECG()
{
    drawCurve(FIR_filtered_data, 130, LCD_HEIGHT, LCD_WIDTH); // 画波形
    lcd_show_string(10, 10, 240, 16, 16, "BPM: ", RED);
    lcd_show_num(50, 10, (uint32_t)heart_rate, 3, 16, RED);
    lcd_show_string(10, 50, 240, 16, 16, "AMP: ", RED);
    lcd_show_num(40, 50, (uint32_t)(Cal_PeakToPeak(ecg_buffer, ECG_LENGTH) * 100), 5, 16, RED);
    lcd_show_string(10, 250, 240, 16, 16, "Xu JingHua ", BLUE);
}

static void Plot_ECG(void)
{
    // 模拟数据采集并绘制心电图
    if (ads1292->is_collecting == 0) {
        return;
    }
    // 读取ADS1292的原始数据到结构体的缓冲区
    ADS1292_Read_Data(ads1292->raw_data);
    // 解析数据
    ECG_process();

    // FIR 滤波
    IIR_filtered_data = IIR_Filter(ads1292->ecg_data);
    FIR_filtered_data = FIR_Filter(IIR_filtered_data);

    // 心跳检测
    heart_break = heartbeat_check(FIR_filtered_data);

    if (heart_break) {
        // 心跳检测成功
        // 计算心跳率
        heart_rate = 60.0 / DWT_GetDeltaT(&DWT_CNT_ECG);
        if (heart_rate > 30 && heart_rate < 200) {
            heart_rate_container = heart_rate;
        }
    } else {
        // 心跳检测失败
        heart_rate = heart_rate_container;
    }

    // 心电图绘制
    if (ecg_count < ECG_LENGTH) {
        lcd_show_string(130, 160, 240, 24, 24, "Frequency", BLUE);
        lcd_show_string(130, 20, 240, 24, 24, "Signal", BLUE);
        ecg_buffer[ecg_count] = FIR_filtered_data;
        ecg_count++;
    } else {
        fft->is_ready = 1;
        ecg_count     = 0;
    }
    Draw_Spectrum();
    Draw_ECG();
    ads1292->is_collecting = 0;
}

static void Plot_SquerWave(void)
{
    // 绘制呼吸阻抗波形
    // 画波形
    // 模拟数据采集并绘制心电图
    if (ads1292->is_collecting == 0) {
        return;
    }
    // 读取ADS1292的原始数据到结构体的缓冲区
    ADS1292_Read_Data(ads1292->raw_data);
    // 解析数据
    ECG_process();

    if (squer_wave_count < ECG_LENGTH) {
        squer_wave_buffer[squer_wave_count] = (ads1292->respirat_impedance);
        squer_wave_count++;
    } else {
        peak_to_peak     = Cal_PeakToPeak(squer_wave_buffer, ECG_LENGTH);
        squer_wave_count = 0;
    }

    drawCurve(ads1292->respirat_impedance, 50, LCD_HEIGHT, LCD_WIDTH); // 画波形
    lcd_show_string(10, 10, 240, 16, 16, "Peak To Peak: ", RED);
    lcd_show_num(120, 10, (uint32_t)(peak_to_peak * 100), 5, 16, RED);
    lcd_show_string(10, 50, 240, 16, 16, "Amp: ", RED);
    lcd_show_num(40, 50, (uint32_t)(peak_to_peak * 100 / 2), 5, 16, RED);
    lcd_show_string(10, 250, 240, 16, 16, "Xu JingHua ", BLUE);
    lcd_show_string(100, 80, 240, 24, 24, "Test Signal", BLUE);

    ads1292->is_collecting = 0;
}

static int32_t get_volt(uint32_t data)
{
    int32_t volt = 0;
    if (data & 0x800000) {
        volt = data | 0xFF000000;
    } else {
        volt = data;
    }
    return volt;
}

static void ECG_process(void)
{
    // 解析状态字节
    ads1292->status = ads1292->raw_data[0];

    // 检测导联状态
    ads1292->lead_left  = !(ads1292->status & 0x04); // 左导联状态
    ads1292->lead_right = !(ads1292->status & 0x02); // 右导联状态
    ads1292->ecg_active = ads1292->lead_left && ads1292->lead_right;

    // 解析呼吸阻抗和ECG数据
    int ecg_data = (ads1292->raw_data[3] << 16) | (ads1292->raw_data[4] << 8) | ads1292->raw_data[5];
    int respirat = (ads1292->raw_data[6] << 16) | (ads1292->raw_data[7] << 8) | ads1292->raw_data[8];

    // 转换24位有符号数为32位
    // if (respirat & 0x800000) respirat |= 0xFF000000; // 符号扩展
    // if (ecg_data & 0x800000) ecg_data |= 0xFF000000; // 符号扩展

    // 更新解析后的数据
    respirat                    = get_volt(respirat);
    ecg_data                    = get_volt(ecg_data);
    ads1292->ecg_data           = ecg_data * 2.42f / 32767.0f;
    ads1292->respirat_impedance = respirat * 2.42f / 32767.0f;

    ecg_test        = ads1292->ecg_data;
    squer_wave_test = ads1292->respirat_impedance;
}

static float Cal_PeakToPeak(float *samples, uint16_t sample_count)
{
    uint16_t i;
    float max = samples[0];
    float min = samples[0];
    for (i = 1; i < sample_count; i++) {
        if (samples[i] > max) {
            max = samples[i];
        }
        if (samples[i] < min) {
            min = samples[i];
        }
    }
    float peak_to_peak = (max - min);
    return peak_to_peak;
}

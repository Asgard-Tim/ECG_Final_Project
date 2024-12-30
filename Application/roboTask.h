#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "ecg.h"
#include "fft.h"

osThreadId_t ECG_TaskHandle;
osThreadId_t FFT_TaskHandle;

const osThreadAttr_t ECG_Task_attributes = {
    .name       = "ECG_Task",
    .stack_size = 128 * 8,
    .priority   = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t FFT_Task_attributes = {
    .name       = "FFT_Task",
    .stack_size = 128 * 8,
    .priority   = (osPriority_t)osPriorityNormal,
};

void StartECG_Task(void *argument);
void StartFFT_Task(void *argument);

void OS_Task_Init(void)
{
    ECG_TaskHandle = osThreadNew(StartECG_Task, NULL, &ECG_Task_attributes);
    FFT_TaskHandle = osThreadNew(StartFFT_Task, NULL, &FFT_Task_attributes);
}

__attribute__((noreturn)) void StartECG_Task(void *argument)
{
    ECG_Init();
    for (;;) {
        ECG_Task();
        osDelay(1);
    }
}

__attribute__((noreturn)) void StartFFT_Task(void *argument)
{
    for (;;) {
        FFT_Task();
        osDelay(10);
    }
}
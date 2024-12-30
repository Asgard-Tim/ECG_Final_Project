#include "IIR.h"
#include "main.h"
#include "stdio.h"

// IIR滤波器相关参数
static float a            = 0.991;
static float last_input     = 0;
static float last_output    = 0;
static float cur_input = 0;
static float cur_output     = 0;

float IIR_Filter(float rawData)
{
    float IIR_Result;
    cur_input  = rawData;
    cur_output = cur_input - last_input + a * last_output;
    IIR_Result = (float)cur_output;
    // printf("%d\n",IIR_Result);
    last_output = cur_output;
    last_input  = cur_input;
    return IIR_Result;
}
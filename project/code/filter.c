/*********************************************************************************************************************
* 文件名称: filter.c
* 功能说明: 通用滤波算法库 - 支持多种滤波算法
* 作者: 参考代码移植
* 日期: 2025-11-02
* 说明: 包含低通、滑动平均、中值、限幅、互补、巴特沃斯、卡尔曼等滤波算法
********************************************************************************************************************/

#include "filter.h"
#include <string.h>
#include <math.h>

//====================================================一阶低通滤波====================================================
void LowPassFilter_Init(LowPassFilter_t *filter, float alpha)
{
    filter->alpha = alpha;
    filter->last_output = 0.0f;
}

float LowPassFilter_Update(LowPassFilter_t *filter, float input)
{
    filter->last_output = filter->alpha * input + (1.0f - filter->alpha) * filter->last_output;
    return filter->last_output;
}

//====================================================滑动平均滤波====================================================
void MovingAvgFilter_Init(MovingAvgFilter_t *filter, float *buffer, uint8 size)
{
    filter->buffer = buffer;
    filter->size = size;
    filter->index = 0;
    filter->sum = 0.0f;
    memset(buffer, 0, size * sizeof(float));
}

float MovingAvgFilter_Update(MovingAvgFilter_t *filter, float input)
{
    filter->sum -= filter->buffer[filter->index];
    filter->buffer[filter->index] = input;
    filter->sum += input;
    filter->index = (filter->index + 1) % filter->size;
    return filter->sum / filter->size;
}

//====================================================中值滤波====================================================
static void bubble_sort(float *arr, uint8 n)
{
    for (uint8 i = 0; i < n - 1; i++) {
        for (uint8 j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                float temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void MedianFilter_Init(MedianFilter_t *filter, float *buffer, uint8 size)
{
    filter->buffer = buffer;
    filter->size = size;
    filter->index = 0;
    memset(buffer, 0, size * sizeof(float));
}

float MedianFilter_Update(MedianFilter_t *filter, float input)
{
    filter->buffer[filter->index] = input;
    filter->index = (filter->index + 1) % filter->size;
    
    float sorted[20];
    memcpy(sorted, filter->buffer, filter->size * sizeof(float));
    bubble_sort(sorted, filter->size);
    
    return sorted[filter->size / 2];
}

//====================================================限幅滤波====================================================
void LimitFilter_Init(LimitFilter_t *filter, float init_value, float max_change)
{
    filter->last_value = init_value;
    filter->max_change = max_change;
}

float LimitFilter_Update(LimitFilter_t *filter, float input)
{
    float change = input - filter->last_value;
    
    if (change > filter->max_change) {
        filter->last_value += filter->max_change;
    } else if (change < -filter->max_change) {
        filter->last_value -= filter->max_change;
    } else {
        filter->last_value = input;
    }
    
    return filter->last_value;
}

//====================================================一阶互补滤波====================================================
float ComplementaryFilter_Update(ComplementaryFilter_t *filter, float high_freq, float low_freq)
{
    return filter->alpha * high_freq + (1.0f - filter->alpha) * low_freq;
}

//====================================================二阶巴特沃斯低通滤波====================================================
void ButterworthFilter_Init(ButterworthFilter_t *filter, float cutoff_freq, float sample_freq)
{
    // 计算归一化截止频率
    float ff = cutoff_freq / sample_freq;
    float ita = 1.0f / tanf(3.14159265f * ff);
    float q = 1.414213562f;  // sqrt(2)
    
    // 计算滤波器系数
    filter->b0 = 1.0f / (1.0f + q * ita + ita * ita);
    filter->b1 = 2.0f * filter->b0;
    filter->b2 = filter->b0;
    filter->a1 = 2.0f * (ita * ita - 1.0f) * filter->b0;
    filter->a2 = -(1.0f - q * ita + ita * ita) * filter->b0;
    
    // 初始化历史数据
    filter->x1 = 0.0f;
    filter->x2 = 0.0f;
    filter->y1 = 0.0f;
    filter->y2 = 0.0f;
}

float ButterworthFilter_Update(ButterworthFilter_t *filter, float input)
{
    // 二阶差分方程: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    float output = filter->b0 * input + filter->b1 * filter->x1 + filter->b2 * filter->x2
                   - filter->a1 * filter->y1 - filter->a2 * filter->y2;
    
    // 更新历史数据
    filter->x2 = filter->x1;
    filter->x1 = input;
    filter->y2 = filter->y1;
    filter->y1 = output;
    
    return output;
}

//====================================================一阶卡尔曼滤波====================================================
void KalmanFilter_Init(KalmanFilter_t *filter, float Q, float R, float init_value)
{
    filter->Q = Q;
    filter->R = R;
    filter->P = 1.0f;
    filter->x = init_value;
}

float KalmanFilter_Update(KalmanFilter_t *filter, float measurement)
{
    // 预测
    float x_predict = filter->x;
    float P_predict = filter->P + filter->Q;
    
    // 更新
    filter->K = P_predict / (P_predict + filter->R);
    filter->x = x_predict + filter->K * (measurement - x_predict);
    filter->P = (1.0f - filter->K) * P_predict;
    
    return filter->x;
}

//====================================================组合滤波====================================================
void MedianLowPassFilter_Init(MedianLowPassFilter_t *filter, float *buffer, uint8 size, float alpha)
{
    MedianFilter_Init(&filter->median, buffer, size);
    LowPassFilter_Init(&filter->lowpass, alpha);
}

float MedianLowPassFilter_Update(MedianLowPassFilter_t *filter, float input)
{
    float median_out = MedianFilter_Update(&filter->median, input);
    return LowPassFilter_Update(&filter->lowpass, median_out);
}




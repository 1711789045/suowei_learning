/*********************************************************************************************************************
* 文件名称: encoder.c
* 功能说明: 编码器模块 - 方向编码器读取与中值+卡尔曼串联滤波
* 作者: Claude Code
* 日期: 2025-11-02
* 更新: 中值滤波去除周期脉冲 + 卡尔曼滤波平滑数据
********************************************************************************************************************/

#include "encoder.h"

// 左右编码器中值滤波器（去除周期脉冲，外部可访问）
MedianFilter_t encoder_left_median;
MedianFilter_t encoder_right_median;

// 左右编码器卡尔曼滤波器（平滑数据，外部可访问）
KalmanFilter_t encoder_left_kalman;
KalmanFilter_t encoder_right_kalman;

// 中值滤波器缓冲区（每个滤波器需要独立的缓冲区）
static float encoder_left_median_buffer[ENCODER_MEDIAN_SIZE];
static float encoder_right_median_buffer[ENCODER_MEDIAN_SIZE];

/*********************************************************************************************************************
* 函数名称: encoder_init
* 功能说明: 初始化编码器和中值+卡尔曼串联滤波器
* 参数说明: 无
* 返回值:   无
********************************************************************************************************************/
void encoder_init(void)
{
    // 初始化左编码器 (方向编码器模式)
    encoder_dir_init(ENCODER_LEFT, ENCODER_LEFT_A, ENCODER_LEFT_B);

    // 初始化右编码器 (方向编码器模式)
    encoder_dir_init(ENCODER_RIGHT, ENCODER_RIGHT_A, ENCODER_RIGHT_B);

    // 初始化左编码器中值滤波器（第一级：去除脉冲）
    // 参数说明: buffer=缓冲区, size=3 (窗口大小)
    MedianFilter_Init(&encoder_left_median, encoder_left_median_buffer, ENCODER_MEDIAN_SIZE);

    // 初始化左编码器卡尔曼滤波器（第二级：平滑数据）
    // 参数说明: Q=0.01 (过程噪声), R=2.0 (测量噪声), init_value=0
    KalmanFilter_Init(&encoder_left_kalman, ENCODER_KALMAN_Q, ENCODER_KALMAN_R, 0.0f);

    // 初始化右编码器中值滤波器（第一级：去除脉冲）
    MedianFilter_Init(&encoder_right_median, encoder_right_median_buffer, ENCODER_MEDIAN_SIZE);

    // 初始化右编码器卡尔曼滤波器（第二级：平滑数据）
    KalmanFilter_Init(&encoder_right_kalman, ENCODER_KALMAN_Q, ENCODER_KALMAN_R, 0.0f);

    printf("[ENCODER] Init OK - Left:TC_CH09(P05_0/1) Right:TC_CH07(P02_0/1) [Median+Kalman Filter]\r\n");
}

/*********************************************************************************************************************
* 函数名称: encoder_reset_filters
* 功能说明: 重置编码器滤波器状态
* 参数说明: 无
* 返回值:   无
* 备注:     在motor_reset()中调用，清除滤波器历史状态
********************************************************************************************************************/
void encoder_reset_filters(void)
{
    // 重新初始化中值滤波器（会清空buffer和历史状态）
    MedianFilter_Init(&encoder_left_median, encoder_left_median_buffer, ENCODER_MEDIAN_SIZE);
    MedianFilter_Init(&encoder_right_median, encoder_right_median_buffer, ENCODER_MEDIAN_SIZE);

    // 重新初始化卡尔曼滤波器（会清空历史状态）
    KalmanFilter_Init(&encoder_left_kalman, ENCODER_KALMAN_Q, ENCODER_KALMAN_R, 0.0f);
    KalmanFilter_Init(&encoder_right_kalman, ENCODER_KALMAN_Q, ENCODER_KALMAN_R, 0.0f);
}

/*********************************************************************************************************************
* 函数名称: encoder_get_left
* 功能说明: 获取左编码器增量值(中值+卡尔曼串联滤波后)
* 参数说明: 无
* 返回值:   int16 - 滤波后的增量值(带符号)
* 备注:     每次调用会清空硬件计数器,应在固定周期(如5ms)调用
*           ⚠️ 左编码器方向反了，需要取反（正转应读正值）
*           串联滤波: 中值滤波去除周期脉冲 → 卡尔曼滤波平滑数据
********************************************************************************************************************/
int16 encoder_get_left(void)
{
    // 读取硬件计数器
    int16 current = encoder_get_count(ENCODER_LEFT);

    // 清空计数器(为下次读取做准备)
    encoder_clear_count(ENCODER_LEFT);

    // 第一级：中值滤波（去除周期性脉冲噪声）
    float median_filtered = MedianFilter_Update(&encoder_left_median, (float)current);

    // 第二级：卡尔曼滤波（平滑数据）
    float kalman_filtered = KalmanFilter_Update(&encoder_left_kalman, median_filtered);

    // ⚠️ 取反修正方向（正转时读到负值，需要反向）
    return (int16)kalman_filtered;
}

/*********************************************************************************************************************
* 函数名称: encoder_get_right
* 功能说明: 获取右编码器增量值(中值+卡尔曼串联滤波后)
* 参数说明: 无
* 返回值:   int16 - 滤波后的增量值(带符号)
* 备注:     每次调用会清空硬件计数器,应在固定周期(如5ms)调用
*           ⚠️ 右编码器方向反了，需要取反（正转应读正值）
*           串联滤波: 中值滤波去除周期脉冲 → 卡尔曼滤波平滑数据
********************************************************************************************************************/
int16 encoder_get_right(void)
{
    // 读取硬件计数器
    int16 current = encoder_get_count(ENCODER_RIGHT);

    // 清空计数器(为下次读取做准备)
    encoder_clear_count(ENCODER_RIGHT);

    // 第一级：中值滤波（去除周期性脉冲噪声）
    float median_filtered = MedianFilter_Update(&encoder_right_median, (float)current);

    // 第二级：卡尔曼滤波（平滑数据）
    float kalman_filtered = KalmanFilter_Update(&encoder_right_kalman, median_filtered);

    // ⚠️ 取反修正方向（正转时读到负值，需要反向）
    return -(int16)kalman_filtered;
}

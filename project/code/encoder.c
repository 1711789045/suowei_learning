/*********************************************************************************************************************
* 文件名称: encoder.c
* 功能说明: 编码器模块 - 方向编码器读取与中值+低通组合滤波
* 作者: Claude Code
* 日期: 2025-11-02
* 更新: 从卡尔曼滤波改为中值+低通滤波 - 去除脉冲噪声+平滑高频噪声
********************************************************************************************************************/

#include "encoder.h"

// 左右编码器中值+低通滤波器（外部可访问，用于重置）
MedianLowPassFilter_t encoder_left_filter;
MedianLowPassFilter_t encoder_right_filter;

// 中值滤波器缓冲区（每个滤波器需要独立的缓冲区）
static float encoder_left_buffer[ENCODER_MEDIAN_SIZE];
static float encoder_right_buffer[ENCODER_MEDIAN_SIZE];

/*********************************************************************************************************************
* 函数名称: encoder_init
* 功能说明: 初始化编码器和中值+低通滤波器
* 参数说明: 无
* 返回值:   无
********************************************************************************************************************/
void encoder_init(void)
{
    // 初始化左编码器 (方向编码器模式)
    encoder_dir_init(ENCODER_LEFT, ENCODER_LEFT_A, ENCODER_LEFT_B);

    // 初始化右编码器 (方向编码器模式)
    encoder_dir_init(ENCODER_RIGHT, ENCODER_RIGHT_A, ENCODER_RIGHT_B);

    // 初始化左编码器中值+低通滤波器
    // 参数说明: buffer=缓冲区, size=3 (窗口大小), alpha=0.3 (低通系数)
    MedianLowPassFilter_Init(&encoder_left_filter, encoder_left_buffer,
                              ENCODER_MEDIAN_SIZE, ENCODER_LOWPASS_ALPHA);

    // 初始化右编码器中值+低通滤波器
    MedianLowPassFilter_Init(&encoder_right_filter, encoder_right_buffer,
                              ENCODER_MEDIAN_SIZE, ENCODER_LOWPASS_ALPHA);

    printf("[ENCODER] Init OK - Left:TC_CH09(P05_0/1) Right:TC_CH07(P02_0/1) [Median+LowPass Filter]\r\n");
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
    // 重新初始化滤波器（会清空buffer和历史状态）
    MedianLowPassFilter_Init(&encoder_left_filter, encoder_left_buffer,
                              ENCODER_MEDIAN_SIZE, ENCODER_LOWPASS_ALPHA);
    MedianLowPassFilter_Init(&encoder_right_filter, encoder_right_buffer,
                              ENCODER_MEDIAN_SIZE, ENCODER_LOWPASS_ALPHA);
}

/*********************************************************************************************************************
* 函数名称: encoder_get_left
* 功能说明: 获取左编码器增量值(中值+低通滤波后)
* 参数说明: 无
* 返回值:   int16 - 滤波后的增量值(带符号)
* 备注:     每次调用会清空硬件计数器,应在固定周期(如5ms)调用
*           ⚠️ 左编码器方向反了，需要取反（正转应读正值）
*           中值+低通滤波优势: 去除脉冲噪声+平滑高频噪声，适合PID微分项
********************************************************************************************************************/
int16 encoder_get_left(void)
{
    // 读取硬件计数器
    int16 current = encoder_get_count(ENCODER_LEFT);

    // 清空计数器(为下次读取做准备)
    encoder_clear_count(ENCODER_LEFT);

    // 中值+低通组合滤波更新
    float filtered = MedianLowPassFilter_Update(&encoder_left_filter, (float)current);

    // ⚠️ 取反修正方向（正转时读到负值，需要反向）
    return (int16)filtered;
}

/*********************************************************************************************************************
* 函数名称: encoder_get_right
* 功能说明: 获取右编码器增量值(中值+低通滤波后)
* 参数说明: 无
* 返回值:   int16 - 滤波后的增量值(带符号)
* 备注:     每次调用会清空硬件计数器,应在固定周期(如5ms)调用
*           ⚠️ 右编码器方向反了，需要取反（正转应读正值）
*           中值+低通滤波优势: 去除脉冲噪声+平滑高频噪声，适合PID微分项
********************************************************************************************************************/
int16 encoder_get_right(void)
{
    // 读取硬件计数器
    int16 current = encoder_get_count(ENCODER_RIGHT);

    // 清空计数器(为下次读取做准备)
    encoder_clear_count(ENCODER_RIGHT);

    // 中值+低通滤波更新
    float filtered = MedianLowPassFilter_Update(&encoder_right_filter, (float)current);

    // ⚠️ 取反修正方向（正转时读到负值，需要反向）
    return -(int16)filtered;
}

/*********************************************************************************************************************
* 文件名称: encoder.c
* 功能说明: 编码器模块 - 方向编码器读取与卡尔曼滤波
* 作者: Claude Code
* 日期: 2025-11-02
* 更新: 从低通滤波改为卡尔曼滤波 - 更优的编码器速度估计
********************************************************************************************************************/

#include "encoder.h"

// 左右编码器卡尔曼滤波器
static KalmanFilter_t encoder_left_kalman;
static KalmanFilter_t encoder_right_kalman;

/*********************************************************************************************************************
* 函数名称: encoder_init
* 功能说明: 初始化编码器和卡尔曼滤波器
* 参数说明: 无
* 返回值:   无
********************************************************************************************************************/
void encoder_init(void)
{
    // 初始化左编码器 (方向编码器模式)
    encoder_dir_init(ENCODER_LEFT, ENCODER_LEFT_A, ENCODER_LEFT_B);

    // 初始化右编码器 (方向编码器模式)
    encoder_dir_init(ENCODER_RIGHT, ENCODER_RIGHT_A, ENCODER_RIGHT_B);

    // 初始化左编码器卡尔曼滤波器
    // 参数说明: Q=0.01 (过程噪声), R=2.0 (测量噪声), init_value=0
    KalmanFilter_Init(&encoder_left_kalman, ENCODER_KALMAN_Q, ENCODER_KALMAN_R, 0.0f);

    // 初始化右编码器卡尔曼滤波器
    KalmanFilter_Init(&encoder_right_kalman, ENCODER_KALMAN_Q, ENCODER_KALMAN_R, 0.0f);

    printf("[ENCODER] Init OK - Left:TC_CH09(P05_0/1) Right:TC_CH07(P02_0/1) [Kalman Filter]\r\n");
}

/*********************************************************************************************************************
* 函数名称: encoder_get_left
* 功能说明: 获取左编码器增量值(卡尔曼滤波后)
* 参数说明: 无
* 返回值:   int16 - 滤波后的增量值(带符号)
* 备注:     每次调用会清空硬件计数器,应在固定周期(如5ms)调用
*           ⚠️ 左编码器方向反了，需要取反（正转应读正值）
*           卡尔曼滤波优势: 最优估计、自适应调节、更好的噪声抑制
********************************************************************************************************************/
int16 encoder_get_left(void)
{
    // 读取硬件计数器
    int16 current = encoder_get_count(ENCODER_LEFT);

    // 清空计数器(为下次读取做准备)
    encoder_clear_count(ENCODER_LEFT);

    // 卡尔曼滤波更新
    float filtered = KalmanFilter_Update(&encoder_left_kalman, (float)current);

    // ⚠️ 取反修正方向（正转时读到负值，需要反向）
    return (int16)filtered;
}

/*********************************************************************************************************************
* 函数名称: encoder_get_right
* 功能说明: 获取右编码器增量值(卡尔曼滤波后)
* 参数说明: 无
* 返回值:   int16 - 滤波后的增量值(带符号)
* 备注:     每次调用会清空硬件计数器,应在固定周期(如5ms)调用
*           ⚠️ 右编码器方向反了，需要取反（正转应读正值）
*           卡尔曼滤波优势: 最优估计、自适应调节、更好的噪声抑制
********************************************************************************************************************/
int16 encoder_get_right(void)
{
    // 读取硬件计数器
    int16 current = encoder_get_count(ENCODER_RIGHT);

    // 清空计数器(为下次读取做准备)
    encoder_clear_count(ENCODER_RIGHT);

    // 卡尔曼滤波更新
    float filtered = KalmanFilter_Update(&encoder_right_kalman, (float)current);

    // ⚠️ 取反修正方向（正转时读到负值，需要反向）
    return -(int16)filtered;
}

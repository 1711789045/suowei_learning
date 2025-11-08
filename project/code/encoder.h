/*********************************************************************************************************************
* 文件名称: encoder.h
* 功能说明: 编码器模块 - 方向编码器读取与中值+卡尔曼串联滤波
* 作者: Claude Code
* 日期: 2025-11-02
* 更新: 中值滤波去除周期脉冲 + 卡尔曼滤波平滑数据
********************************************************************************************************************/

#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "zf_common_headfile.h"
#include "filter.h"

// 编码器接口定义
#define ENCODER_LEFT                (TC_CH09_ENCODER)                      // 左编码器
#define ENCODER_LEFT_A              (TC_CH09_ENCODER_CH1_P05_0)            // 左编码器A相
#define ENCODER_LEFT_B              (TC_CH09_ENCODER_CH2_P05_1)            // 左编码器B相

#define ENCODER_RIGHT               (TC_CH07_ENCODER)                      // 右编码器
#define ENCODER_RIGHT_A             (TC_CH07_ENCODER_CH1_P02_0)            // 右编码器A相
#define ENCODER_RIGHT_B             (TC_CH07_ENCODER_CH2_P02_1)            // 右编码器B相

// 中值滤波参数 (去除周期性脉冲噪声)
// MEDIAN_SIZE: 中值滤波窗口大小 (推荐3，去除偶发脉冲)
#define ENCODER_MEDIAN_SIZE         (3)                                    // 中值滤波窗口

// 卡尔曼滤波参数 (编码器速度平滑)
// Q: 过程噪声协方差 (越大越信任测量值)
// R: 测量噪声协方差 (越大越信任预测值)
// 参考快速选择指南推荐: Q=0.01, R=2.0
#define ENCODER_KALMAN_Q            (0.01f)                                // 过程噪声
#define ENCODER_KALMAN_R            (2.0f)                                 // 测量噪声

// 中值滤波器状态（外部可访问，用于重置）
extern MedianFilter_t encoder_left_median;
extern MedianFilter_t encoder_right_median;

// 卡尔曼滤波器状态（外部可访问，用于重置）
extern KalmanFilter_t encoder_left_kalman;
extern KalmanFilter_t encoder_right_kalman;

// API函数声明
void encoder_init(void);                                                   // 初始化编码器
void encoder_reset_filters(void);                                          // 重置编码器滤波器状态
int16 encoder_get_left(void);                                              // 获取左编码器增量(中值+卡尔曼串联滤波后)
int16 encoder_get_right(void);                                             // 获取右编码器增量(中值+卡尔曼串联滤波后)

#endif // _ENCODER_H_

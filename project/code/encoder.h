/*********************************************************************************************************************
* 文件名称: encoder.h
* 功能说明: 编码器模块 - 方向编码器读取与卡尔曼滤波
* 作者: Claude Code
* 日期: 2025-11-02
* 更新: 从低通滤波改为卡尔曼滤波 - 更优的编码器速度估计
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

// 卡尔曼滤波参数 (编码器速度专用参数)
// Q: 过程噪声协方差 (越大越信任测量值)
// R: 测量噪声协方差 (越大越信任预测值)
// 参考快速选择指南推荐: Q=0.01, R=2.0
#define ENCODER_KALMAN_Q            (0.01f)                                // 过程噪声
#define ENCODER_KALMAN_R            (2.0f)                                 // 测量噪声

// API函数声明
void encoder_init(void);                                                   // 初始化编码器
int16 encoder_get_left(void);                                              // 获取左编码器增量(卡尔曼滤波后)
int16 encoder_get_right(void);                                             // 获取右编码器增量(卡尔曼滤波后)

#endif // _ENCODER_H_

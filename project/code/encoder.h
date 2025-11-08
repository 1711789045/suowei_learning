/*********************************************************************************************************************
* 文件名称: encoder.h
* 功能说明: 编码器模块 - 方向编码器读取与中值+低通组合滤波
* 作者: Claude Code
* 日期: 2025-11-02
* 更新: 从卡尔曼滤波改为中值+低通滤波 - 去除脉冲噪声+平滑高频噪声
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

// 中值+低通滤波参数 (编码器速度专用参数)
// MEDIAN_SIZE: 中值滤波窗口大小 (去除脉冲噪声)
// LOWPASS_ALPHA: 低通滤波系数 (平滑高频噪声, 0.0-1.0, 越小越平滑)
// 推荐参数: SIZE=3, ALPHA=0.3 (约15ms延迟，去噪效果好)
#define ENCODER_MEDIAN_SIZE         (3)                                    // 中值滤波窗口
#define ENCODER_LOWPASS_ALPHA       (0.3f)                                 // 低通滤波系数

// 中值+低通滤波器状态（外部可访问，用于重置）
extern MedianLowPassFilter_t encoder_left_filter;
extern MedianLowPassFilter_t encoder_right_filter;

// API函数声明
void encoder_init(void);                                                   // 初始化编码器
void encoder_reset_filters(void);                                          // 重置编码器滤波器状态
int16 encoder_get_left(void);                                              // 获取左编码器增量(中值+低通滤波后)
int16 encoder_get_right(void);                                             // 获取右编码器增量(中值+低通滤波后)

#endif // _ENCODER_H_

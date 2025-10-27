/*********************************************************************************************************************
* 文件名称: encoder.h
* 功能说明: 编码器模块 - 方向编码器读取与滤波
* 作者: Claude Code
* 日期: 2025-10-27
********************************************************************************************************************/

#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "zf_common_headfile.h"

// 编码器接口定义
#define ENCODER_LEFT                (TC_CH09_ENCODER)                      // 左编码器
#define ENCODER_LEFT_A              (TC_CH09_ENCODER_CH1_P05_0)            // 左编码器A相
#define ENCODER_LEFT_B              (TC_CH09_ENCODER_CH2_P05_1)            // 左编码器B相

#define ENCODER_RIGHT               (TC_CH07_ENCODER)                      // 右编码器
#define ENCODER_RIGHT_A             (TC_CH07_ENCODER_CH1_P02_0)            // 右编码器A相
#define ENCODER_RIGHT_B             (TC_CH07_ENCODER_CH2_P02_1)            // 右编码器B相

// 低通滤波系数 (filtered = NEW_COEF × new + OLD_COEF × old)
#define ENCODER_FILTER_NEW_COEF     (0.9f)
#define ENCODER_FILTER_OLD_COEF     (0.1f)

// API函数声明
void encoder_init(void);                                                   // 初始化编码器
int16 encoder_get_left(void);                                              // 获取左编码器增量(滤波后)
int16 encoder_get_right(void);                                             // 获取右编码器增量(滤波后)

#endif // _ENCODER_H_

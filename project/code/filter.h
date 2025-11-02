/*********************************************************************************************************************
* 文件名称: filter.h
* 功能说明: 通用滤波算法库 - 支持多种滤波算法
* 作者: 参考代码移植
* 日期: 2025-11-02
* 说明: 包含低通、滑动平均、中值、限幅、互补、巴特沃斯、卡尔曼等滤波算法
********************************************************************************************************************/

#ifndef _FILTER_H_
#define _FILTER_H_

#include "zf_common_typedef.h"

// 一阶低通滤波器
typedef struct {
    float alpha;        // 滤波系数 (0-1)
    float last_output;  // 上次输出值
} LowPassFilter_t;

// 滑动平均滤波器
typedef struct {
    float *buffer;      // 数据缓冲区
    uint8 size;         // 窗口大小
    uint8 index;        // 当前索引
    float sum;          // 数据和
} MovingAvgFilter_t;

// 中值滤波器
typedef struct {
    float *buffer;      // 数据缓冲区
    uint8 size;         // 窗口大小
    uint8 index;        // 当前索引
} MedianFilter_t;

// 限幅滤波器
typedef struct {
    float last_value;   // 上次值
    float max_change;   // 最大变化量
} LimitFilter_t;

// 一阶互补滤波器
typedef struct {
    float alpha;        // 滤波系数 (0-1)
} ComplementaryFilter_t;

// 二阶巴特沃斯低通滤波器
typedef struct {
    float a1, a2;       // 输出系数
    float b0, b1, b2;   // 输入系数
    float x1, x2;       // 输入历史
    float y1, y2;       // 输出历史
} ButterworthFilter_t;

// 一阶卡尔曼滤波器
typedef struct {
    float Q;            // 过程噪声协方差
    float R;            // 测量噪声协方差
    float P;            // 估计误差协方差
    float K;            // 卡尔曼增益
    float x;            // 状态估计值
} KalmanFilter_t;

//====================================================一阶低通滤波====================================================
// 特点: 简单高效, 平滑数据, 有延迟
// 用途: 陀螺仪/加速度计等传感器数据滤波
// alpha越小越平滑但延迟越大, 推荐0.1-0.3
void LowPassFilter_Init(LowPassFilter_t *filter, float alpha);
float LowPassFilter_Update(LowPassFilter_t *filter, float input);

//====================================================滑动平均滤波====================================================
// 特点: 平滑效果好, 消除随机噪声
// 用途: 编码器速度/电压等波动数据
// 窗口越大越平滑但延迟越大, 推荐5-10
void MovingAvgFilter_Init(MovingAvgFilter_t *filter, float *buffer, uint8 size);
float MovingAvgFilter_Update(MovingAvgFilter_t *filter, float input);

//====================================================中值滤波====================================================
// 特点: 去除脉冲干扰, 保留边缘
// 用途: 去除偶发的异常值/毛刺
// 窗口建议奇数, 推荐3-7
void MedianFilter_Init(MedianFilter_t *filter, float *buffer, uint8 size);
float MedianFilter_Update(MedianFilter_t *filter, float input);

//====================================================限幅滤波====================================================
// 特点: 限制变化速率, 防止突变
// 用途: 防止传感器读数突变
// max_change根据实际情况设定
void LimitFilter_Init(LimitFilter_t *filter, float init_value, float max_change);
float LimitFilter_Update(LimitFilter_t *filter, float input);

//====================================================一阶互补滤波====================================================
// 特点: 融合两种传感器数据
// 用途: 陀螺仪+加速度计融合
// alpha为高频数据权重, 推荐0.95-0.98
float ComplementaryFilter_Update(ComplementaryFilter_t *filter, float high_freq, float low_freq);

//====================================================二阶巴特沃斯低通滤波====================================================
// 特点: 平滑性好, 相位延迟小, 无振铃
// 用途: 对平滑度要求高的场合
// cutoff_freq为截止频率(Hz), sample_freq为采样频率(Hz)
void ButterworthFilter_Init(ButterworthFilter_t *filter, float cutoff_freq, float sample_freq);
float ButterworthFilter_Update(ButterworthFilter_t *filter, float input);

//====================================================一阶卡尔曼滤波====================================================
// 特点: 最优估计, 自适应调节
// 用途: 融合模型预测和测量值
// Q: 过程噪声(越大越信任测量值), R: 测量噪声(越大越信任预测值)
// Q推荐0.001-0.01, R推荐0.1-1.0
void KalmanFilter_Init(KalmanFilter_t *filter, float Q, float R, float init_value);
float KalmanFilter_Update(KalmanFilter_t *filter, float measurement);

//====================================================组合滤波====================================================
// 中值+低通组合 (先去毛刺再平滑)
typedef struct {
    MedianFilter_t median;
    LowPassFilter_t lowpass;
} MedianLowPassFilter_t;

void MedianLowPassFilter_Init(MedianLowPassFilter_t *filter, float *buffer, uint8 size, float alpha);
float MedianLowPassFilter_Update(MedianLowPassFilter_t *filter, float input);

#endif


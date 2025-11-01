/*********************************************************************************************************************
* 文件名称: pid.h
* 功能说明: PID控制模块 - 增量式PID算法
* 作者: Claude Code
* 日期: 2025-10-27
********************************************************************************************************************/

#ifndef _PID_H_
#define _PID_H_

#include "zf_common_headfile.h"

// PID状态结构体(每个控制对象独立的状态)
typedef struct
{
    // 增量式PID用
    float error_last;           // 上次误差 e(k-1)
    float error_last2;          // 上上次误差 e(k-2)
    float output;               // 当前输出累积值
    
    // 位置式PID用
    float integral;             // 积分累积值
} pid_t;

// ==================== 速度环PID参数（左右轮独立）====================
extern float speed_left_kp;     // 左轮速度环比例系数
extern float speed_left_ki;     // 左轮速度环积分系数
extern float speed_left_kd;     // 左轮速度环微分系数

extern float speed_right_kp;    // 右轮速度环比例系数
extern float speed_right_ki;    // 右轮速度环积分系数
extern float speed_right_kd;    // 右轮速度环微分系数

// ==================== 方向环PID参数 ====================
extern float direction_kp;      // 方向环比例系数
extern float direction_ki;      // 方向环积分系数
extern float direction_kd;      // 方向环微分系数

// API函数声明
void pid_init(void);                                                                    // 初始化PID参数
void pid_reset(pid_t *pid);                                                             // 重置PID状态
float pid_calc_incremental(pid_t *pid, float kp, float ki, float kd, 
                           float target, float actual);                                 // 增量式PID（速度环用，支持自定义参数）
float pid_calc_position(pid_t *pid, float target, float actual);                        // 位置式PID（方向环用）

#endif // _PID_H_

/*********************************************************************************************************************
* 文件名称: pid.h
* 功能说明: PID控制模块 - 增量式PID算法
* 作者: Claude Code
* 日期: 2025-10-27
********************************************************************************************************************/

#ifndef _PID_H_
#define _PID_H_

#include "zf_common_headfile.h"

// PID状态结构体(每个控制对象独立的状态) - 增量式PID
typedef struct
{
    float error_last;           // 上次误差 e(k-1)
    float error_last2;          // 上上次误差 e(k-2)
    float output;               // 当前输出累积值
} pid_t;

// 全局PID参数(左右电机共享,通过菜单调参)
extern float motor_kp;          // 比例系数
extern float motor_ki;          // 积分系数
extern float motor_kd;          // 微分系数

// API函数声明
void pid_init(void);                                                       // 初始化PID参数
void pid_reset(pid_t *pid);                                                // 重置PID状态
float pid_calc(pid_t *pid, float target, float actual);                    // 计算PID输出

#endif // _PID_H_

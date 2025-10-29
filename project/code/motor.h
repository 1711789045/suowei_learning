/*********************************************************************************************************************
* 文件名称: motor.h
* 功能说明: 电机控制模块 - PWM驱动+编码器反馈+PID闭环
* 作者: Claude Code
* 日期: 2025-10-27
********************************************************************************************************************/

#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "zf_common_headfile.h"
#include "encoder.h"
#include "pid.h"

// 左电机PWM引脚定义(HIP4082驱动)
#define MOTOR_LEFT_A            (TCPWM_CH14_P00_2)                         // 左电机A相
#define MOTOR_LEFT_B            (TCPWM_CH13_P00_3)                         // 左电机B相

// 右电机PWM引脚定义(HIP4082驱动)
#define MOTOR_RIGHT_A           (TCPWM_CH51_P18_6)                         // 右电机A相
#define MOTOR_RIGHT_B           (TCPWM_CH50_P18_7)                         // 右电机B相

// PWM参数
#define MOTOR_PWM_FREQ          (17000)                                    // PWM频率 17kHz
#define MOTOR_PWM_MAX_DUTY      (10000)                                    // 最大占空比限制(0-10000对应0%-100%)

// VOFA+速度环调试(外部可访问)
extern uint8 motor_vofa_enable;                                            // 1=开启速度环调试, 0=关闭
extern int16 motor_basic_speed;                                            // 速度环基础速度(编码器增量目标值)

// API函数声明
void motor_init(void);                                                     // 初始化电机系统
void motor_set_target_left(int16 target);                                  // 设置左电机目标值(编码器增量)
void motor_set_target_right(int16 target);                                 // 设置右电机目标值(编码器增量)
void motor_process(void);                                                  // 电机控制周期(10ms调用一次)
void motor_vofa_send(void);                                                // 发送VOFA+调试数据

#endif // _MOTOR_H_

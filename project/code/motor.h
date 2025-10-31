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

// ==================== DRV8701E 驱动引脚定义 ====================
// DRV8701E控制方式：DIR（方向GPIO）+ PWM（速度PWM）
// DIR=HIGH + PWM → 正转
// DIR=LOW  + PWM → 反转

// 左电机引脚（电机2）
#define MOTOR_LEFT_DIR          (P18_6)                                    // 左电机方向控制（普通GPIO）
#define MOTOR_LEFT_PWM          (TCPWM_CH50_P18_7)                         // 左电机速度控制（PWM）

// 右电机引脚（电机1）
#define MOTOR_RIGHT_DIR         (P00_2)                                    // 右电机方向控制（普通GPIO）
#define MOTOR_RIGHT_PWM         (TCPWM_CH13_P00_3)                         // 右电机速度控制（PWM）

// PWM参数
#define MOTOR_PWM_FREQ          (17000)                                    // PWM频率 17kHz
#define MOTOR_PWM_MAX_DUTY      (4000)                                     // 最大占空比限制(0-10000对应0%-100%)

// ==================== 速度环参数（外部可访问）====================
extern uint8 speed_debug_enable;                                           // 速度环调试开关(1=开启, 0=关闭)
extern int16 basic_speed;                                                  // 基础速度(编码器增量目标值)

// ==================== 方向环参数（外部可访问）====================
extern uint8 direction_debug_enable;                                       // 方向环调试开关(1=开启, 0=关闭)
extern float inner_wheel_ratio;                                            // 内轮减速系数
extern float outer_wheel_ratio;                                            // 外轮加速系数

// API函数声明
void motor_init(void);                                                     // 初始化电机系统
void motor_set_target_left(int16 target);                                  // 设置左电机目标值(编码器增量)
void motor_set_target_right(int16 target);                                 // 设置右电机目标值(编码器增量)
void motor_process(void);                                                  // 电机控制周期(5ms调用一次)
void motor_vofa_send(void);                                                // 发送VOFA+调试数据

#endif // _MOTOR_H_

/*********************************************************************************************************************
* CYT2BL3 Opensourec Library 即（CYT2BL3 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是CYT2BL3 开源库的一部分
*
* CYT2BL3 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 文件名称          control
* 功能说明          发车控制模块 - 发车/停车/停车检测
* 作者              Claude Code
* 日期              2025-10-31
********************************************************************************************************************/

#ifndef _CONTROL_H_
#define _CONTROL_H_

#include "zf_common_headfile.h"

// ==================== 运行状态标志 ====================
extern uint8 car_running;           // 小车运行状态（0=停止，1=运行）
extern uint8 stop_flag;             // 停车标志位

// ==================== API函数声明 ====================

/**
 * @brief  初始化控制模块
 * @param  无
 * @return 无
 */
void control_init(void);

/**
 * @brief  发车函数（菜单调用）
 * @param  无
 * @return 无
 * @note   第一次按确认键：显示提示，启动3秒倒计时
 *         3秒内再次按确认键：倒计时3-2-1后发车
 *         3秒内未按确认键：取消发车
 */
void start_car(void);

/**
 * @brief  停车函数
 * @param  无
 * @return 无
 * @note   立即关闭方向环 → 速度环目标置0 → 延时1s刹车
 *         → 关闭速度环 → PWM置0 → 开启菜单
 */
void stop_car(void);

/**
 * @brief  停车检测（主循环调用）
 * @param  无
 * @return 无
 * @note   检测prospect值，小于5时触发停车
 */
void stop_detection(void);

#endif // _CONTROL_H_




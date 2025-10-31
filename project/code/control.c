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

#include "zf_common_headfile.h"
#include "control.h"
#include "motor.h"
#include "menu.h"
#include "key.h"
#include "image.h"

// ==================== 全局变量 ====================
uint8 car_running = 0;              // 小车运行状态（0=停止，1=运行）
uint8 stop_flag = 0;                // 停车标志位

// ==================== 函数实现 ====================

/**
 * @brief  初始化控制模块
 * @param  无
 * @return 无
 */
void control_init(void)
{
    car_running = 0;
    stop_flag = 0;
}

/**
 * @brief  发车函数（菜单调用）
 * @param  无
 * @return 无
 * @note   第一次按确认键：显示提示，启动3秒倒计时
 *         3秒内再次按确认键：倒计时3-2-1后发车
 *         3秒内未按确认键：取消发车
 */
void start_car(void)
{
    uint32 timeout_count = 0;
    uint8 key_pressed = 0;
    
    // 清屏并显示提示信息
    ips114_clear();
    ips114_show_string(20, 40, "Press KEY3 again");
    ips114_show_string(20, 60, "in 3s to START");
    
    // 等待3秒内再次按下KEY3
    timeout_count = 0;
    while(timeout_count < 150)  // 150 * 20ms = 3000ms
    {
        // 扫描按键
        key_scanner();
        
        // 检测KEY3（确认键）
        if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
        {
            key_pressed = 1;
            break;
        }
        
        system_delay_ms(20);
        timeout_count++;
    }
    
    // 如果3秒内未按键，取消发车
    if(!key_pressed)
    {
        ips114_clear();
        ips114_show_string(40, 60, "START Cancelled");
        system_delay_ms(1000);
        menu_refresh();  // 刷新菜单显示
        return;
    }
    
    // 倒计时显示
    ips114_clear();
    ips114_show_string(40, 40, "Starting in 3...");
    system_delay_ms(1000);
    
    ips114_clear();
    ips114_show_string(40, 40, "Starting in 2...");
    system_delay_ms(1000);
    
    ips114_clear();
    ips114_show_string(40, 40, "Starting in 1...");
    system_delay_ms(1000);
    
    // 清屏（关闭显示）
    ips114_clear();
    
    // 设置运行状态
    car_running = 1;
    
    // 开启速度环和方向环
    speed_debug_enable = 1;
    direction_debug_enable = 1;
    
    // 退出菜单
    menu_exit();
}

/**
 * @brief  停车函数
 * @param  无
 * @return 无
 * @note   立即关闭方向环 → 速度环目标置0 → 延时1s刹车
 *         → 关闭速度环 → PWM置0 → 显示提示 → 等待按键 → 开启菜单
 */
void stop_car(void)
{
    // 1. 立即关闭方向环
    direction_debug_enable = 0;
    
    // 2. 速度环目标置0（启动刹车）
    motor_set_target_left(0);
    motor_set_target_right(0);
    
    // 3. 延时1000ms，让速度环执行刹车
    system_delay_ms(1000);
    
    // 4. 关闭速度环
    speed_debug_enable = 0;
    
    // 5. PWM置0（彻底停止）
    pwm_set_duty(MOTOR_LEFT_PWM, 0);
    pwm_set_duty(MOTOR_RIGHT_PWM, 0);
    
    // 6. 设置停止状态
    car_running = 0;
    
    // 7. 显示停车提示信息
    ips114_clear();
    ips114_show_string(30, 40, "CAR STOPPED!");
    ips114_show_string(20, 60, "Press KEY4 to");
    ips114_show_string(30, 80, "return menu");
    
    // 8. 等待按下KEY4返回键
    while(1)
    {
        // 扫描按键
        key_scanner();
        
        // 检测KEY4（返回键）
        if(key_get_state(KEY_4) == KEY_SHORT_PRESS || 
           key_get_state(KEY_4) == KEY_LONG_PRESS)
        {
            key_clear_state(KEY_4);  // 清除按键状态
            break;  // 退出等待循环
        }
        
        system_delay_ms(20);  // 20ms延时
    }
    
    // 9. 开启菜单
    menu_example_enter();
}

/**
 * @brief  停车检测（主循环调用）
 * @param  无
 * @return 无
 * @note   检测prospect值，小于5时触发停车
 */
void stop_detection(void)
{
    // 只在运行状态下检测
    if(car_running)
    {
        // 检测前瞻值
        if(prospect < 5)
        {
            // 设置停车标志
            stop_flag = 1;
            
            // 触发停车
            stop_car();
            
            // 清空标志
            stop_flag = 0;
        }
    }
}


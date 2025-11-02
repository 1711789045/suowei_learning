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
volatile uint32 system_time_ms = 0; // 系统运行时间(ms)，在PIT中断中递增

// ==================== 发车状态机变量 ====================
static start_state_t start_state = START_IDLE;   // 当前状态
static start_state_t last_state = START_IDLE;    // 上次状态（用于检测状态切换）
static uint32 start_timestamp = 0;               // 状态开始时间戳(ms)
static int16 target_speed_saved = 0;             // 保存的目标速度

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
    

    // 清屏（关闭显示）
    ips114_clear();
    
    // ========== 启动状态机发车（非阻塞） ==========
    
    // 1. 保存预设的basic_speed
    target_speed_saved = basic_speed;
    
    // 2. 设置运行状态
    car_running = 1;
    speed_debug_enable = 1;    
    direction_debug_enable = 1;
    
    // 3. 退出菜单
    menu_exit();
    
    // 4. 启动状态机（从原地调整状态开始）
    //    motor_reset()将在状态机进入START_ALIGN时调用
    start_state = START_ALIGN;
    start_timestamp = system_time_ms;  // 记录开始时间

}

/**
 * @brief  发车状态机处理（主循环调用）
 * @param  无
 * @return 无
 * @note   非阻塞方式执行发车时序，允许图像处理和方向环持续工作
 */
void start_car_process(void)
{
    uint32 elapsed_time;
    float accel_ratio;
    
    // 只在非空闲状态下执行
    if (start_state == START_IDLE)
    {
        return;
    }
    
    // 检测状态切换（刚进入新状态时执行初始化）
    if (start_state != last_state)
    {
        // 刚从IDLE切换到START_ALIGN状态时，重置电机控制系统
        if (last_state == START_IDLE && start_state == START_ALIGN)
        {
            motor_reset();  // ?? 在原地调整之前清零所有状态（PID+目标值+实际值+编码器滤波器+硬件计数器）
        }
        last_state = start_state;
    }
    
    // 计算已经过的时间（基于精确的定时器计数）
    elapsed_time = system_time_ms - start_timestamp;
    
    // 状态机（基于精确的毫秒计时）
    switch (start_state)
    {
        case START_ALIGN:  // 原地调整状态（basic_speed=0）
            basic_speed = 0;
            if (elapsed_time >= 1000)  // 精确1秒
            {
                start_state = START_ACCEL;
                start_timestamp = system_time_ms;  // 重置时间戳
            }
            break;
            
        case START_ACCEL:  // 线性加速状态（0%→100%，持续1秒）
            // 线性加速公式: speed = target × (elapsed_time / 1000)
            // elapsed_time: 0→1000ms
            // accel_ratio: 0.0→1.0
            accel_ratio = (float)elapsed_time / 1000.0f;
            
            // 限制在[0, 1]范围内
            if (accel_ratio > 1.0f)
                accel_ratio = 1.0f;
            
            basic_speed = (int16)(target_speed_saved * accel_ratio);
            
            if (elapsed_time >= 1000)  // 加速完成（1秒）
            {
                start_state = START_RUNNING;
                start_timestamp = system_time_ms;
            }
            break;
            
        case START_RUNNING:  // 正常运行状态（100%速度）
            basic_speed = target_speed_saved;
            start_state = START_IDLE;  // 完成，回到空闲状态
            break;
            
        default:
            start_state = START_IDLE;
            break;
    }
}

/**
 * @brief  停车函数
 * @param  无
 * @return 无
 * @note   立即关闭方向环 → 速度环目标置0 → 延时1s刹车
 *         → 关闭速度环 → PWM置0 → 开启菜单
 */
void stop_car(void)
{
    // 1. 立即关闭方向环
    direction_debug_enable = 0;
    
    // 2. 速度环目标置0（启动刹车）
    motor_set_target_left(0);
    motor_set_target_right(0);
    
    
    // 4. 关闭速度环
    speed_debug_enable = 0;
    
    // 5. PWM置0（彻底停止）
    pwm_set_duty(MOTOR_LEFT_PWM, 0);
    pwm_set_duty(MOTOR_RIGHT_PWM, 0);
    
    // 6. 设置停止状态
    car_running = 0;
    
    // 7. 开启菜单
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


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
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          key
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-10-26       Claude             菜单按键处理模块
********************************************************************************************************************/

#include "key.h"

// ==================== 配置参数 ====================
#define LONG_PRESS_TIME         30      // 长按检测时间（单位：扫描周期，约30*20ms=600ms）
#define LONG_PRESS_REPEAT_TIME  5       // 长按连续触发间隔（单位：扫描周期，约5*20ms=100ms）

// ==================== 内部变量 ====================
static uint32 key_scan_period = 20;     // 按键扫描周期（毫秒）
static uint8 long_press_cnt = 0;        // 长按计数器
static uint8 long_press_button = 0;     // 长按按键ID（0=无，1-4对应KEY1-4）
static uint8 long_press_repeat_cnt = 0; // 长按连续触发计数器
static menu_key_e key_event = MENU_KEY_NONE;    // 按键事件

// 按键状态变量
static key_state_enum key1_last = KEY_RELEASE;
static key_state_enum key2_last = KEY_RELEASE;
static key_state_enum key3_last = KEY_RELEASE;
static key_state_enum key4_last = KEY_RELEASE;

// ==================== API函数实现 ====================

/**
 * @brief  初始化菜单按键
 */
void menu_key_init(uint32 scan_period)
{
    key_scan_period = scan_period;

    // 初始化按键（使用逐飞按键驱动）
    key_init(10);   // 初始化按键，连续10次采样相同才认为状态有效

    // 清除状态
    long_press_cnt = 0;
    long_press_button = 0;
    long_press_repeat_cnt = 0;
    key_event = MENU_KEY_NONE;

    printf("[KEY] Menu key initialized (scan period: %d ms)\r\n", scan_period);
}

/**
 * @brief  扫描按键状态
 */
void menu_key_scan(void)
{
    // 获取当前按键状态
    key_state_enum key1_state = key_get_state(KEY_1);
    key_state_enum key2_state = key_get_state(KEY_2);
    key_state_enum key3_state = key_get_state(KEY_3);
    key_state_enum key4_state = key_get_state(KEY_4);

    // 长按检测（检测按键持续按下）
    if (key1_state == KEY_SHORT_PRESS || key1_state == KEY_LONG_PRESS)
    {
        long_press_cnt++;
        if (long_press_cnt >= LONG_PRESS_TIME)
        {
            long_press_cnt = LONG_PRESS_TIME;
            long_press_button = 1;
        }
    }
    else if (key2_state == KEY_SHORT_PRESS || key2_state == KEY_LONG_PRESS)
    {
        long_press_cnt++;
        if (long_press_cnt >= LONG_PRESS_TIME)
        {
            long_press_cnt = LONG_PRESS_TIME;
            long_press_button = 2;
        }
    }
    else if (key3_state == KEY_SHORT_PRESS || key3_state == KEY_LONG_PRESS)
    {
        long_press_cnt++;
        if (long_press_cnt >= LONG_PRESS_TIME)
        {
            long_press_cnt = LONG_PRESS_TIME;
            long_press_button = 3;
        }
    }
    else if (key4_state == KEY_SHORT_PRESS || key4_state == KEY_LONG_PRESS)
    {
        long_press_cnt++;
        if (long_press_cnt >= LONG_PRESS_TIME)
        {
            long_press_cnt = LONG_PRESS_TIME;
            long_press_button = 4;
        }
    }
    else
    {
        long_press_cnt = 0;
        long_press_button = 0;
    }

    // 检测按键事件（释放时触发或长按连续触发）
    if (key_event == MENU_KEY_NONE)  // 如果上次事件已被处理
    {
        // 长按连续触发逻辑
        if (long_press_button > 0)
        {
            long_press_repeat_cnt++;
            if (long_press_repeat_cnt >= LONG_PRESS_REPEAT_TIME)
            {
                long_press_repeat_cnt = 0;  // 重置重复计数器，准备下次触发

                // 根据长按的按键ID发送对应事件
                if (long_press_button == 1)
                    key_event = MENU_KEY_UP;
                else if (long_press_button == 2)
                    key_event = MENU_KEY_DOWN;
                else if (long_press_button == 3)
                    key_event = MENU_KEY_ENTER;
                else if (long_press_button == 4)
                    key_event = MENU_KEY_BACK;
            }
        }
        // 短按检测（按键释放时触发）
        else
        {
            // KEY1: 上键
            if (key1_state == KEY_RELEASE && key1_last != KEY_RELEASE)
            {
                key_event = MENU_KEY_UP;
            }
            // KEY2: 下键
            else if (key2_state == KEY_RELEASE && key2_last != KEY_RELEASE)
            {
                key_event = MENU_KEY_DOWN;
            }
            // KEY3: 确认键
            else if (key3_state == KEY_RELEASE && key3_last != KEY_RELEASE)
            {
                key_event = MENU_KEY_ENTER;
            }
            // KEY4: 返回键
            else if (key4_state == KEY_RELEASE && key4_last != KEY_RELEASE)
            {
                key_event = MENU_KEY_BACK;
            }
        }
    }

    // 保存按键状态
    key1_last = key1_state;
    key2_last = key2_state;
    key3_last = key3_state;
    key4_last = key4_state;
}

/**
 * @brief  获取按键事件
 */
menu_key_e menu_key_get_event(void)
{
    menu_key_e event = key_event;
    key_event = MENU_KEY_NONE;  // 清除事件标志
    return event;
}

/**
 * @brief  检查是否有按键按下
 */
uint8 menu_key_is_pressed(void)
{
    return (key_event != MENU_KEY_NONE) ? 1 : 0;
}

/**
 * @brief  清除所有按键状态
 */
void menu_key_clear_all(void)
{
    key_event = MENU_KEY_NONE;
    long_press_cnt = 0;
    long_press_button = 0;
    long_press_repeat_cnt = 0;
}

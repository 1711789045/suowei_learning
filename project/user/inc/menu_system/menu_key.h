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
* 文件名称          menu_key
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-10-26       Claude             移植自龙芯C车代码，使用zf_device_key驱动
********************************************************************************************************************/

#ifndef _MENU_KEY_H_
#define _MENU_KEY_H_

#include "zf_common_headfile.h"

// ==================== 按键定义 ====================
// 按键功能枚举（与菜单操作对应）
typedef enum {
    MENU_KEY_NONE = 0,          // 无按键
    MENU_KEY_UP,                // 上键（KEY1）
    MENU_KEY_DOWN,              // 下键（KEY2）
    MENU_KEY_ENTER,             // 确认键（KEY3）
    MENU_KEY_BACK               // 返回键（KEY4）
} menu_key_e;

// 按键事件类型
typedef enum {
    MENU_KEY_EVENT_NONE = 0,    // 无事件
    MENU_KEY_EVENT_SHORT,       // 短按事件
    MENU_KEY_EVENT_LONG         // 长按事件
} menu_key_event_e;

// ==================== API 函数 ====================

/**
 * @brief  初始化菜单按键
 * @param  scan_period: 按键扫描周期（毫秒）
 * @return 无
 * @note   在菜单系统初始化时调用
 */
void menu_key_init(uint32 scan_period);

/**
 * @brief  扫描按键状态
 * @param  无
 * @return 无
 * @note   在定时器中周期调用（建议20ms）
 */
void menu_key_scan(void);

/**
 * @brief  获取按键事件
 * @param  无
 * @return 按键事件（menu_key_e类型）
 * @note   返回后自动清除事件标志
 *         如果有按键按下，返回对应的按键代码，否则返回MENU_KEY_NONE
 */
menu_key_e menu_key_get_event(void);

/**
 * @brief  检查是否有按键按下
 * @param  无
 * @return 0=无按键, 1=有按键
 */
uint8 menu_key_is_pressed(void);

/**
 * @brief  清除所有按键状态
 * @param  无
 * @return 无
 */
void menu_key_clear_all(void);

#endif

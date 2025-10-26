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
* 文件名称          menu_display
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-10-26       Claude             移植自龙芯C车代码，适配IPS114屏幕
********************************************************************************************************************/

#ifndef _MENU_DISPLAY_H_
#define _MENU_DISPLAY_H_

#include "zf_common_headfile.h"
#include "menu_config.h"

// ==================== 屏幕参数（IPS114: 240x135）====================
#define MENU_SCREEN_W           240             // 屏幕宽度
#define MENU_SCREEN_H           135             // 屏幕高度
#define MENU_FONT_H             16              // 字体高度（8x16字体）
#define MENU_ITEMS_PER_PAGE     6               // 每页显示的参数数量（原IPS200为8，IPS114高度小，改为6）

#define MENU_TITLE_Y            0               // 标题Y坐标
#define MENU_ITEM_START_Y       MENU_FONT_H     // 第一个菜单项Y坐标

// 颜色定义
#define MENU_COLOR_BG           RGB565_BLACK    // 背景色
#define MENU_COLOR_TEXT         RGB565_WHITE    // 文本颜色
#define MENU_COLOR_SELECT       RGB565_BLUE     // 选中项颜色
#define MENU_COLOR_EDIT         RGB565_RED      // 编辑模式颜色

// ==================== 菜单单元类型 ====================
typedef enum {
    MENU_UNIT_NORMAL = 0,       // 普通参数
    MENU_UNIT_FUNCTION,         // 功能函数
    MENU_UNIT_PAGE              // 页面入口
} menu_unit_type_e;

// ==================== 菜单单元结构体 ====================
// 菜单单元（链表节点）
typedef struct MENU_UNIT {
    char name[16];                      // 显示名称
    menu_unit_type_e type;              // 单元类型

    // 参数相关（仅当type=MENU_UNIT_NORMAL时有效）
    void *param_ptr;                    // 参数指针
    config_type_e param_type;           // 参数类型
    float delta;                        // 调整增量
    uint8 num_int;                      // 显示整数位数
    uint8 num_dec;                      // 显示小数位数

    // 功能函数（仅当type=MENU_UNIT_FUNCTION时有效）
    void (*function)(void);             // 功能函数指针

    // 链表指针
    struct MENU_UNIT *up;               // 上一项
    struct MENU_UNIT *down;             // 下一项
    struct MENU_UNIT *enter;            // 进入子菜单/页面
    struct MENU_UNIT *back;             // 返回上级菜单

    // 索引（用于快速定位）
    uint8 index[2];                     // [页面索引, 项目索引]
} menu_unit_t;

// ==================== API 函数 ====================

/**
 * @brief  初始化菜单显示
 * @param  无
 * @return 无
 */
void menu_display_init(void);

/**
 * @brief  显示菜单标题
 * @param  title: 标题字符串
 * @return 无
 */
void menu_display_title(const char *title);

/**
 * @brief  显示菜单项
 * @param  unit: 菜单单元指针
 * @param  line: 行号（0-5）
 * @param  selected: 是否选中
 * @param  edit_mode: 是否处于编辑模式
 * @return 无
 */
void menu_display_item(menu_unit_t *unit, uint8 line, uint8 selected, uint8 edit_mode);

/**
 * @brief  显示参数值
 * @param  unit: 菜单单元指针
 * @param  x: X坐标
 * @param  y: Y坐标
 * @param  color: 文本颜色
 * @return 无
 */
void menu_display_param_value(menu_unit_t *unit, uint16 x, uint16 y, uint16 color);

/**
 * @brief  清除指定区域
 * @param  x1: 起始X坐标
 * @param  y1: 起始Y坐标
 * @param  x2: 结束X坐标
 * @param  y2: 结束Y坐标
 * @return 无
 */
void menu_display_clear_area(uint16 x1, uint16 y1, uint16 x2, uint16 y2);

/**
 * @brief  清屏
 * @param  无
 * @return 无
 */
void menu_display_clear(void);

/**
 * @brief  显示字符串
 * @param  x: X坐标
 * @param  y: Y坐标
 * @param  str: 字符串
 * @param  color: 文本颜色
 * @return 无
 */
void menu_display_string(uint16 x, uint16 y, const char *str, uint16 color);

/**
 * @brief  显示整数
 * @param  x: X坐标
 * @param  y: Y坐标
 * @param  value: 整数值
 * @param  color: 文本颜色
 * @return 无
 */
void menu_display_int(uint16 x, uint16 y, int32 value, uint16 color);

/**
 * @brief  显示浮点数
 * @param  x: X坐标
 * @param  y: Y坐标
 * @param  value: 浮点数值
 * @param  num_int: 整数位数
 * @param  num_dec: 小数位数
 * @param  color: 文本颜色
 * @return 无
 */
void menu_display_float(uint16 x, uint16 y, float value, uint8 num_int, uint8 num_dec, uint16 color);

#endif

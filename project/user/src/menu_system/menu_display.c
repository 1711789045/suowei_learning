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

#include "menu_display.h"
#include <stdio.h>

// ==================== API函数实现 ====================

/**
 * @brief  初始化菜单显示
 */
void menu_display_init(void)
{
    // 初始化IPS114屏幕（竖屏模式）
    ips114_init();
    ips114_set_dir(IPS114_PORTAIT);
    ips114_set_color(MENU_COLOR_TEXT, MENU_COLOR_BG);
    ips114_clear();

    printf("[MENU_DISPLAY] Display initialized (IPS114 %dx%d)\r\n", MENU_SCREEN_W, MENU_SCREEN_H);
}

/**
 * @brief  显示菜单标题
 */
void menu_display_title(const char *title)
{
    // 清除标题区域
    menu_display_clear_area(0, MENU_TITLE_Y, MENU_SCREEN_W - 1, MENU_FONT_H - 1);

    // 显示标题（居中）
    uint16 title_x = (MENU_SCREEN_W - strlen(title) * 8) / 2;  // 8x16字体，每字符宽8像素
    ips114_show_string(title_x, MENU_TITLE_Y, title);
}

/**
 * @brief  显示菜单项
 */
void menu_display_item(menu_unit_t *unit, uint8 line, uint8 selected, uint8 edit_mode)
{
    if (unit == NULL || line >= MENU_ITEMS_PER_PAGE)
        return;

    uint16 y = MENU_ITEM_START_Y + line * MENU_FONT_H;
    uint16 color = MENU_COLOR_TEXT;

    // 清除该行
    menu_display_clear_area(0, y, MENU_SCREEN_W - 1, y + MENU_FONT_H - 1);

    // 显示选中标记
    if (selected)
    {
        ips114_show_string(0, y, ">");
        if (edit_mode)
            color = MENU_COLOR_EDIT;    // 编辑模式用红色
        else
            color = MENU_COLOR_SELECT;  // 选中用蓝色
    }

    // 显示名称
    ips114_show_string(10, y, unit->name);

    // 显示值（如果是参数）
    if (unit->type == MENU_UNIT_NORMAL && unit->param_ptr != NULL)
    {
        menu_display_param_value(unit, 120, y, color);
    }
}

/**
 * @brief  显示参数值
 */
void menu_display_param_value(menu_unit_t *unit, uint16 x, uint16 y, uint16 color)
{
    if (unit == NULL || unit->param_ptr == NULL)
        return;

    char buf[32];

    switch (unit->param_type)
    {
        case CONFIG_TYPE_FLOAT:
        {
            float val = *(float *)unit->param_ptr;
            sprintf(buf, "%.*f", unit->num_dec, val);
            ips114_show_string(x, y, buf);
            break;
        }
        case CONFIG_TYPE_DOUBLE:
        {
            double val = *(double *)unit->param_ptr;
            sprintf(buf, "%.*lf", unit->num_dec, val);
            ips114_show_string(x, y, buf);
            break;
        }
        case CONFIG_TYPE_INT:
        {
            int val = *(int *)unit->param_ptr;
            sprintf(buf, "%d", val);
            ips114_show_string(x, y, buf);
            break;
        }
        case CONFIG_TYPE_INT16:
        {
            int16 val = *(int16 *)unit->param_ptr;
            sprintf(buf, "%d", val);
            ips114_show_string(x, y, buf);
            break;
        }
        case CONFIG_TYPE_INT8:
        {
            int8 val = *(int8 *)unit->param_ptr;
            sprintf(buf, "%d", val);
            ips114_show_string(x, y, buf);
            break;
        }
        case CONFIG_TYPE_UINT32:
        {
            uint32 val = *(uint32 *)unit->param_ptr;
            sprintf(buf, "%u", val);
            ips114_show_string(x, y, buf);
            break;
        }
        case CONFIG_TYPE_UINT16:
        {
            uint16 val = *(uint16 *)unit->param_ptr;
            sprintf(buf, "%u", val);
            ips114_show_string(x, y, buf);
            break;
        }
        case CONFIG_TYPE_UINT8:
        {
            uint8 val = *(uint8 *)unit->param_ptr;
            sprintf(buf, "%u", val);
            ips114_show_string(x, y, buf);
            break;
        }
    }
}

/**
 * @brief  清除指定区域
 */
void menu_display_clear_area(uint16 x1, uint16 y1, uint16 x2, uint16 y2)
{
    ips114_draw_rectangle(x1, y1, x2, y2, MENU_COLOR_BG);
}

/**
 * @brief  清屏
 */
void menu_display_clear(void)
{
    ips114_clear();
}

/**
 * @brief  显示字符串
 */
void menu_display_string(uint16 x, uint16 y, const char *str, uint16 color)
{
    ips114_show_string(x, y, str);
}

/**
 * @brief  显示整数
 */
void menu_display_int(uint16 x, uint16 y, int32 value, uint16 color)
{
    char buf[16];
    sprintf(buf, "%d", value);
    ips114_show_string(x, y, buf);
}

/**
 * @brief  显示浮点数
 */
void menu_display_float(uint16 x, uint16 y, float value, uint8 num_int, uint8 num_dec, uint16 color)
{
    char buf[32];
    sprintf(buf, "%.*f", num_dec, value);
    ips114_show_string(x, y, buf);
}

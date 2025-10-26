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
* 文件名称          menu_core
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-10-26       Claude             移植自龙芯C车代码，增强存档位功能
********************************************************************************************************************/

#include "menu_core.h"
#include <string.h>

// ==================== 内部数据 ====================
static menu_unit_t menu_units[MENU_MAX_UNITS];     // 菜单单元池
static uint8 menu_unit_count = 0;                  // 已使用的菜单单元数量

static menu_unit_t *current_unit = NULL;           // 当前选中的菜单单元
static menu_unit_t *page_first_unit = NULL;        // 当前页面第一个显示的单元
static uint8 current_line = 0;                     // 当前选中行（0-5）
static uint8 edit_mode = 0;                        // 编辑模式标志
static uint8 menu_active = 0;                      // 菜单激活标志

// ==================== 内部函数声明 ====================
static void menu_navigate_up(void);
static void menu_navigate_down(void);
static void menu_navigate_enter(void);
static void menu_navigate_back(void);
static void menu_edit_param(int8 direction);
static void menu_refresh_page(void);

// ==================== API函数实现 ====================

/**
 * @brief  初始化菜单系统
 */
void menu_init(void)
{
    printf("[MENU] Initializing menu system...\r\n");

    // 初始化各模块
    menu_display_init();
    menu_key_init(20);  // 20ms扫描周期
    config_init();

    // 初始化变量
    menu_unit_count = 0;
    current_unit = NULL;
    page_first_unit = NULL;
    current_line = 0;
    edit_mode = 0;
    menu_active = 0;

    printf("[MENU] Menu system initialized\r\n");
}

/**
 * @brief  创建菜单单元
 */
menu_unit_t* menu_create_unit(const char *name, menu_unit_type_e type)
{
    if (menu_unit_count >= MENU_MAX_UNITS)
    {
        printf("[MENU ERROR] Menu units limit exceeded!\r\n");
        return NULL;
    }

    menu_unit_t *unit = &menu_units[menu_unit_count++];
    memset(unit, 0, sizeof(menu_unit_t));

    // 设置名称
    strncpy(unit->name, name, sizeof(unit->name) - 1);
    unit->name[sizeof(unit->name) - 1] = '\0';

    // 设置类型
    unit->type = type;

    return unit;
}

/**
 * @brief  创建参数单元
 */
menu_unit_t* menu_create_param(const char *name, void *param_ptr, config_type_e param_type,
                                float delta, uint8 num_int, uint8 num_dec)
{
    menu_unit_t *unit = menu_create_unit(name, MENU_UNIT_NORMAL);
    if (unit == NULL)
        return NULL;

    unit->param_ptr = param_ptr;
    unit->param_type = param_type;
    unit->delta = delta;
    unit->num_int = num_int;
    unit->num_dec = num_dec;

    return unit;
}

/**
 * @brief  创建功能单元
 */
menu_unit_t* menu_create_function(const char *name, void (*function)(void))
{
    menu_unit_t *unit = menu_create_unit(name, MENU_UNIT_FUNCTION);
    if (unit == NULL)
        return NULL;

    unit->function = function;

    return unit;
}

/**
 * @brief  链接菜单单元
 */
void menu_link(menu_unit_t *current, menu_unit_t *up, menu_unit_t *down,
               menu_unit_t *enter, menu_unit_t *back)
{
    if (current == NULL)
        return;

    current->up = up;
    current->down = down;
    current->enter = enter;
    current->back = back;
}

/**
 * @brief  菜单处理函数
 */
void menu_process(void)
{
    if (!menu_active)
        return;

    // 扫描按键
    menu_key_scan();

    // 获取按键事件
    menu_key_e key = menu_key_get_event();

    if (key == MENU_KEY_NONE)
        return;

    // 处理按键
    if (edit_mode)
    {
        // 编辑模式
        switch (key)
        {
            case MENU_KEY_UP:
                menu_edit_param(1);     // 增加
                break;
            case MENU_KEY_DOWN:
                menu_edit_param(-1);    // 减少
                break;
            case MENU_KEY_ENTER:
            case MENU_KEY_BACK:
                edit_mode = 0;          // 退出编辑模式
                menu_refresh();
                break;
            default:
                break;
        }
    }
    else
    {
        // 普通模式
        switch (key)
        {
            case MENU_KEY_UP:
                menu_navigate_up();
                break;
            case MENU_KEY_DOWN:
                menu_navigate_down();
                break;
            case MENU_KEY_ENTER:
                menu_navigate_enter();
                break;
            case MENU_KEY_BACK:
                menu_navigate_back();
                break;
            default:
                break;
        }
    }
}

/**
 * @brief  刷新菜单显示
 */
void menu_refresh(void)
{
    menu_refresh_page();
}

/**
 * @brief  进入菜单
 */
void menu_enter(menu_unit_t *root)
{
    if (root == NULL)
        return;

    current_unit = root;
    page_first_unit = root;
    current_line = 0;
    edit_mode = 0;
    menu_active = 1;

    menu_refresh();
}

/**
 * @brief  退出菜单
 */
void menu_exit(void)
{
    menu_active = 0;
    menu_display_clear();
}

/**
 * @brief  检查菜单是否激活
 */
uint8 menu_is_active(void)
{
    return menu_active;
}

// ==================== 内部函数实现 ====================

/**
 * @brief  向上导航
 */
static void menu_navigate_up(void)
{
    if (current_unit == NULL || current_unit->up == NULL)
        return;

    current_unit = current_unit->up;

    if (current_line > 0)
    {
        current_line--;
    }
    else
    {
        // 需要向上滚动页面
        page_first_unit = current_unit;
    }

    menu_refresh();
}

/**
 * @brief  向下导航
 */
static void menu_navigate_down(void)
{
    if (current_unit == NULL || current_unit->down == NULL)
        return;

    current_unit = current_unit->down;

    if (current_line < MENU_ITEMS_PER_PAGE - 1)
    {
        current_line++;
    }
    else
    {
        // 需要向下滚动页面
        menu_unit_t *temp = page_first_unit;
        if (temp != NULL && temp->down != NULL)
            page_first_unit = temp->down;
    }

    menu_refresh();
}

/**
 * @brief  进入/确认
 */
static void menu_navigate_enter(void)
{
    if (current_unit == NULL)
        return;

    if (current_unit->type == MENU_UNIT_NORMAL)
    {
        // 参数项：进入编辑模式
        edit_mode = 1;
        menu_refresh();
    }
    else if (current_unit->type == MENU_UNIT_FUNCTION)
    {
        // 功能项：执行函数
        if (current_unit->function != NULL)
        {
            current_unit->function();
        }
    }
    else if (current_unit->enter != NULL)
    {
        // 页面入口：进入子菜单
        current_unit = current_unit->enter;
        page_first_unit = current_unit;
        current_line = 0;
        menu_refresh();
    }
}

/**
 * @brief  返回
 */
static void menu_navigate_back(void)
{
    if (current_unit == NULL || current_unit->back == NULL)
        return;

    current_unit = current_unit->back;
    page_first_unit = current_unit;
    current_line = 0;
    menu_refresh();
}

/**
 * @brief  编辑参数
 */
static void menu_edit_param(int8 direction)
{
    if (current_unit == NULL || current_unit->param_ptr == NULL)
        return;

    float change = current_unit->delta * direction;

    switch (current_unit->param_type)
    {
        case CONFIG_TYPE_FLOAT:
            *(float *)current_unit->param_ptr += change;
            break;
        case CONFIG_TYPE_DOUBLE:
            *(double *)current_unit->param_ptr += (double)change;
            break;
        case CONFIG_TYPE_INT:
            *(int *)current_unit->param_ptr += (int)change;
            break;
        case CONFIG_TYPE_INT16:
            *(int16 *)current_unit->param_ptr += (int16)change;
            break;
        case CONFIG_TYPE_INT8:
            *(int8 *)current_unit->param_ptr += (int8)change;
            break;
        case CONFIG_TYPE_UINT32:
            if (direction > 0 || *(uint32 *)current_unit->param_ptr >= (uint32)(-change))
                *(uint32 *)current_unit->param_ptr += (uint32)change;
            break;
        case CONFIG_TYPE_UINT16:
            if (direction > 0 || *(uint16 *)current_unit->param_ptr >= (uint16)(-change))
                *(uint16 *)current_unit->param_ptr += (uint16)change;
            break;
        case CONFIG_TYPE_UINT8:
            if (direction > 0 || *(uint8 *)current_unit->param_ptr >= (uint8)(-change))
                *(uint8 *)current_unit->param_ptr += (uint8)change;
            break;
    }

    // 只刷新当前行
    menu_display_item(current_unit, current_line, 1, edit_mode);
}

/**
 * @brief  刷新页面显示
 */
static void menu_refresh_page(void)
{
    menu_display_clear();

    // 显示标题
    menu_display_title("Menu System");

    // 显示菜单项
    menu_unit_t *unit = page_first_unit;
    for (uint8 i = 0; i < MENU_ITEMS_PER_PAGE && unit != NULL; i++)
    {
        uint8 selected = (unit == current_unit) ? 1 : 0;
        menu_display_item(unit, i, selected, edit_mode && selected);
        unit = unit->down;
    }
}

// ==================== 内置功能函数 ====================

/**
 * @brief  保存配置到存档位（内部功能）
 */
void menu_func_save_config(void)
{
    // TODO: 实现存档位选择界面
    // 暂时保存到存档位0
    config_save_slot(0);
    printf("[MENU] Configuration saved to slot 0\r\n");
}

/**
 * @brief  加载配置从存档位（内部功能）
 */
void menu_func_load_config(void)
{
    // TODO: 实现存档位选择界面
    // 暂时从存档位0加载
    config_load_slot(0);
    printf("[MENU] Configuration loaded from slot 0\r\n");
    menu_refresh();
}

/**
 * @brief  重置为默认值
 */
void menu_func_reset_default(void)
{
    config_reset_default();
    printf("[MENU] Reset to default values\r\n");
    menu_refresh();
}

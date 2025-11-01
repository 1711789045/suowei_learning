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
* 文件名称          menu
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-10-26       Claude             菜单系统（核心+显示+示例）
********************************************************************************************************************/

#include "menu.h"
#include "pid.h"            // PID参数（motor_kp/ki/kd）
#include "motor.h"          // 电机控制（VOFA+开关）
#include "flash.h"          // Flash配置管理（自动保存/加载）
#include "image.h"          // 图像处理模块
#include "control.h"        // 发车/停车控制
#include <string.h>
#include <stdio.h>

// ==================== 显示模块内部函数 ====================

/**
 * @brief  显示参数值
 */
static void menu_display_param_value(menu_unit_t *unit, uint16 x, uint16 y, uint16 color)
{
    if (unit == NULL || unit->param_ptr == NULL)
        return;

    char buf[32];

    // 设置显示颜色
    ips114_set_color(color, MENU_COLOR_BG);

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
            // 使用固定宽度格式化，确保清除旧字符
            sprintf(buf, "%-5d", val);  // 左对齐，5位宽度，不足用空格填充
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

    // 恢复默认颜色
    ips114_set_color(MENU_COLOR_TEXT, MENU_COLOR_BG);
}

/**
 * @brief  显示菜单项
 */
static void menu_display_item(menu_unit_t *unit, uint8 line, uint8 selected, uint8 edit_mode)
{
    if (unit == NULL || line >= MENU_ITEMS_PER_PAGE)
        return;

    uint16 y = MENU_ITEM_START_Y + line * MENU_FONT_H;
    uint16 color = MENU_COLOR_TEXT;

    // 不再清除整行，直接覆盖绘制（提升性能）
    // 背景会在ips114_show_string时自动使用背景色填充

    // 显示选中标记或清空标记位
    if (selected)
    {
        ips114_show_string(0, y, ">");
        if (edit_mode)
            color = MENU_COLOR_EDIT;    // 编辑模式用红色
        else
            color = MENU_COLOR_SELECT;  // 选中用蓝色
    }
    else
    {
        ips114_show_string(0, y, " ");  // 清空光标位置
    }

    // 显示名称（使用相应颜色）
    ips114_set_color(color, MENU_COLOR_BG);
    ips114_show_string(10, y, unit->name);
    ips114_set_color(MENU_COLOR_TEXT, MENU_COLOR_BG);  // 恢复默认颜色

    // 显示值（如果是参数）
    if (unit->type == MENU_UNIT_NORMAL && unit->param_ptr != NULL)
    {
        menu_display_param_value(unit, 120, y, color);
    }
}

/**
 * @brief  只刷新光标位置（优化刷新速度）
 */
static void menu_display_cursor(uint8 line, uint8 show)
{
    if (line >= MENU_ITEMS_PER_PAGE)
        return;

    uint16 y = MENU_ITEM_START_Y + line * MENU_FONT_H;

    // 直接覆盖绘制，不画线
    if (show)
    {
        ips114_show_string(0, y, ">");
    }
    else
    {
        ips114_show_string(0, y, " ");  // 空格清除光标
    }
}

// ==================== 菜单核心模块数据 ====================
static menu_unit_t menu_units[MENU_MAX_UNITS];     // 菜单单元池
static uint8 menu_unit_count = 0;                  // 已使用的菜单单元数量

static menu_unit_t *current_unit = NULL;           // 当前选中的菜单单元
static menu_unit_t *page_first_unit = NULL;        // 当前页面第一个显示的单元
static uint8 current_line = 0;                     // 当前选中行（0-7）
static uint8 last_line = 0;                        // 上次选中行（用于局部刷新）
static uint8 edit_mode = 0;                        // 编辑模式标志
static uint8 menu_active = 0;                      // 菜单激活标志

// ==================== 延迟更新参数 ====================
// basic_speed临时变量：编辑时修改此变量，退出编辑才更新到motor.h的basic_speed
static int16 temp_basic_speed = 100;
static menu_unit_t *basic_speed_menu_unit = NULL;  // basic_speed菜单项指针（用于判断）

// 菜单导航栈（记住每层菜单的状态）
#define MENU_STACK_SIZE 10
typedef struct {
    menu_unit_t *page_first;
    menu_unit_t *current;
    uint8 line;
} menu_state_t;
static menu_state_t menu_stack[MENU_STACK_SIZE];
static uint8 menu_stack_depth = 0;

// ==================== 菜单核心内部函数 ====================
static void menu_navigate_up(void);
static void menu_navigate_down(void);
static void menu_navigate_enter(void);
static void menu_navigate_back(void);
static void menu_edit_param(int8 direction);
static void menu_refresh_page(void);
static void menu_refresh_partial(void);

/**
 * @brief  计算当前层级菜单项数量
 */
static uint8 menu_count_items(menu_unit_t *first)
{
    uint8 count = 0;
    menu_unit_t *p = first;
    while (p != NULL && count < MENU_MAX_UNITS)  // 防止无限循环
    {
        count++;
        p = p->down;
        if (p == first)  // 检测到循环链接
            break;
    }
    return count;
}

/**
 * @brief  获取当前层级最后一项
 */
static menu_unit_t* menu_get_last_item(menu_unit_t *first)
{
    menu_unit_t *p = first;
    while (p != NULL && p->down != NULL)
    {
        p = p->down;
    }
    return p;
}

/**
 * @brief  向上导航（只移动光标，不移动内容）
 */
static void menu_navigate_up(void)
{
    if (current_unit == NULL || page_first_unit == NULL)
        return;

    // 计算当前层级的菜单项总数（带循环检测）
    uint8 total_items = 0;
    menu_unit_t *p = page_first_unit;
    while (p != NULL && total_items < MENU_MAX_UNITS)
    {
        total_items++;
        p = p->down;
        if (p == page_first_unit)  // 检测到循环
            break;
    }

    if (total_items == 0)
        return;

    // 保存旧的光标位置和旧的菜单单元
    uint8 old_line = current_line;
    menu_unit_t *old_unit = current_unit;

    // 向上移动光标（循环）
    if (current_line == 0)
    {
        // 在第一行，跳到最后一行
        current_line = (total_items > MENU_ITEMS_PER_PAGE) ? (MENU_ITEMS_PER_PAGE - 1) : (total_items - 1);
    }
    else
    {
        // 光标上移一行
        current_line--;
    }

    // 更新current_unit指针
    current_unit = page_first_unit;
    for (uint8 i = 0; i < current_line; i++)
    {
        if (current_unit->down != NULL)
            current_unit = current_unit->down;
    }

    // 刷新颜色：旧位置恢复白色，新位置变绿色
    menu_display_item(old_unit, old_line, 0, 0);        // 旧位置恢复为未选中状态（白色）
    menu_display_item(current_unit, current_line, 1, 0); // 新位置显示为选中状态（绿色）
}

/**
 * @brief  向下导航（只移动光标，不移动内容）
 */
static void menu_navigate_down(void)
{
    if (current_unit == NULL || page_first_unit == NULL)
        return;

    // 计算当前层级的菜单项总数（带循环检测）
    uint8 total_items = 0;
    menu_unit_t *p = page_first_unit;
    while (p != NULL && total_items < MENU_MAX_UNITS)
    {
        total_items++;
        p = p->down;
        if (p == page_first_unit)  // 检测到循环
            break;
    }

    if (total_items == 0)
        return;

    // 保存旧的光标位置和旧的菜单单元
    uint8 old_line = current_line;
    menu_unit_t *old_unit = current_unit;

    // 向下移动光标（循环）
    uint8 max_line = (total_items > MENU_ITEMS_PER_PAGE) ? (MENU_ITEMS_PER_PAGE - 1) : (total_items - 1);
    if (current_line >= max_line)
    {
        // 在最后一行，跳到第一行
        current_line = 0;
    }
    else
    {
        // 光标下移一行
        current_line++;
    }

    // 更新current_unit指针
    current_unit = page_first_unit;
    for (uint8 i = 0; i < current_line; i++)
    {
        if (current_unit->down != NULL)
            current_unit = current_unit->down;
    }

    // 刷新颜色：旧位置恢复白色，新位置变绿色
    menu_display_item(old_unit, old_line, 0, 0);        // 旧位置恢复为未选中状态（白色）
    menu_display_item(current_unit, current_line, 1, 0); // 新位置显示为选中状态（绿色）
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
        // ⭐ 进入编辑模式前，同步真实值到临时变量
        if (current_unit == basic_speed_menu_unit)
        {
            temp_basic_speed = basic_speed;  // 同步当前值
        }
        
        // 参数项：进入编辑模式（只局部刷新当前行）
        edit_mode = 1;
        menu_display_item(current_unit, current_line, 1, 1);  // 显示为编辑模式颜色
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
        // 页面入口：进入子菜单（保存当前状态到栈）
        if (menu_stack_depth < MENU_STACK_SIZE)
        {
            menu_stack[menu_stack_depth].page_first = page_first_unit;
            menu_stack[menu_stack_depth].current = current_unit;
            menu_stack[menu_stack_depth].line = current_line;
            menu_stack_depth++;
        }

        // 进入子菜单
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
    if (edit_mode)
    {
        // 如果在编辑模式，退出编辑模式（只局部刷新当前行）
        edit_mode = 0;
        
        // ⭐ 检查是否是basic_speed参数，如果是则更新真实值
        if (current_unit == basic_speed_menu_unit)
        {
            basic_speed = temp_basic_speed;  // 延迟更新：临时变量 → 真实变量
            printf("[MENU] Basic Speed updated: %d (delayed update)\r\n", basic_speed);
        }
        
        menu_display_item(current_unit, current_line, 1, 0);  // 显示为选中模式颜色

        // 退出编辑模式时自动保存（掉电不丢失）
        config_auto_save();

        return;
    }

    // 从栈中恢复上级菜单状态
    if (menu_stack_depth > 0)
    {
        menu_stack_depth--;
        page_first_unit = menu_stack[menu_stack_depth].page_first;
        current_unit = menu_stack[menu_stack_depth].current;
        current_line = menu_stack[menu_stack_depth].line;
        menu_refresh();
    }
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
            {
                float old_val = *(float *)current_unit->param_ptr;
                *(float *)current_unit->param_ptr += change;
                float new_val = *(float *)current_unit->param_ptr;
                printf("[MENU_EDIT] %s: %.2f -> %.2f (ptr=0x%08X)\r\n",
                       current_unit->name, old_val, new_val, (uint32)current_unit->param_ptr);
            }
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
 * @brief  刷新页面显示（全屏刷新）
 */
static void menu_refresh_page(void)
{
    ips114_clear();

    // 显示菜单项
    menu_unit_t *unit = page_first_unit;
    for (uint8 i = 0; i < MENU_ITEMS_PER_PAGE && unit != NULL; i++)
    {
        uint8 selected = (unit == current_unit) ? 1 : 0;
        menu_display_item(unit, i, selected, edit_mode && selected);
        unit = unit->down;

        // 防止循环链接导致重复显示：如果回到起点则停止
        if (unit == page_first_unit)
            break;
    }
}

/**
 * @brief  局部刷新（只刷新光标位置）
 */
static void menu_refresh_partial(void)
{
    // 清除上次选中行的光标
    menu_display_cursor(last_line, 0);

    // 显示当前选中行的光标
    menu_display_cursor(current_line, 1);
}

// ==================== 菜单核心 API 实现 ====================

/**
 * @brief  初始化菜单系统
 */
void menu_init(void)
{
    printf("[MENU] Initializing menu system...\r\n");

    // 初始化屏幕
    ips114_init();
    ips114_set_dir(IPS114_PORTAIT);
    ips114_set_color(MENU_COLOR_TEXT, MENU_COLOR_BG);
    ips114_clear();

    // 初始化按键
    menu_key_init(20);  // 20ms扫描周期

    // 初始化配置系统
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

    // 只有当enter不为NULL时才设置，避免覆盖auto_link设置的enter指针
    if (enter != NULL)
        current->enter = enter;

    current->back = back;
}

/**
 * @brief  自动链接辅助 - 静态变量（维护每个页面的子节点链表）
 */
#define MAX_PAGE_TRACK 20  // 最多跟踪20个页面
static struct {
    menu_unit_t* page;          // 页面指针
    menu_unit_t* first_child;   // 第一个子节点
    menu_unit_t* last_child;    // 最后一个子节点
} page_tracker[MAX_PAGE_TRACK] = {0};
static uint8 page_tracker_count = 0;

/**
 * @brief  自动链接子节点到父页面
 * @param  child: 子节点
 * @param  parent: 父页面
 * @note   自动维护每个页面的子节点链表，简化菜单创建代码
 */
void menu_auto_link_child(menu_unit_t* child, menu_unit_t* parent)
{
    if (child == NULL || parent == NULL)
        return;

    // 查找该页面是否已在跟踪表中
    int8 page_idx = -1;
    for (uint8 i = 0; i < page_tracker_count; i++)
    {
        if (page_tracker[i].page == parent)
        {
            page_idx = i;
            break;
        }
    }

    // 如果是新页面,添加到跟踪表
    if (page_idx == -1)
    {
        if (page_tracker_count >= MAX_PAGE_TRACK)
        {
            printf("[MENU] Error: 页面跟踪表已满\r\n");
            return;
        }
        page_idx = page_tracker_count;
        page_tracker[page_idx].page = parent;
        page_tracker[page_idx].first_child = NULL;
        page_tracker[page_idx].last_child = NULL;
        page_tracker_count++;
    }

    // 链接子节点
    if (page_tracker[page_idx].first_child == NULL)
    {
        // 第一个子节点
        page_tracker[page_idx].first_child = child;
        page_tracker[page_idx].last_child = child;

        // 链接: parent.enter -> child, child自环
        parent->enter = child;
        child->up = child;
        child->down = child;
        child->back = parent;
    }
    else
    {
        // 后续子节点
        menu_unit_t* last = page_tracker[page_idx].last_child;

        // 插入到链表尾部
        last->down = child;
        child->up = last;
        child->down = page_tracker[page_idx].first_child;
        child->back = parent;

        page_tracker[page_idx].first_child->up = child;
        page_tracker[page_idx].last_child = child;
    }
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
                // ⭐ 退出编辑模式前，检查是否是basic_speed参数
                if (current_unit == basic_speed_menu_unit)
                {
                    basic_speed = temp_basic_speed;  // 延迟更新：临时变量 → 真实变量
                    printf("[MENU] Basic Speed updated: %d (delayed update)\r\n", basic_speed);
                }
                
                edit_mode = 0;          // 退出编辑模式
                config_auto_save();     // 自动保存（掉电不丢失）
                menu_display_item(current_unit, current_line, 1, 0);  // 刷新为选中状态
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
    ips114_clear();
}

/**
 * @brief  检查菜单是否激活
 */
uint8 menu_is_active(void)
{
    return menu_active;
}

// ==================== 内置功能函数 ====================

/**
 * @brief  在屏幕底部显示提示信息
 */
static void menu_show_message(const char *msg)
{
    // 串口打印
    printf("[MENU] %s\r\n", msg);

    // 在屏幕底部显���提示（Y=112，字体高度16，范围112-127，不超过135）
    ips114_set_color(RGB565_GREEN, MENU_COLOR_BG);
    ips114_show_string(0, 112, (const char *)msg);
    ips114_set_color(MENU_COLOR_TEXT, MENU_COLOR_BG);  // 恢复默认颜色

    // 延迟800ms让用户看到提示
    system_delay_ms(800);

    // 清除提示信息（用背景色覆盖）
    ips114_set_color(MENU_COLOR_BG, MENU_COLOR_BG);
    ips114_show_string(0, 112, "                    ");  // 20个空格清除
    ips114_set_color(MENU_COLOR_TEXT, MENU_COLOR_BG);  // 恢复默认颜色
}

/**
 * @brief  保存配置到存档位1
 */
void menu_func_save_slot1(void)
{
    config_save_slot(0);
    menu_show_message("Saved to Slot 1");
}

/**
 * @brief  保存配置到存档位2
 */
void menu_func_save_slot2(void)
{
    config_save_slot(1);
    menu_show_message("Saved to Slot 2");
}

/**
 * @brief  保存配置到存档位3
 */
void menu_func_save_slot3(void)
{
    config_save_slot(2);
    menu_show_message("Saved to Slot 3");
}

/**
 * @brief  保存配置到存档位4
 */
void menu_func_save_slot4(void)
{
    config_save_slot(3);
    menu_show_message("Saved to Slot 4");
}

/**
 * @brief  从存档位1加载
 */
void menu_func_load_slot1(void)
{
    config_load_slot(0);
    temp_basic_speed = basic_speed;  // ⭐ 同步加载后的值到临时变量
    menu_show_message("Loaded from Slot 1");
    menu_refresh();
}

/**
 * @brief  从存档位2加载
 */
void menu_func_load_slot2(void)
{
    config_load_slot(1);
    temp_basic_speed = basic_speed;  // ⭐ 同步加载后的值到临时变量
    menu_show_message("Loaded from Slot 2");
    menu_refresh();
}

/**
 * @brief  从存档位3加载
 */
void menu_func_load_slot3(void)
{
    config_load_slot(2);
    temp_basic_speed = basic_speed;  // ⭐ 同步加载后的值到临时变量
    menu_show_message("Loaded from Slot 3");
    menu_refresh();
}

/**
 * @brief  从存档位4加载
 */
void menu_func_load_slot4(void)
{
    config_load_slot(3);
    temp_basic_speed = basic_speed;  // ⭐ 同步加载后的值到临时变量
    menu_show_message("Loaded from Slot 4");
    menu_refresh();
}

/**
 * @brief  实时显示图像（按返回键退出）
 */
void menu_func_show_image(void)
{
    // 清屏准备显示图像
    ips114_clear();

    uint32 frame_count = 0;
    uint8 exit_flag = 0;

    // 进入图像显示循环
    while(!exit_flag)
    {
        // 直接检测返回键的按下状态（不等待释放）
        key_scanner();  // 更新按键状态
        key_state_enum key4_state = key_get_state(KEY_4);

        // 按键按下时立即退出（不需要松开）
        if(key4_state == KEY_SHORT_PRESS || key4_state == KEY_LONG_PRESS)
        {
            exit_flag = 1;
            key_clear_state(KEY_4);  // 清除按键状态
            break;  // 立即退出循环
        }

        // 处理图像（如果有新图像）
        if(mt9v03x_finish_flag)
        {
            frame_count++;

            // 显示图像和边线（仅图像，无任何文字叠加）
            // MT9V03X: 188x120, IPS114: 240x135 → 不会越界
            image_process(MT9V03X_W, MT9V03X_H, 1);

            // 清除标志
            mt9v03x_finish_flag = 0;
        }

        // 短延时，避免CPU占用过高
        // 延时时间缩短，提高按键检测频率
        system_delay_ms(5);
    }

    // 退出时清屏并刷新菜单
    ips114_clear();
    menu_refresh();

}

/**
 * @brief  测试函数
 */
void menu_func_test(void)
{
    menu_show_message("Test Function");
    // TODO: 实现测试功能
}

/**
 * @brief  VOFA+调试开关（功能函数-已废弃，改用参数方式）
 */
void menu_func_speed_debug_toggle(void)
{
    speed_debug_enable = !speed_debug_enable;
}

void menu_func_direction_debug_toggle(void)
{
    direction_debug_enable = !direction_debug_enable;
}

/**
 * @brief  发车函数（菜单中调用）
 */
void menu_func_car_start(void)
{
    start_car();  // 调用control模块的发车函数
}

// ==================== 菜单示例 ====================

// 速度环参数使用pid.h中的全局变量: speed_kp, speed_ki, speed_kd
// 方向环参数使用pid.h中的全局变量: direction_kp, direction_ki, direction_kd
// 基础速度使用motor.h中的全局变量: basic_speed（通过temp_basic_speed延迟更新）
// 调试开关使用motor.h中的全局变量: speed_debug_enable, direction_debug_enable
// 差速系数使用motor.h中的全局变量: inner_wheel_ratio, outer_wheel_ratio
// 注意：temp_basic_speed和basic_speed_menu_unit已在文件前面定义

// 菜单单元指针
static menu_unit_t *menu_root = NULL;

/**
 * @brief  创建菜单示例
 */
void menu_example_create(void)
{
    // ========== 一级菜单 ==========
    menu_root = menu_create_unit("Main Menu", MENU_UNIT_PAGE);

    menu_unit_t *car_start = menu_create_function("Car_Start", menu_func_car_start);
    menu_unit_t *parameter = menu_create_unit("Parameter", MENU_UNIT_PAGE);
    menu_unit_t *save_config = menu_create_unit("Save_config", MENU_UNIT_PAGE);
    menu_unit_t *load_config = menu_create_unit("Load_config", MENU_UNIT_PAGE);
    menu_unit_t *debug = menu_create_unit("Debug", MENU_UNIT_PAGE);

    // ========== Parameter二级菜单 ==========
    menu_unit_t *direction_page = menu_create_unit("DirectionPID", MENU_UNIT_PAGE);  // 方向环
    menu_unit_t *speed_page = menu_create_unit("SpeedPID", MENU_UNIT_PAGE);          // 速度环
    menu_unit_t *image_page = menu_create_unit("Image", MENU_UNIT_PAGE);

    // ========== 方向环三级参数 (位置式PID+差速系数) ==========
    // 定义默认值
    static float direction_kp_default = 0.0f;
    static float direction_ki_default = 0.0f;
    static float direction_kd_default = 0.0f;
    static float inner_ratio_default = 1.0f;
    static float outer_ratio_default = 1.0f;
    
    // 创建菜单单元
    static menu_unit_t* direction_kp_unit = NULL;
    static menu_unit_t* direction_ki_unit = NULL;
    static menu_unit_t* direction_kd_unit = NULL;
    static menu_unit_t* inner_ratio_unit = NULL;
    static menu_unit_t* outer_ratio_unit = NULL;
    
    direction_kp_unit = menu_create_param("Dir Kp", &direction_kp, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    direction_ki_unit = menu_create_param("Dir Ki", &direction_ki, CONFIG_TYPE_FLOAT, 0.01f, 2, 3);
    direction_kd_unit = menu_create_param("Dir Kd", &direction_kd, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    inner_ratio_unit = menu_create_param("Inner Ratio", &inner_wheel_ratio, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    outer_ratio_unit = menu_create_param("Outer Ratio", &outer_wheel_ratio, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    
    // 注册到配置系统
    config_register_item("direction_kp", &direction_kp, CONFIG_TYPE_FLOAT, &direction_kp_default, "Dir Kp");
    config_register_item("direction_ki", &direction_ki, CONFIG_TYPE_FLOAT, &direction_ki_default, "Dir Ki");
    config_register_item("direction_kd", &direction_kd, CONFIG_TYPE_FLOAT, &direction_kd_default, "Dir Kd");
    config_register_item("inner_ratio", &inner_wheel_ratio, CONFIG_TYPE_FLOAT, &inner_ratio_default, "Inner Ratio");
    config_register_item("outer_ratio", &outer_wheel_ratio, CONFIG_TYPE_FLOAT, &outer_ratio_default, "Outer Ratio");
    
    // 链接到父页面
    menu_auto_link_child(direction_kp_unit, direction_page);
    menu_auto_link_child(direction_ki_unit, direction_page);
    menu_auto_link_child(direction_kd_unit, direction_page);
    menu_auto_link_child(inner_ratio_unit, direction_page);
    menu_auto_link_child(outer_ratio_unit, direction_page);

    // ========== 速度环三级参数 (左右轮独立PID+基础速度) ==========
    // 定义默认值
    static float speed_left_kp_default = 0.0f;
    static float speed_left_ki_default = 0.0f;
    static float speed_left_kd_default = 0.0f;
    static float speed_right_kp_default = 0.0f;
    static float speed_right_ki_default = 0.0f;
    static float speed_right_kd_default = 0.0f;
    static int16 basic_speed_default = 100;
    
    // 初始化临时变量（从motor.h的全局变量读取当前值）
    temp_basic_speed = basic_speed;

    // 创建菜单单元（左轮）
    static menu_unit_t* speed_left_kp_unit = NULL;
    static menu_unit_t* speed_left_ki_unit = NULL;
    static menu_unit_t* speed_left_kd_unit = NULL;
    
    // 创建菜单单元（右轮）
    static menu_unit_t* speed_right_kp_unit = NULL;
    static menu_unit_t* speed_right_ki_unit = NULL;
    static menu_unit_t* speed_right_kd_unit = NULL;

    // 左轮PID参数
    speed_left_kp_unit = menu_create_param("Left Kp", &speed_left_kp, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    speed_left_ki_unit = menu_create_param("Left Ki", &speed_left_ki, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    speed_left_kd_unit = menu_create_param("Left Kd", &speed_left_kd, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    
    // 右轮PID参数
    speed_right_kp_unit = menu_create_param("Right Kp", &speed_right_kp, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    speed_right_ki_unit = menu_create_param("Right Ki", &speed_right_ki, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    speed_right_kd_unit = menu_create_param("Right Kd", &speed_right_kd, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    
    // ⭐ basic_speed使用临时变量，退出编辑时才更新真实值
    basic_speed_menu_unit = menu_create_param("Basic Speed", &temp_basic_speed, CONFIG_TYPE_INT16, 10.0f, 3, 0);

    // ========== 注册到配置系统（替换旧参数为左右轮独立参数）==========
    // 注意：配置项顺序必须与旧版本保持一致，新参数放在最后
    // 旧Flash顺序：direction(5项) → speed_kp/ki/kd(3项) → basic_speed(1项) → image(2项)
    // 新顺序：direction(5项) → speed_left_kp/ki/kd(3项，替换旧speed) → basic_speed(1项) → image(2项) → speed_right(3项，新增)
    
    config_register_item("speed_left_kp", &speed_left_kp, CONFIG_TYPE_FLOAT, &speed_left_kp_default, "Left Kp");
    config_register_item("speed_left_ki", &speed_left_ki, CONFIG_TYPE_FLOAT, &speed_left_ki_default, "Left Ki");
    config_register_item("speed_left_kd", &speed_left_kd, CONFIG_TYPE_FLOAT, &speed_left_kd_default, "Left Kd");
    config_register_item("basic_speed", &basic_speed, CONFIG_TYPE_INT16, &basic_speed_default, "Basic Speed");

    // 链接到父页面（顺序：左轮Kp/Ki/Kd → 右轮Kp/Ki/Kd → 基础速度）
    menu_auto_link_child(speed_left_kp_unit, speed_page);
    menu_auto_link_child(speed_left_ki_unit, speed_page);
    menu_auto_link_child(speed_left_kd_unit, speed_page);
    menu_auto_link_child(speed_right_kp_unit, speed_page);
    menu_auto_link_child(speed_right_ki_unit, speed_page);
    menu_auto_link_child(speed_right_kd_unit, speed_page);
    menu_auto_link_child(basic_speed_menu_unit, speed_page);

    // ========== Image三级参数 ==========
    // mid_weight_select: 权重数组选择（1-5）
    static uint16 mid_weight_select_default = 1;
    static menu_unit_t* mid_weight_select_unit = NULL;
    mid_weight_select_unit = menu_create_param("Weight Select", &mid_weight_select, CONFIG_TYPE_UINT16, 1.0f, 1, 0);
    config_register_item("mid_weight_select", &mid_weight_select, CONFIG_TYPE_UINT16, &mid_weight_select_default, "Weight Select");
    menu_auto_link_child(mid_weight_select_unit, image_page);
    
    // cross_enable: 十字识别开关（0=关闭，1=开启）
    static uint16 cross_enable_default = 0;
    static menu_unit_t* cross_enable_unit = NULL;
    cross_enable_unit = menu_create_param("Cross Enable", &cross_enable, CONFIG_TYPE_UINT16, 1.0f, 1, 0);
    config_register_item("cross_enable", &cross_enable, CONFIG_TYPE_UINT16, &cross_enable_default, "Cross Enable");
    menu_auto_link_child(cross_enable_unit, image_page);

    // ========== 右轮参数（新增，放在最后保持兼容性）==========
    // 注册右轮PID参数到配置系统（新增配置项，放在最后）
    config_register_item("speed_right_kp", &speed_right_kp, CONFIG_TYPE_FLOAT, &speed_right_kp_default, "Right Kp");
    config_register_item("speed_right_ki", &speed_right_ki, CONFIG_TYPE_FLOAT, &speed_right_ki_default, "Right Ki");
    config_register_item("speed_right_kd", &speed_right_kd, CONFIG_TYPE_FLOAT, &speed_right_kd_default, "Right Kd");

    // ========== Save_config二级菜单 ==========
    menu_unit_t *save_slot1 = menu_create_function("Slot 1", menu_func_save_slot1);
    menu_unit_t *save_slot2 = menu_create_function("Slot 2", menu_func_save_slot2);
    menu_unit_t *save_slot3 = menu_create_function("Slot 3", menu_func_save_slot3);
    menu_unit_t *save_slot4 = menu_create_function("Slot 4", menu_func_save_slot4);

    // ========== Load_config二级菜单 ==========
    menu_unit_t *load_slot1 = menu_create_function("Slot 1", menu_func_load_slot1);
    menu_unit_t *load_slot2 = menu_create_function("Slot 2", menu_func_load_slot2);
    menu_unit_t *load_slot3 = menu_create_function("Slot 3", menu_func_load_slot3);
    menu_unit_t *load_slot4 = menu_create_function("Slot 4", menu_func_load_slot4);

    // ========== Debug二级菜单 ==========
    menu_unit_t *debug_show_image = menu_create_function("Show Image", menu_func_show_image);
    menu_unit_t *debug_test = menu_create_function("Test Function", menu_func_test);
    menu_unit_t *debug_speed_loop = menu_create_param("SpeedDebug", &speed_debug_enable, CONFIG_TYPE_UINT8, 1.0f, 1, 0);
    menu_unit_t *debug_direction_loop = menu_create_param("DirectionDebug", &direction_debug_enable, CONFIG_TYPE_UINT8, 1.0f, 1, 0);

    // ==================== 链接菜单 ====================

    // 一级菜单链接（循环），back为NULL表示顶层菜单
    menu_link(menu_root, NULL, car_start, car_start, NULL);
    menu_link(car_start, debug, parameter, NULL, NULL);          // 循环：上一个是debug，无back
    menu_link(parameter, car_start, save_config, direction_page, NULL);  // enter进入方向环页
    menu_link(save_config, parameter, load_config, save_slot1, NULL);
    menu_link(load_config, save_config, debug, load_slot1, NULL);
    menu_link(debug, load_config, car_start, debug_show_image, NULL);  // 循环：下一个是car_start，无back

    // Parameter二级菜单链接（循环）
    menu_link(direction_page, image_page, speed_page, NULL, parameter);  // 循环：方向环页
    menu_link(speed_page, direction_page, image_page, NULL, parameter);  // 速度环页
    menu_link(image_page, speed_page, direction_page, NULL, parameter);  // 循环：图像页

    // 三级参数链接已由MENU_ADD_PARAM_AUTO自动完成,无需手动链接!

    // Save_config二级菜单链接（循环）
    menu_link(save_slot1, save_slot4, save_slot2, NULL, save_config);        // 循环
    menu_link(save_slot2, save_slot1, save_slot3, NULL, save_config);
    menu_link(save_slot3, save_slot2, save_slot4, NULL, save_config);
    menu_link(save_slot4, save_slot3, save_slot1, NULL, save_config);        // 循环

    // Load_config二级菜单链接（循环）
    menu_link(load_slot1, load_slot4, load_slot2, NULL, load_config);        // 循环
    menu_link(load_slot2, load_slot1, load_slot3, NULL, load_config);
    menu_link(load_slot3, load_slot2, load_slot4, NULL, load_config);
    menu_link(load_slot4, load_slot3, load_slot1, NULL, load_config);        // 循环

    // Debug二级菜单链接（循环）
    menu_link(debug_show_image, debug_direction_loop, debug_test, NULL, debug);           // 循环
    menu_link(debug_test, debug_show_image, debug_speed_loop, NULL, debug);
    menu_link(debug_speed_loop, debug_test, debug_direction_loop, NULL, debug);
    menu_link(debug_direction_loop, debug_speed_loop, debug_show_image, NULL, debug);     // 循环

}

/**
 * @brief  进入菜单示例
 */
void menu_example_enter(void)
{
    // 进入一级菜单，从car_start开始
    if (menu_root != NULL && menu_root->down != NULL)
    {
        menu_unit_t *car_start = menu_root->down;
        current_unit = car_start;
        page_first_unit = car_start;
        current_line = 0;
        edit_mode = 0;
        menu_active = 1;
        menu_refresh();
    }
}

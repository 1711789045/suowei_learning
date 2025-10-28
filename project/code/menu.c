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

    // 显示名称
    ips114_show_string(10, y, unit->name);

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

    // 保存旧的光标位置
    uint8 old_line = current_line;

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

    // 只刷新光标位置（局部刷新）
    menu_display_cursor(old_line, 0);       // 清除旧光标
    menu_display_cursor(current_line, 1);   // 显示新光标
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

    // 保存旧的光标位置
    uint8 old_line = current_line;

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

    // 只刷新光标位置（局部刷新）
    menu_display_cursor(old_line, 0);       // 清除旧光标
    menu_display_cursor(current_line, 1);   // 显示新光标
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
 * @brief  在屏幕底部显示提示信息（非阻塞版本）
 */
static void menu_show_message(const char *msg)
{
    // 直接在串口打印，不再屏幕显示（避免阻塞和刷新问题）
    printf("[MENU] %s\r\n", msg);

    // 如果需要屏幕提示，可以在menu_refresh时显示状态栏
    // 这里先简化，只用串口提示
}

/**
 * @brief  发车函数
 */
void menu_func_car_start(void)
{
    menu_show_message("Car Start!");
    printf("[CAR] Car started!\r\n");
    // TODO: 实现发车逻辑
}

/**
 * @brief  保存配置到存档位1
 */
void menu_func_save_slot1(void)
{
    config_save_slot(0);
    menu_show_message("Saved to Slot 1");
    printf("[MENU] Config saved to slot 1\r\n");
}

/**
 * @brief  保存配置到存档位2
 */
void menu_func_save_slot2(void)
{
    config_save_slot(1);
    menu_show_message("Saved to Slot 2");
    printf("[MENU] Config saved to slot 2\r\n");
}

/**
 * @brief  保存配置到存档位3
 */
void menu_func_save_slot3(void)
{
    config_save_slot(2);
    menu_show_message("Saved to Slot 3");
    printf("[MENU] Config saved to slot 3\r\n");
}

/**
 * @brief  保存配置到存档位4
 */
void menu_func_save_slot4(void)
{
    config_save_slot(3);
    menu_show_message("Saved to Slot 4");
    printf("[MENU] Config saved to slot 4\r\n");
}

/**
 * @brief  从存档位1加载
 */
void menu_func_load_slot1(void)
{
    config_load_slot(0);
    menu_show_message("Loaded from Slot 1");
    printf("[MENU] Config loaded from slot 1\r\n");
    menu_refresh();
}

/**
 * @brief  从存档位2加载
 */
void menu_func_load_slot2(void)
{
    config_load_slot(1);
    menu_show_message("Loaded from Slot 2");
    printf("[MENU] Config loaded from slot 2\r\n");
    menu_refresh();
}

/**
 * @brief  从存档位3加载
 */
void menu_func_load_slot3(void)
{
    config_load_slot(2);
    menu_show_message("Loaded from Slot 3");
    printf("[MENU] Config loaded from slot 3\r\n");
    menu_refresh();
}

/**
 * @brief  从存档位4加载
 */
void menu_func_load_slot4(void)
{
    config_load_slot(3);
    menu_show_message("Loaded from Slot 4");
    printf("[MENU] Config loaded from slot 4\r\n");
    menu_refresh();
}

/**
 * @brief  显示图像
 */
void menu_func_show_image(void)
{
    menu_show_message("Show Image");
    printf("[DEBUG] Show image\r\n");
    // TODO: 实现图像显示
}

/**
 * @brief  测试函数
 */
void menu_func_test(void)
{
    menu_show_message("Test Function");
    printf("[DEBUG] Test function\r\n");
    // TODO: 实现测试功能
}

/**
 * @brief  VOFA+调试开关（功能函数-已废弃，改用参数方式）
 */
void menu_func_vofa_toggle(void)
{
    motor_vofa_enable = !motor_vofa_enable;
    printf("[DEBUG] VOFA+ %s\r\n", motor_vofa_enable ? "ON" : "OFF");
}

// ==================== 菜单示例 ====================

// 舵机参数
static float servo_center = 1500.0f;
static float servo_left_max = 1800.0f;
static float servo_right_max = 1200.0f;
static float servo_kp = 1.0f;

// 电机参数使用pid.h中的全局变量: motor_kp, motor_ki, motor_kd
// VOFA+开关使用motor.h中的全局变量: motor_vofa_enable

// 图像参数
static uint16 image_threshold = 128;
static uint16 image_exposure = 100;
static float image_gain = 1.0f;

// 菜单单元指针
static menu_unit_t *menu_root = NULL;

/**
 * @brief  创建菜单示例
 */
void menu_example_create(void)
{
    // 注册参数到配置系统
    config_register_item("servo_center", &servo_center, CONFIG_TYPE_FLOAT, &servo_center, "Servo Center");
    config_register_item("servo_left_max", &servo_left_max, CONFIG_TYPE_FLOAT, &servo_left_max, "Servo Left Max");
    config_register_item("servo_right_max", &servo_right_max, CONFIG_TYPE_FLOAT, &servo_right_max, "Servo Right Max");
    config_register_item("servo_kp", &servo_kp, CONFIG_TYPE_FLOAT, &servo_kp, "Servo Kp");

    config_register_item("motor_kp", &motor_kp, CONFIG_TYPE_FLOAT, &motor_kp, "Motor Kp");
    config_register_item("motor_ki", &motor_ki, CONFIG_TYPE_FLOAT, &motor_ki, "Motor Ki");
    config_register_item("motor_kd", &motor_kd, CONFIG_TYPE_FLOAT, &motor_kd, "Motor Kd");
    config_register_item("motor_vofa_enable", &motor_vofa_enable, CONFIG_TYPE_UINT8, &motor_vofa_enable, "VOFA Enable");

    config_register_item("image_threshold", &image_threshold, CONFIG_TYPE_UINT16, &image_threshold, "Image Threshold");
    config_register_item("image_exposure", &image_exposure, CONFIG_TYPE_UINT16, &image_exposure, "Image Exposure");
    config_register_item("image_gain", &image_gain, CONFIG_TYPE_FLOAT, &image_gain, "Image Gain");

    // ========== 一级菜单 ==========
    menu_root = menu_create_unit("Main Menu", MENU_UNIT_PAGE);

    menu_unit_t *car_start = menu_create_function("Car_Start", menu_func_car_start);
    menu_unit_t *parameter = menu_create_unit("Parameter", MENU_UNIT_PAGE);
    menu_unit_t *save_config = menu_create_unit("Save_config", MENU_UNIT_PAGE);
    menu_unit_t *load_config = menu_create_unit("Load_config", MENU_UNIT_PAGE);
    menu_unit_t *debug = menu_create_unit("Debug", MENU_UNIT_PAGE);

    // ========== Parameter二级菜单 ==========
    menu_unit_t *servo_page = menu_create_unit("Servo", MENU_UNIT_PAGE);
    menu_unit_t *motor_page = menu_create_unit("Motor", MENU_UNIT_PAGE);
    menu_unit_t *image_page = menu_create_unit("Image", MENU_UNIT_PAGE);

    // ========== Servo三级参数 ==========
    menu_unit_t *servo_param1 = menu_create_param("servo_center", &servo_center, CONFIG_TYPE_FLOAT, 10.0f, 4, 0);
    menu_unit_t *servo_param2 = menu_create_param("left_max", &servo_left_max, CONFIG_TYPE_FLOAT, 10.0f, 4, 0);
    menu_unit_t *servo_param3 = menu_create_param("right_max", &servo_right_max, CONFIG_TYPE_FLOAT, 10.0f, 4, 0);
    menu_unit_t *servo_param4 = menu_create_param("servo_kp", &servo_kp, CONFIG_TYPE_FLOAT, 0.1f, 2, 1);

    // ========== Motor三级参数 ==========
    menu_unit_t *motor_param1 = menu_create_param("motor_kp", &motor_kp, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    menu_unit_t *motor_param2 = menu_create_param("motor_ki", &motor_ki, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    menu_unit_t *motor_param3 = menu_create_param("motor_kd", &motor_kd, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);

    // ========== Image三级参数 ==========
    menu_unit_t *image_param1 = menu_create_param("threshold", &image_threshold, CONFIG_TYPE_UINT16, 5.0f, 3, 0);
    menu_unit_t *image_param2 = menu_create_param("exposure", &image_exposure, CONFIG_TYPE_UINT16, 10.0f, 3, 0);
    menu_unit_t *image_param3 = menu_create_param("gain", &image_gain, CONFIG_TYPE_FLOAT, 0.1f, 1, 1);

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
    menu_unit_t *debug_vofa = menu_create_param("VOFA Debug", &motor_vofa_enable, CONFIG_TYPE_UINT8, 1.0f, 0, 0);

    // ==================== 链接菜单 ====================

    // 一级菜单链接（循环），back为NULL表示顶层菜单
    menu_link(menu_root, NULL, car_start, car_start, NULL);
    menu_link(car_start, debug, parameter, NULL, NULL);          // 循环：上一个是debug，无back
    menu_link(parameter, car_start, save_config, servo_page, NULL);
    menu_link(save_config, parameter, load_config, save_slot1, NULL);
    menu_link(load_config, save_config, debug, load_slot1, NULL);
    menu_link(debug, load_config, car_start, debug_show_image, NULL);  // 循环：下一个是car_start，无back

    // Parameter二级菜单链接（循环）
    menu_link(servo_page, image_page, motor_page, servo_param1, parameter);  // 循环：上一个是image_page
    menu_link(motor_page, servo_page, image_page, motor_param1, parameter);
    menu_link(image_page, motor_page, servo_page, image_param1, parameter);  // 循环：下一个是servo_page

    // Servo三级参数链接（循环）
    menu_link(servo_param1, servo_param4, servo_param2, NULL, servo_page);   // 循环
    menu_link(servo_param2, servo_param1, servo_param3, NULL, servo_page);
    menu_link(servo_param3, servo_param2, servo_param4, NULL, servo_page);
    menu_link(servo_param4, servo_param3, servo_param1, NULL, servo_page);   // 循环

    // Motor三级参数链接（循环）
    menu_link(motor_param1, motor_param3, motor_param2, NULL, motor_page);   // 循环
    menu_link(motor_param2, motor_param1, motor_param3, NULL, motor_page);
    menu_link(motor_param3, motor_param2, motor_param1, NULL, motor_page);   // 循环

    // Image三级参数链接（循环）
    menu_link(image_param1, image_param3, image_param2, NULL, image_page);   // 循环
    menu_link(image_param2, image_param1, image_param3, NULL, image_page);
    menu_link(image_param3, image_param2, image_param1, NULL, image_page);   // 循环

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
    menu_link(debug_show_image, debug_vofa, debug_test, NULL, debug);        // 循环
    menu_link(debug_test, debug_show_image, debug_vofa, NULL, debug);
    menu_link(debug_vofa, debug_test, debug_show_image, NULL, debug);        // 循环

    printf("[MENU_EXAMPLE] Three-level menu created\r\n");
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

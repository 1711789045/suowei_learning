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
* 文件名称          menu_example
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-10-26       Claude             菜单系统使用示例
********************************************************************************************************************/

#include "menu_example.h"
#include "menu_core.h"

// ==================== 示例参数 ====================
static float test_speed = 100.0f;
static float test_kp = 5.0f;
static int test_mode = 0;
static uint16 test_value = 1000;

// ==================== 菜单单元指针 ====================
static menu_unit_t *menu_root = NULL;

/**
 * @brief  创建菜单示例
 */
void menu_example_create(void)
{
    // 注册参数到配置系统
    config_register_item("test_speed", &test_speed, CONFIG_TYPE_FLOAT, &test_speed, "Test Speed");
    config_register_item("test_kp", &test_kp, CONFIG_TYPE_FLOAT, &test_kp, "Test Kp");
    config_register_item("test_mode", &test_mode, CONFIG_TYPE_INT, &test_mode, "Test Mode");
    config_register_item("test_value", &test_value, CONFIG_TYPE_UINT16, &test_value, "Test Value");

    // 创建主菜单
    menu_root = menu_create_unit("Main Menu", MENU_UNIT_PAGE);

    // 创建参数页面
    menu_unit_t *param_page = menu_create_unit("Parameters", MENU_UNIT_PAGE);
    menu_unit_t *param1 = menu_create_param("Speed", &test_speed, CONFIG_TYPE_FLOAT, 10.0f, 3, 1);
    menu_unit_t *param2 = menu_create_param("Kp", &test_kp, CONFIG_TYPE_FLOAT, 0.1f, 2, 2);
    menu_unit_t *param3 = menu_create_param("Mode", &test_mode, CONFIG_TYPE_INT, 1.0f, 2, 0);
    menu_unit_t *param4 = menu_create_param("Value", &test_value, CONFIG_TYPE_UINT16, 100.0f, 4, 0);

    // 创建功能页面
    menu_unit_t *func_page = menu_create_unit("Functions", MENU_UNIT_PAGE);
    menu_unit_t *func_save = menu_create_function("Save Config", menu_func_save_config);
    menu_unit_t *func_load = menu_create_function("Load Config", menu_func_load_config);
    menu_unit_t *func_reset = menu_create_function("Reset Default", menu_func_reset_default);

    // 链接主菜单
    menu_link(menu_root, NULL, param_page, param_page, NULL);
    menu_link(param_page, menu_root, func_page, param1, menu_root);
    menu_link(func_page, param_page, NULL, func_save, menu_root);

    // 链接参数页面
    menu_link(param1, NULL, param2, NULL, param_page);
    menu_link(param2, param1, param3, NULL, param_page);
    menu_link(param3, param2, param4, NULL, param_page);
    menu_link(param4, param3, NULL, NULL, param_page);

    // 链接功能页面
    menu_link(func_save, NULL, func_load, NULL, func_page);
    menu_link(func_load, func_save, func_reset, NULL, func_page);
    menu_link(func_reset, func_load, NULL, NULL, func_page);

    printf("[MENU_EXAMPLE] Menu created\r\n");
}

/**
 * @brief  进入菜单示例
 */
void menu_example_enter(void)
{
    if (menu_root != NULL)
    {
        menu_enter(menu_root);
    }
}

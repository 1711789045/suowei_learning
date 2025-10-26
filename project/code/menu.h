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

#ifndef _MENU_H_
#define _MENU_H_

#include "zf_common_headfile.h"
#include "key.h"
#include "flash.h"

// ==================== 屏幕参数（IPS114: 240x135）====================
#define MENU_SCREEN_W           240             // 屏幕宽度
#define MENU_SCREEN_H           135             // 屏幕高度
#define MENU_FONT_H             16              // 字体高度（8x16字体）
#define MENU_ITEMS_PER_PAGE     8               // 每页显示的参数数量(去掉标题后可显示8行)

#define MENU_ITEM_START_Y       0               // 第一个菜单项Y坐标(从顶部开始)

// 颜色定义
#define MENU_COLOR_BG           RGB565_BLACK    // 背景色
#define MENU_COLOR_TEXT         RGB565_WHITE    // 文本颜色
#define MENU_COLOR_SELECT       RGB565_BLUE     // 选中项颜色
#define MENU_COLOR_EDIT         RGB565_RED      // 编辑模式颜色

// ==================== 菜单配置 ====================
#define MENU_MAX_UNITS      100         // 最大菜单单元数量

// ==================== 菜单单元类型 ====================
typedef enum {
    MENU_UNIT_NORMAL = 0,       // 普通参数
    MENU_UNIT_FUNCTION,         // 功能函数
    MENU_UNIT_PAGE              // 页面入口
} menu_unit_type_e;

// ==================== 菜单单元结构体 ====================
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

// ==================== 菜单核心 API ====================

/**
 * @brief  初始化菜单系统
 * @param  无
 * @return 无
 * @note   在main函数中调用，初始化所有菜单模块
 */
void menu_init(void);

/**
 * @brief  创建菜单单元
 * @param  name: 显示名称
 * @param  type: 单元类型
 * @return 菜单单元指针
 */
menu_unit_t* menu_create_unit(const char *name, menu_unit_type_e type);

/**
 * @brief  创建参数单元
 * @param  name: 显示名称
 * @param  param_ptr: 参数指针
 * @param  param_type: 参数类型
 * @param  delta: 调整增量
 * @param  num_int: 整数位数
 * @param  num_dec: 小数位数
 * @return 菜单单元指针
 */
menu_unit_t* menu_create_param(const char *name, void *param_ptr, config_type_e param_type,
                                float delta, uint8 num_int, uint8 num_dec);

/**
 * @brief  创建功能单元
 * @param  name: 显示名称
 * @param  function: 功能函数指针
 * @return 菜单单元指针
 */
menu_unit_t* menu_create_function(const char *name, void (*function)(void));

/**
 * @brief  链接菜单单元
 * @param  current: 当前单元
 * @param  up: 上一项
 * @param  down: 下一项
 * @param  enter: 进入项
 * @param  back: 返回项
 * @return 无
 */
void menu_link(menu_unit_t *current, menu_unit_t *up, menu_unit_t *down,
               menu_unit_t *enter, menu_unit_t *back);

/**
 * @brief  菜单处理函数
 * @param  无
 * @return 无
 * @note   在主循环或定时器中周期调用（建议20ms）
 */
void menu_process(void);

/**
 * @brief  刷新菜单显示
 * @param  无
 * @return 无
 */
void menu_refresh(void);

/**
 * @brief  进入菜单
 * @param  root: 根菜单单元
 * @return 无
 */
void menu_enter(menu_unit_t *root);

/**
 * @brief  退出菜单
 * @param  无
 * @return 无
 */
void menu_exit(void);

/**
 * @brief  检查菜单是否激活
 * @param  无
 * @return 0=未激活, 1=已激活
 */
uint8 menu_is_active(void);

// ==================== 内置功能函数 ====================

/**
 * @brief  保存配置到存档位
 * @param  无
 * @return 无
 */
void menu_func_save_config(void);

/**
 * @brief  加载配置从存档位
 * @param  无
 * @return 无
 */
void menu_func_load_config(void);

/**
 * @brief  重置为默认值
 * @param  无
 * @return 无
 */
void menu_func_reset_default(void);

// ==================== 菜单示例 API ====================

/**
 * @brief  创建菜单示例
 * @param  无
 * @return 无
 */
void menu_example_create(void);

/**
 * @brief  进入菜单示例
 * @param  无
 * @return 无
 */
void menu_example_enter(void);

#endif

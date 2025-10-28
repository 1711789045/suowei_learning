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
* 文件名称          flash
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-10-26       Claude             Flash配置管理模块，支持4存档位
********************************************************************************************************************/

#ifndef _FLASH_H_
#define _FLASH_H_

#include "zf_common_headfile.h"

// ==================== 配置参数 ====================
#define CONFIG_MAX_ITEMS        50              // 最大支持的配置项数量
#define CONFIG_SAVE_SLOT_COUNT  4               // 存档位数量

#define CONFIG_FLASH_SECTION    0               // Flash扇区索引
#define CONFIG_FLASH_PAGE_BASE  0               // Flash页起始索引（Slot 0-3使用Page 0-3）
#define CONFIG_FLASH_PAGE_AUTO  4               // 自动保存页面（掉电不丢失，Page 4）
#define CONFIG_MAGIC_NUMBER     0x4D454E55      // 魔数 'MENU'，用于检测Flash是否已初始化

// ==================== 数据类型 ====================

// 配置项类型枚举
typedef enum {
    CONFIG_TYPE_FLOAT = 0,
    CONFIG_TYPE_DOUBLE,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_INT16,
    CONFIG_TYPE_INT8,
    CONFIG_TYPE_UINT32,
    CONFIG_TYPE_UINT16,
    CONFIG_TYPE_UINT8
} config_type_e;

// 配置项结构体
typedef struct {
    char key[32];           // 参数名
    void *var_ptr;          // 变量指针
    config_type_e type;     // 变量类型
    union {
        float f;
        double d;
        int i;
        int16 i16;
        int8 i8;
        uint32 u32;
        uint16 u16;
        uint8 u8;
    } default_val;          // 默认值
    char comment[32];       // 注释/说明
} config_item_t;

// ==================== API 函数 ====================

/**
 * @brief  初始化配置系统
 * @param  无
 * @return 无
 * @note   在 main 函数中程序启动时调用一次
 *         会自动初始化Flash并加载默认存档位(0)
 *         如果Flash未初始化，会创建默认配置
 */
void config_init(void);

/**
 * @brief  注册配置项
 * @param  key: 参数名（用于菜单显示）
 * @param  var_ptr: 变量指针
 * @param  type: 变量类型
 * @param  default_val: 默认值
 * @param  comment: 注释说明
 * @return 无
 */
void config_register_item(const char *key, void *var_ptr, config_type_e type, void *default_val, const char *comment);

/**
 * @brief  保存所有配置到指定存档位
 * @param  slot: 存档位编号 (0-3)
 * @return 0=成功, 其他=失败
 * @note   保存所有已注册的变量到Flash指定存档位
 */
uint8 config_save_slot(uint8 slot);

/**
 * @brief  从指定存档位加载配置
 * @param  slot: 存档位编号 (0-3)
 * @return 0=成功, 其他=失败
 * @note   从Flash指定存档位加载所有已注册的变量
 *         如果存档位未初始化，会加载默认值
 */
uint8 config_load_slot(uint8 slot);

/**
 * @brief  检查指定存档位是否已初始化
 * @param  slot: 存档位编号 (0-3)
 * @return 0=未初始化, 1=已初始化
 */
uint8 config_check_slot(uint8 slot);

/**
 * @brief  自动保存当前配置（掉电不丢失）
 * @param  无
 * @return 0=成功, 其他=失败
 * @note   保存到固定的Flash页面（Page 4），退出编辑模式时自动调用
 */
uint8 config_auto_save(void);

/**
 * @brief  自动加载配置（掉电不丢失）
 * @param  无
 * @return 0=成功, 其他=失败
 * @note   从固定的Flash页面（Page 4）加载，开机时自动调用
 *         如果未初始化，会加载默认值
 */
uint8 config_auto_load(void);

/**
 * @brief  重置所有变量为默认值
 * @param  无
 * @return 无
 */
void config_reset_default(void);

/**
 * @brief  获取已注册的配置项数量
 * @param  无
 * @return 配置项数量
 */
uint8 config_get_item_count(void);

/**
 * @brief  获取指定索引的配置项
 * @param  index: 配置项索引
 * @return 配置项指针，如果索引无效返回NULL
 */
config_item_t* config_get_item(uint8 index);

#endif

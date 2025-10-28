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

#include "flash.h"
#include <string.h>

// ==================== 内部数据 ====================
static config_item_t config_items[CONFIG_MAX_ITEMS];   // 配置项数组
static uint8 config_item_count = 0;                    // 已注册的配置项数量

// ==================== 内部函数声明 ====================
static uint32 config_get_flash_page(uint8 slot);
static void config_write_to_buffer(void);
static void config_read_from_buffer(void);

// ==================== 内部函数实现 ====================

/**
 * @brief  获取存档位对应的Flash页号
 * @param  slot: 存档位编号 (0-3)
 * @return Flash页号
 */
static uint32 config_get_flash_page(uint8 slot)
{
    if (slot >= CONFIG_SAVE_SLOT_COUNT)
        slot = 0;  // 超出范围时使用slot 0

    return CONFIG_FLASH_PAGE_BASE + slot;
}

/**
 * @brief  将所有配置项写入Flash缓冲区
 * @param  无
 * @return 无
 */
static void config_write_to_buffer(void)
{
    uint32 index = 0;

    // 写入魔数
    flash_union_buffer[index++].uint32_type = CONFIG_MAGIC_NUMBER;

    // 写入配置项数量
    flash_union_buffer[index++].uint32_type = config_item_count;

    // 写入所有配置项的值
    for (uint8 i = 0; i < config_item_count; i++)
    {
        config_item_t *item = &config_items[i];

        switch (item->type)
        {
            case CONFIG_TYPE_FLOAT:
                flash_union_buffer[index++].float_type = *(float *)item->var_ptr;
                break;
            case CONFIG_TYPE_DOUBLE:
                // double占用2个uint32空间
                {
                    double val = *(double *)item->var_ptr;
                    uint32 *p = (uint32 *)&val;
                    flash_union_buffer[index++].uint32_type = p[0];
                    flash_union_buffer[index++].uint32_type = p[1];
                }
                break;
            case CONFIG_TYPE_INT:
                flash_union_buffer[index++].int32_type = *(int *)item->var_ptr;
                break;
            case CONFIG_TYPE_INT16:
                flash_union_buffer[index++].int16_type = *(int16 *)item->var_ptr;
                break;
            case CONFIG_TYPE_INT8:
                flash_union_buffer[index++].int8_type = *(int8 *)item->var_ptr;
                break;
            case CONFIG_TYPE_UINT32:
                flash_union_buffer[index++].uint32_type = *(uint32 *)item->var_ptr;
                break;
            case CONFIG_TYPE_UINT16:
                flash_union_buffer[index++].uint16_type = *(uint16 *)item->var_ptr;
                break;
            case CONFIG_TYPE_UINT8:
                flash_union_buffer[index++].uint8_type = *(uint8 *)item->var_ptr;
                break;
        }
    }
}

/**
 * @brief  从Flash缓冲区读取所有配置项
 * @param  无
 * @return 无
 */
static void config_read_from_buffer(void)
{
    uint32 index = 0;

    // 跳过魔数
    uint32 magic = flash_union_buffer[index++].uint32_type;
    if (magic != CONFIG_MAGIC_NUMBER)
    {
        // Flash未初始化，加载默认值
        config_reset_default();
        return;
    }

    // 读取配置项数量
    uint32 saved_count = flash_union_buffer[index++].uint32_type;
    if (saved_count != config_item_count)
    {
        // 配置项数量不匹配，可能版本不同，使用默认值
        printf("[CONFIG] Item count mismatch (saved=%d, current=%d), using defaults\r\n",
               saved_count, config_item_count);
        config_reset_default();
        return;
    }

    // 读取所有配置项的值
    for (uint8 i = 0; i < config_item_count; i++)
    {
        config_item_t *item = &config_items[i];

        switch (item->type)
        {
            case CONFIG_TYPE_FLOAT:
                *(float *)item->var_ptr = flash_union_buffer[index++].float_type;
                break;
            case CONFIG_TYPE_DOUBLE:
                // double占用2个uint32空间
                {
                    double val;
                    uint32 *p = (uint32 *)&val;
                    p[0] = flash_union_buffer[index++].uint32_type;
                    p[1] = flash_union_buffer[index++].uint32_type;
                    *(double *)item->var_ptr = val;
                }
                break;
            case CONFIG_TYPE_INT:
                *(int *)item->var_ptr = flash_union_buffer[index++].int32_type;
                break;
            case CONFIG_TYPE_INT16:
                *(int16 *)item->var_ptr = flash_union_buffer[index++].int16_type;
                break;
            case CONFIG_TYPE_INT8:
                *(int8 *)item->var_ptr = flash_union_buffer[index++].int8_type;
                break;
            case CONFIG_TYPE_UINT32:
                *(uint32 *)item->var_ptr = flash_union_buffer[index++].uint32_type;
                break;
            case CONFIG_TYPE_UINT16:
                *(uint16 *)item->var_ptr = flash_union_buffer[index++].uint16_type;
                break;
            case CONFIG_TYPE_UINT8:
                *(uint8 *)item->var_ptr = flash_union_buffer[index++].uint8_type;
                break;
        }
    }
}

// ==================== API函数实现 ====================

/**
 * @brief  注册配置项
 */
void config_register_item(const char *key, void *var_ptr, config_type_e type, void *default_val, const char *comment)
{
    if (config_item_count >= CONFIG_MAX_ITEMS)
    {
        printf("[CONFIG ERROR] Config items limit exceeded!\r\n");
        return;
    }

    config_item_t *item = &config_items[config_item_count];

    // 复制参数名
    strncpy(item->key, key, sizeof(item->key) - 1);
    item->key[sizeof(item->key) - 1] = '\0';

    // 设置变量指针和类型
    item->var_ptr = var_ptr;
    item->type = type;

    // 复制默认值
    switch (type)
    {
        case CONFIG_TYPE_FLOAT:
            item->default_val.f = *(float *)default_val;
            break;
        case CONFIG_TYPE_DOUBLE:
            item->default_val.d = *(double *)default_val;
            break;
        case CONFIG_TYPE_INT:
            item->default_val.i = *(int *)default_val;
            break;
        case CONFIG_TYPE_INT16:
            item->default_val.i16 = *(int16 *)default_val;
            break;
        case CONFIG_TYPE_INT8:
            item->default_val.i8 = *(int8 *)default_val;
            break;
        case CONFIG_TYPE_UINT32:
            item->default_val.u32 = *(uint32 *)default_val;
            break;
        case CONFIG_TYPE_UINT16:
            item->default_val.u16 = *(uint16 *)default_val;
            break;
        case CONFIG_TYPE_UINT8:
            item->default_val.u8 = *(uint8 *)default_val;
            break;
    }

    // 复制注释
    if (comment != NULL)
    {
        strncpy(item->comment, comment, sizeof(item->comment) - 1);
        item->comment[sizeof(item->comment) - 1] = '\0';
    }
    else
    {
        item->comment[0] = '\0';
    }

    config_item_count++;
}

/**
 * @brief  保存所有配置到指定存档位
 */
uint8 config_save_slot(uint8 slot)
{
    if (slot >= CONFIG_SAVE_SLOT_COUNT)
    {
        printf("[CONFIG ERROR] Invalid slot %d\r\n", slot);
        return 1;
    }

    uint32 page = config_get_flash_page(slot);

    // 清空缓冲区
    flash_buffer_clear();

    // 将配置写入缓冲区
    config_write_to_buffer();

    // 计算需要写入的数据长度
    uint32 write_len = 2;  // 魔数 + 配置项数量
    for (uint8 i = 0; i < config_item_count; i++)
    {
        if (config_items[i].type == CONFIG_TYPE_DOUBLE)
            write_len += 2;  // double占用2个位置
        else
            write_len += 1;
    }

    // 擦除Flash页
    flash_erase_page(CONFIG_FLASH_SECTION, page);

    // 写入Flash
    flash_write_page_from_buffer(CONFIG_FLASH_SECTION, page, write_len);

    printf("[CONFIG] Saved to slot %d (page %d)\r\n", slot, page);
    return 0;
}

/**
 * @brief  从指定存档位加载配置
 */
uint8 config_load_slot(uint8 slot)
{
    if (slot >= CONFIG_SAVE_SLOT_COUNT)
    {
        printf("[CONFIG ERROR] Invalid slot %d\r\n", slot);
        return 1;
    }

    uint32 page = config_get_flash_page(slot);

    // 检查Flash是否已初始化
    if (!config_check_slot(slot))
    {
        printf("[CONFIG] Slot %d not initialized, using defaults\r\n", slot);
        config_reset_default();
        return 1;
    }

    // 清空缓冲区
    flash_buffer_clear();

    // 计算需要读取的数据长度
    uint32 read_len = 2;  // 魔数 + 配置项数量
    for (uint8 i = 0; i < config_item_count; i++)
    {
        if (config_items[i].type == CONFIG_TYPE_DOUBLE)
            read_len += 2;
        else
            read_len += 1;
    }

    // 从Flash读取
    flash_read_page_to_buffer(CONFIG_FLASH_SECTION, page, read_len);

    // 从缓冲区读取配置
    config_read_from_buffer();

    printf("[CONFIG] Loaded from slot %d (page %d)\r\n", slot, page);
    return 0;
}

/**
 * @brief  检查指定存档位是否已初始化
 */
uint8 config_check_slot(uint8 slot)
{
    if (slot >= CONFIG_SAVE_SLOT_COUNT)
        return 0;

    uint32 page = config_get_flash_page(slot);

    // 使用flash_check检查页是否有数据
    uint8 result = flash_check(CONFIG_FLASH_SECTION, page);

    return result;
}

/**
 * @brief  自动保存当前配置（掉电不丢失）
 */
uint8 config_auto_save(void)
{
    // 清空缓冲区
    flash_buffer_clear();

    // 将配置写入缓冲区
    config_write_to_buffer();

    // 计算需要写入的数据长度
    uint32 write_len = 2;  // 魔数 + 配置项数量
    for (uint8 i = 0; i < config_item_count; i++)
    {
        if (config_items[i].type == CONFIG_TYPE_DOUBLE)
            write_len += 2;  // double占用2个位置
        else
            write_len += 1;
    }

    // 擦除Flash页
    flash_erase_page(CONFIG_FLASH_SECTION, CONFIG_FLASH_PAGE_AUTO);

    // 写入Flash
    flash_write_page_from_buffer(CONFIG_FLASH_SECTION, CONFIG_FLASH_PAGE_AUTO, write_len);

    printf("[CONFIG] Auto-saved (page %d)\r\n", CONFIG_FLASH_PAGE_AUTO);
    return 0;
}

/**
 * @brief  自动加载配置（掉电不丢失）
 */
uint8 config_auto_load(void)
{
    // 检查Flash是否已初始化
    uint8 has_data = flash_check(CONFIG_FLASH_SECTION, CONFIG_FLASH_PAGE_AUTO);
    if (!has_data)
    {
        printf("[CONFIG] Auto-load: No saved data, using defaults\r\n");
        config_reset_default();
        return 1;
    }

    // 清空缓冲区
    flash_buffer_clear();

    // 计算需要读取的数据长度
    uint32 read_len = 2;  // 魔数 + 配置项数量
    for (uint8 i = 0; i < config_item_count; i++)
    {
        if (config_items[i].type == CONFIG_TYPE_DOUBLE)
            read_len += 2;
        else
            read_len += 1;
    }

    // 从Flash读取
    flash_read_page_to_buffer(CONFIG_FLASH_SECTION, CONFIG_FLASH_PAGE_AUTO, read_len);

    // 从缓冲区读取配置
    config_read_from_buffer();

    printf("[CONFIG] Auto-loaded (page %d)\r\n", CONFIG_FLASH_PAGE_AUTO);
    return 0;
}

/**
 * @brief  重置所有变量为默认值
 */
void config_reset_default(void)
{
    for (uint8 i = 0; i < config_item_count; i++)
    {
        config_item_t *item = &config_items[i];

        switch (item->type)
        {
            case CONFIG_TYPE_FLOAT:
                *(float *)item->var_ptr = item->default_val.f;
                break;
            case CONFIG_TYPE_DOUBLE:
                *(double *)item->var_ptr = item->default_val.d;
                break;
            case CONFIG_TYPE_INT:
                *(int *)item->var_ptr = item->default_val.i;
                break;
            case CONFIG_TYPE_INT16:
                *(int16 *)item->var_ptr = item->default_val.i16;
                break;
            case CONFIG_TYPE_INT8:
                *(int8 *)item->var_ptr = item->default_val.i8;
                break;
            case CONFIG_TYPE_UINT32:
                *(uint32 *)item->var_ptr = item->default_val.u32;
                break;
            case CONFIG_TYPE_UINT16:
                *(uint16 *)item->var_ptr = item->default_val.u16;
                break;
            case CONFIG_TYPE_UINT8:
                *(uint8 *)item->var_ptr = item->default_val.u8;
                break;
        }
    }

    printf("[CONFIG] Reset to default values\r\n");
}

/**
 * @brief  获取已注册的配置项数量
 */
uint8 config_get_item_count(void)
{
    return config_item_count;
}

/**
 * @brief  获取指定索引的配置项
 */
config_item_t* config_get_item(uint8 index)
{
    if (index >= config_item_count)
        return NULL;

    return &config_items[index];
}

/**
 * @brief  初始化配置系统
 */
void config_init(void)
{
    printf("[CONFIG] Initializing configuration system...\r\n");

    // 初始化Flash
    flash_init();

    // 配置项在各模块的初始化函数中注册
    // 加载配置由main函数在注册完配置项后调用config_auto_load()完成

    printf("[CONFIG] Configuration system initialized\r\n");
}

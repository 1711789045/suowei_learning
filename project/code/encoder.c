/*********************************************************************************************************************
* 文件名称: encoder.c
* 功能说明: 编码器模块 - 方向编码器读取与滤波
* 作者: Claude Code
* 日期: 2025-10-27
********************************************************************************************************************/

#include "encoder.h"

// 上次滤波值(用于低通滤波计算)
static int16 encoder_left_last = 0;
static int16 encoder_right_last = 0;

/*********************************************************************************************************************
* 函数名称: encoder_init
* 功能说明: 初始化编码器
* 参数说明: 无
* 返回值:   无
********************************************************************************************************************/
void encoder_init(void)
{
    // 初始化左编码器 (方向编码器模式)
    encoder_dir_init(ENCODER_LEFT, ENCODER_LEFT_A, ENCODER_LEFT_B);

    // 初始化右编码器 (方向编码器模式)
    encoder_dir_init(ENCODER_RIGHT, ENCODER_RIGHT_A, ENCODER_RIGHT_B);

    // 清空初始值
    encoder_left_last = 0;
    encoder_right_last = 0;

    printf("[ENCODER] Init OK - Left:TC_CH09(P05_0/1) Right:TC_CH07(P02_0/1)\r\n");
}

/*********************************************************************************************************************
* 函数名称: encoder_get_left
* 功能说明: 获取左编码器增量值(低通滤波后)
* 参数说明: 无
* 返回值:   int16 - 滤波后的增量值(带符号)
* 备注:     每次调用会清空硬件计数器,应在固定周期(如10ms)调用
********************************************************************************************************************/
int16 encoder_get_left(void)
{
    // 读取硬件计数器
    int16 current = encoder_get_count(ENCODER_LEFT);

    // 清空计数器(为下次读取做准备)
    encoder_clear_count(ENCODER_LEFT);

    // 低通滤波: filtered = 0.9 × current + 0.1 × last
    int16 filtered = (int16)(current * ENCODER_FILTER_NEW_COEF + encoder_left_last * ENCODER_FILTER_OLD_COEF);

    // 保存本次滤波值供下次使用
    encoder_left_last = filtered;

    return filtered;
}

/*********************************************************************************************************************
* 函数名称: encoder_get_right
* 功能说明: 获取右编码器增量值(低通滤波后)
* 参数说明: 无
* 返回值:   int16 - 滤波后的增量值(带符号)
* 备注:     每次调用会清空硬件计数器,应在固定周期(如10ms)调用
*           ⚠️ 右编码器方向反了，需要取反（正转应读正值）
********************************************************************************************************************/
int16 encoder_get_right(void)
{
    // 读取硬件计数器
    int16 current = encoder_get_count(ENCODER_RIGHT);

    // 清空计数器(为下次读取做准备)
    encoder_clear_count(ENCODER_RIGHT);

    // 低通滤波: filtered = 0.9 × current + 0.1 × last
    int16 filtered = (int16)(current * ENCODER_FILTER_NEW_COEF + encoder_right_last * ENCODER_FILTER_OLD_COEF);

    // 保存本次滤波值供下次使用
    encoder_right_last = filtered;

    // ⚠️ 取反修正方向（实测：正转时读到负值，需要反向）
    return -filtered;
}

/*********************************************************************************************************************
* CYT2BL3 Opensourec Library (CYT2BL3 开源库) 是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 CYT2BL3 开源库的一部分
*
* CYT2BL3 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL (GNU General Public License，即 GNU通用公共许可证) 的条款
* 即 GPL 的第3版 (即 GPL3.0) 或 (您选择的) 任何后来的版本，重新发布和/或修改它
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
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明 (即本声明)
*
* 文件名称          main_cm4
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-11-19     pudding            first version
* 2025-10-29     Claude             转换为UTF-8编码，修复中文乱码
* 2025-10-29     Claude             重构主函数结构，封装初始化代码
********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "menu.h"           // 菜单系统
#include "key.h"            // 按键处理
#include "flash.h"          // Flash配置管理
#include "motor.h"          // 电机控制系统
#include "encoder.h"        // 编码器模块
#include "pid.h"            // PID控制模块
#include "image.h"          // 图像处理模块

// 切换工程或工程移动位置后，需要执行以下步骤
// 第一步 关闭所有打开的文件
// 第二步 project->clean  等待其返回后再编译

// **************************** 代码区域 ****************************

/**
 * @brief  系统初始化函数
 * @param  无
 * @return 无
 * @note   按顺序初始化所有外设和系统模块
 */
void system_init(void)
{
    // 1. 时钟初始化 (必须最先执行)
    clock_init(SYSTEM_CLOCK_160M);

    // 2. 调试串口初始化
    debug_init();
    printf("\r\n========================================\r\n");
    printf("  CYT2BL3 Smart Car System v1.0\r\n");
    printf("  System Clock: 160MHz\r\n");
    printf("========================================\r\n");

    // 3. 电机控制系统初始化
    motor_init();

    // 4. 定时器中断初始化 (10ms电机控制周期)
    pit_ms_init(PIT_CH0, 10);

    // 5. 摄像头初始化 (MT9V03X 188x120@120FPS)
    mt9v03x_init();

    // 6. 菜单系统初始化 (内部会初始化Flash和按键)
    menu_init();
    menu_example_create();

    // 7. 加载配置 (从Flash Page 4加载，掉电不丢失)
    config_auto_load();

    // 8. 进入菜单
    menu_example_enter();

    printf("\r\n[System] All modules initialized successfully\r\n");
    printf("[System] System ready, entering main loop...\r\n\r\n");
}

/**
 * @brief  主函数
 * @param  无
 * @return 无
 */
int main(void)
{
    // 系统初始化
    system_init();

    // 主循环
    while(1)
    {
        // 菜单系统处理
        if(menu_is_active())
        {
            menu_process();             // 处理菜单逻辑 (按键、显示、编辑等)
            system_delay_ms(10);        // 10ms延时，100Hz刷新率
        }
        else
        {
            // 图像处理 (只在菜单未激活时执行，避免显示冲突)
            if(mt9v03x_finish_flag)
            {
                image_process(MT9V03X_W, MT9V03X_H, 1);  // mode=1: 显示边线
                mt9v03x_finish_flag = 0;
            }
        }
    }
}

// **************************** 代码区域 ****************************

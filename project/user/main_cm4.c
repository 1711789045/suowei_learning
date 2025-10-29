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
 * @note   按顺序初始化所有外设和系统模块，失败时打印错误并在屏幕显示
 */
void system_init(void)
{
    uint8 init_failed = 0;  // 初始化失败标志

    // 1. 时钟初始化 (必须最先执行)
    printf("\r\n========================================\r\n");
    printf("  CYT2BL3 Smart Car System v1.0\r\n");
    printf("  Initializing...\r\n");
    printf("========================================\r\n");

    printf("[1/8] Clock initialization...");
    clock_init(SYSTEM_CLOCK_160M);
    printf("OK (160MHz)\r\n");

    // 2. 调试串口初始化
    printf("[2/8] Debug UART initialization...");
    debug_init();
    printf("OK\r\n");

    // 3. 电机控制系统初始化
    printf("[3/8] Motor system initialization...");
    motor_init();
    printf("OK\r\n");

    // 4. 定时器中断初始化 (10ms电机控制周期)
    printf("[4/8] Timer interrupt (10ms)...");
    pit_ms_init(PIT_CH0, 10);
    printf("OK\r\n");

    // 5. 摄像头初始化 (MT9V03X 188x120@120FPS)
    printf("[5/8] Camera MT9V03X initialization...");
    if(mt9v03x_init())
    {
        printf("FAILED!\r\n");
        printf("[ERROR] Camera initialization failed\r\n");
        init_failed = 1;
    }
    else
    {
        printf("OK (188x120@120FPS)\r\n");
    }

    // 6. 菜单系统初始化 (内部会初始化Flash和按键)
    printf("[6/8] Menu system initialization...");
    menu_init();
    menu_example_create();
    printf("OK\r\n");

    // 7. 加载配置 (从Flash Slot 0自动加载)
    printf("[7/8] Loading config from Flash...");
    config_auto_load();
    printf("OK\r\n");

    // 8. 进入菜单
    printf("[8/8] Entering menu...");
    menu_example_enter();
    printf("OK\r\n");

    // 初始化完成提示
    printf("\r\n========================================\r\n");
    if(init_failed)
    {
        printf("  System initialized with ERRORS!\r\n");
        printf("  Please check hardware connections\r\n");
        printf("========================================\r\n\r\n");

        // 在屏幕上显示错误信息
        ips114_clear();
        ips114_show_string(0, 0, "INIT ERROR!");
        ips114_show_string(0, 20, "Camera failed");
        ips114_show_string(0, 40, "Check hardware");
        system_delay_ms(3000);  // 显示3秒
        menu_refresh();  // 刷新菜单显示
    }
    else
    {
        printf("  All modules initialized successfully!\r\n");
        printf("  System ready\r\n");
        printf("========================================\r\n\r\n");
    }
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
            // 降低打印频率，每1秒打印一次（避免刷屏）
            static uint16 print_cnt = 0;
            print_cnt++;
            if(print_cnt >= 100)  // 100 * 10ms = 1000ms
            {
                print_cnt = 0;
                printf("[PID] Kp=%.2f Ki=%.2f Kd=%.2f (addr: Kp=0x%08X Ki=0x%08X Kd=0x%08X)\r\n",
                       motor_kp, motor_ki, motor_kd,
                       (uint32)&motor_kp, (uint32)&motor_ki, (uint32)&motor_kd);
            }
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

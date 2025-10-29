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

// 本工程是开源库示例工程，请不要直接移动或更改其中任何内容
// 本工程是开源库示例工程，请不要直接移动或更改其中任何内容
// 本工程是开源库示例工程，请不要直接移动或更改其中任何内容

// **************************** 代码区域 ****************************

int main(void)
{
    clock_init(SYSTEM_CLOCK_160M);      // 时钟设置及系统初始化 <必须最开始>

    debug_init();                       // 调试串口初始化
    printf("[MAIN] === System Boot Start ===\r\n");
    printf("[MAIN] Step 1: Clock init OK (160MHz)\r\n");
    printf("[MAIN] Step 2: Debug UART init OK\r\n");

    // 此处编写用户代码 例如外设初始化代码

    // 初始化电机控制系统
    printf("[MAIN] Step 3: Motor init starting...\r\n");
    motor_init();                       // 初始化电机PWM、编码器、PID
    printf("[MAIN] Step 3: Motor init OK\r\n");

    printf("[MAIN] Step 4: PIT timer init starting...\r\n");
    pit_ms_init(PIT_CH0, 10);           // 初始化10ms定时器中断
    printf("[MAIN] Step 4: PIT timer init OK (10ms)\r\n");

    // 初始化MT9V03X摄像头
    printf("[MAIN] Step 5: MT9V03X camera init starting...\r\n");
    mt9v03x_init();                     // 初始化MT9V03X (188x120@120FPS)
    printf("[MAIN] Step 5: MT9V03X camera init OK (188x120@120FPS)\r\n");

    // 初始化菜单系统 (内部会初始化Flash和配置系统)
    printf("[MAIN] Step 6: Menu system init starting...\r\n");
    menu_init();
    printf("[MAIN] Step 6: Menu system init OK\r\n");

    printf("[MAIN] Step 7: Menu create starting...\r\n");
    menu_example_create();              // 创建菜单 (会注册配置项)
    printf("[MAIN] Step 7: Menu create OK\r\n");

    // 自动加载配置 (掉电不丢失，从Flash Page 4加载)
    printf("[MAIN] Step 8: Config auto load starting...\r\n");
    config_auto_load();
    printf("[MAIN] Step 8: Config auto load OK\r\n");

    // 进入菜单
    printf("[MAIN] Step 9: Menu enter starting...\r\n");
    menu_example_enter();               // 直接进入菜单
    printf("[MAIN] Step 9: Menu enter OK\r\n");

    printf("[MAIN] === System Boot Complete ===\r\n");
    printf("[MAIN] Menu system started\r\n");
    printf("[MAIN] Motor control system started (10ms interrupt)\r\n");
    printf("[MAIN] Entering main loop...\r\n");




    // 此处编写用户代码 例如外设初始化代码

    uint32 loop_count = 0;
    uint32 image_count = 0;

    while(1)
    {
        // 此处编写需要循环执行的代码

        // 菜单系统处理 (使用10ms延时，相当于100Hz刷新率)
        if(menu_is_active())
        {
            menu_process();             // 处理菜单逻辑
            system_delay_ms(10);        // 10ms延时，按键每20ms扫描一次 (2次延时)
        }
        else
        {
            // 只有菜单未激活时才显示图像 (避免冲突)
            // 图像处理 (MT9V03X摄像头)
            if(mt9v03x_finish_flag){
                image_count++;
                printf("[MAIN] Image processing #%d starting...\r\n", image_count);
                // mode=1: 显示边线，mode=0: 不显示
                image_process(MT9V03X_W, MT9V03X_H, 1);
                printf("[MAIN] Image processing #%d OK\r\n", image_count);
                mt9v03x_finish_flag = 0;
            }
        }

        // 每1000次循环输出一次心跳信息
        loop_count++;
        if(loop_count % 1000 == 0){
            printf("[MAIN] Main loop alive: %d iterations\r\n", loop_count);
        }

        // 此处编写需要循环执行的代码
    }
}

// **************************** 代码区域 ****************************

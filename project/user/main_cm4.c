/*********************************************************************************************************************
* CYT2BL3 Opensourec Library ���� CYT2BL3 ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2022 SEEKFREE ��ɿƼ�
*
* ���ļ��� CYT2BL3 ��Դ���һ����
*
* CYT2BL3 ��Դ�� ���������
* �����Ը���������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù�������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
*
* ����Դ��ķ�����ϣ�����ܷ������ã�����δ�������κεı�֤
* ����û�������������Ի��ʺ��ض���;�ı�֤
* ����ϸ����μ� GPL
*
* ��Ӧ�����յ�����Դ���ͬʱ�յ�һ�� GPL �ĸ���
* ���û�У������<https://www.gnu.org/licenses/>
*
* ����ע����
* ����Դ��ʹ�� GPL3.0 ��Դ����֤Э�� ������������Ϊ���İ汾
* ��������Ӣ�İ��� libraries/doc �ļ����µ� GPL3_permission_statement.txt �ļ���
* ����֤������ libraries �ļ����� �����ļ����µ� LICENSE �ļ�
* ��ӭ��λʹ�ò����������� ���޸�����ʱ���뱣����ɿƼ��İ�Ȩ����������������
*
* �ļ�����          main_cm4
* ��˾����          �ɶ���ɿƼ����޹�˾
* �汾��Ϣ          �鿴 libraries/doc �ļ����� version �ļ� �汾˵��
* ��������          IAR 9.40.1
* ����ƽ̨          CYT2BL3
* ��������          https://seekfree.taobao.com/
*
* �޸ļ�¼
* ����              ����                ��ע
* 2024-11-19       pudding            first version
********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "menu.h"           // 菜单系统
#include "key.h"            // 按键处理
#include "flash.h"          // Flash配置管理
#include "motor.h"          // 电机控制系统
#include "encoder.h"        // 编码器模块
#include "pid.h"            // PID控制模块
#include "image.h"          // 图像处理模块

// ���µĹ��̻��߹����ƶ���λ�����ִ�����²���
// ��һ�� �ر��������д򿪵��ļ�
// �ڶ��� project->clean  �ȴ��·�����������

// �������ǿ�Դ��չ��� ��������ֲ���߲��Ը���������
// �������ǿ�Դ��չ��� ��������ֲ���߲��Ը���������
// �������ǿ�Դ��չ��� ��������ֲ���߲��Ը���������

// **************************** �������� ****************************

int main(void)
{
    clock_init(SYSTEM_CLOCK_160M);      // ʱ�����ü�ϵͳ��ʼ��<��ر���>

    debug_init();                       // ���Դ��ڳ�ʼ��
    printf("[MAIN] === System Boot Start ===\r\n");
    printf("[MAIN] Step 1: Clock init OK (160MHz)\r\n");
    printf("[MAIN] Step 2: Debug UART init OK\r\n");

    // �˴���д�û����� ���������ʼ�������

    // 初始化电机控制系统
    printf("[MAIN] Step 3: Motor init starting...\r\n");
    motor_init();                       // 初始化电机PWM、编码器、PID
    printf("[MAIN] Step 3: Motor init OK\r\n");

    printf("[MAIN] Step 4: PIT timer init starting...\r\n");
    pit_ms_init(PIT_CH0, 10);           // 初始化10ms定时器中断
    printf("[MAIN] Step 4: PIT timer init OK (10ms)\r\n");

    // 初始化MT9V03X摄像头
    printf("[MAIN] Step 5: MT9V03X camera init starting...\r\n");
    mt9v03x_init();                     // 初始化MT9V03X（188x120@120FPS）
    printf("[MAIN] Step 5: MT9V03X camera init OK (188x120@120FPS)\r\n");

    // 初始化菜单系统（内部会初始化Flash和配置系统）
    printf("[MAIN] Step 6: Menu system init starting...\r\n");
    menu_init();
    printf("[MAIN] Step 6: Menu system init OK\r\n");

    printf("[MAIN] Step 7: Menu create starting...\r\n");
    menu_example_create();              // 创建菜单（会注册配置项）
    printf("[MAIN] Step 7: Menu create OK\r\n");

    // 自动加载配置（掉电不丢失，从Flash Page 4加载）
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




    // �˴���д�û����� ���������ʼ�������

    uint32 loop_count = 0;
    uint32 image_count = 0;

    while(1)
    {
        // �˴���д��Ҫѭ��ִ�еĴ���

        // 菜单系统处理（使用10ms延时，相当于100Hz刷新率）
        if(menu_is_active())
        {
            menu_process();             // 处理菜单逻辑
            system_delay_ms(10);        // 10ms延时，按键每20ms扫描一次（2次延时）
        }
        else
        {
            // 只有菜单未激活时才显示图像（避免冲突）
            // 图像处理（MT9V03X摄像头）
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

        // �˴���д��Ҫѭ��ִ�еĴ���
    }
}

// **************************** �������� ****************************

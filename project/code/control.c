/*********************************************************************************************************************
* CYT2BL3 Opensourec Library ����CYT2BL3 ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2022 SEEKFREE ��ɿƼ�
*
* ���ļ���CYT2BL3 ��Դ���һ����
*
* CYT2BL3 ��Դ�� ��������
* �����Ը��������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
*
* �ļ�����          control
* ����˵��          ��������ģ�� - ����/ͣ��/ͣ�����
* ����              Claude Code
* ����              2025-10-31
********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "control.h"
#include "motor.h"
#include "menu.h"
#include "key.h"
#include "image.h"
#include "pid.h"
#include <stdio.h>

// ==================== ȫ�ֱ��� ====================
uint8 car_running = 0;              // С������״̬��0=ֹͣ��1=���У�
uint8 stop_flag = 0;                // ͣ����־λ

// ==================== ����ʵ�� ====================

/**
 * @brief  ��ʼ������ģ��
 * @param  ��
 * @return ��
 */
void control_init(void)
{
    car_running = 0;
    stop_flag = 0;
}

/**
 * @brief  �����������˵����ã�
 * @param  ��
 * @return ��
 * @note   ��һ�ΰ�ȷ�ϼ�����ʾ��ʾ������3�뵹��ʱ
 *         3�����ٴΰ�ȷ�ϼ�������ʱ3-2-1�󷢳�
 *         3����δ��ȷ�ϼ���ȡ������
 */
void start_car(void)
{
    uint32 timeout_count = 0;
    uint8 key_pressed = 0;
    
    // ��������ʾ��ʾ��Ϣ
    ips114_clear();
    ips114_show_string(20, 40, "Press KEY3 again");
    ips114_show_string(20, 60, "in 3s to START");
    
    // �ȴ�3�����ٴΰ���KEY3
    timeout_count = 0;
    while(timeout_count < 150)  // 150 * 20ms = 3000ms
    {
        // ɨ�谴��
        key_scanner();
        
        // ���KEY3��ȷ�ϼ���
        if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
        {
            key_pressed = 1;
            break;
        }
        
        system_delay_ms(20);
        timeout_count++;
    }
    
    // ���3����δ������ȡ������
    if(!key_pressed)
    {
        ips114_clear();
        ips114_show_string(40, 60, "START Cancelled");
        system_delay_ms(1000);
        menu_refresh();  // ˢ�²˵���ʾ
        return;
    }
    
    // ==================== ��������ʱ���ɼ�ͼ�����ݣ�====================
    // ��������ʾ����ʱ
    ips114_clear();
    
    // ����ʱ3�룬ͬʱ�ɼ�ͼ������
    for(uint8 i = 3; i > 0; i--)
    {
        char buf[16];
        sprintf(buf, "START in %d", i);
        ips114_show_string(40, 60, buf);
        system_delay_ms(1000);  // �ӳ�1��
    }
    
    // ��ʾ"GO!"
    ips114_clear();
    ips114_show_string(60, 60, "GO!");
    system_delay_ms(500);  // ͣ��0.5��
    
    // ==================== �˳��˵���׼������ ====================
    menu_exit();
    
    // �������ر���ʾ��������Ļˢ�¶����ܵ�Ӱ�죩
    ips114_clear();
    
    // ==================== ����PID״̬ ====================
    // ���������ʷ���ͻ��֣������������
    pid_reset(&pid_speed_left);
    pid_reset(&pid_speed_right);
    pid_reset(&pid_direction);
    
    // ==================== ���ͼ��������Ч�� ====================
    // ȷ��final_mid_line�ں���Χ�ڣ���ֹ��ʼֵΪ0���µľ޴���
    if(final_mid_line < 20 || final_mid_line > IMAGE_W - 20)
    {
        final_mid_line = IMAGE_W / 2;  // ǿ����Ϊ�е㣨94��
        printf("[CONTROL] Warning: final_mid_line invalid, reset to center\r\n");
    }
    
    // ==================== ����ʽ���� ====================
    // �ȿ������򻷣�����У����̬��
    direction_debug_enable = 1;
    
    // ����ԭʼbasic_speed
    int16 target_speed = basic_speed;
    
    // ��1�׶Σ�30%�ٶȣ�����300ms��У��������̬��
    basic_speed = target_speed * 0.3;
    speed_debug_enable = 1;
    system_delay_ms(300);
    
    // ��2�׶Σ�60%�ٶȣ�����200ms
    basic_speed = target_speed * 0.6;
    system_delay_ms(200);
    
    // ��3�׶Σ��ָ�Ŀ���ٶ�
    basic_speed = target_speed;
    
    // ��������״̬
    car_running = 1;
    
    printf("[CONTROL] Car started! mid_line=%d, basic_speed=%d\r\n", 
           final_mid_line, basic_speed);
}

/**
 * @brief  ͣ������
 * @param  ��
 * @return ��
 * @note   �����رշ��� �� �ٶȻ�Ŀ����0 �� ��ʱ1sɲ��
 *         �� �ر��ٶȻ� �� PWM��0 �� ��ʾ��ʾ �� �ȴ����� �� �����˵�
 */
void stop_car(void)
{
    // 1. �����رշ���
    direction_debug_enable = 0;
    
    
    
    // 4. �ر��ٶȻ�
    speed_debug_enable = 0;
    
    // 5. PWM��0������ֹͣ��
    pwm_set_duty(MOTOR_LEFT_PWM, 0);
    pwm_set_duty(MOTOR_RIGHT_PWM, 0);
    
    // 6. ����ֹͣ״̬
    car_running = 0;
    
    // 7. ��ʾͣ����ʾ��Ϣ
    ips114_clear();
    ips114_show_string(30, 40, "CAR STOPPED!");
    ips114_show_string(20, 60, "Press KEY4 to");
    ips114_show_string(30, 80, "return menu");
    
    // 8. �ȴ�����KEY4���ؼ�
    while(1)
    {
        // ɨ�谴��
        key_scanner();
        
        // ���KEY4�����ؼ���
        if(key_get_state(KEY_4) == KEY_SHORT_PRESS || 
           key_get_state(KEY_4) == KEY_LONG_PRESS)
        {
            key_clear_state(KEY_4);  // �������״̬
            break;  // �˳��ȴ�ѭ��
        }
        
        system_delay_ms(20);  // 20ms��ʱ
    }
    
    // 9. �����˵�
    menu_example_enter();
}

/**
 * @brief  ͣ����⣨��ѭ�����ã�
 * @param  ��
 * @return ��
 * @note   ���prospectֵ��С��5ʱ����ͣ��
 */
void stop_detection(void)
{
    // ֻ������״̬�¼��
    if(car_running)
    {
        // ���ǰհֵ
        if(prospect < 5)
        {
            // ����ͣ����־
            stop_flag = 1;
            
            // ����ͣ��
            stop_car();
            
            // ��ձ�־
            stop_flag = 0;
        }
    }
}


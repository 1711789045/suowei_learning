/*********************************************************************************************************************
* 文件名称: motor.c
* 功能说明: 电机控制模块 - PWM驱动+编码器反馈+PID闭环
* 作者: Claude Code
* 日期: 2025-10-27
********************************************************************************************************************/

#include "motor.h"
#include <string.h>

// 左右电机独立的PID状态
static pid_t pid_left;
static pid_t pid_right;

// 目标值(编码器增量)
static int16 target_left = 0;
static int16 target_right = 0;

// 实际值(编码器增量,用于VOFA+显示)
static int16 actual_left = 0;
static int16 actual_right = 0;

// VOFA+速度环调试
uint8 motor_vofa_enable = 0;        // 速度环调试开关(1=开启, 0=关闭)
int16 motor_basic_speed = 100;      // 速度环基础速度(默认100，编码器增量目标值)

// 内部函数声明
static void motor_set_pwm_left(int16 pwm);
static void motor_set_pwm_right(int16 pwm);

/*********************************************************************************************************************
* 函数名称: motor_init
* 功能说明: 初始化电机系统(PWM+编码器+PID)
* 参数说明: 无
* 返回值:   无
********************************************************************************************************************/
void motor_init(void)
{
    // 初始化PWM (HIP4082驱动, 17kHz, 初始占空比0)
    pwm_init(MOTOR_LEFT_A, MOTOR_PWM_FREQ, 0);
    pwm_init(MOTOR_LEFT_B, MOTOR_PWM_FREQ, 0);
    pwm_init(MOTOR_RIGHT_A, MOTOR_PWM_FREQ, 0);
    pwm_init(MOTOR_RIGHT_B, MOTOR_PWM_FREQ, 0);

    // 初始化编码器
    encoder_init();

    // 初始化PID参数
    pid_init();

    // 重置PID状态
    pid_reset(&pid_left);
    pid_reset(&pid_right);

    // 清空目标值
    target_left = 0;
    target_right = 0;
    actual_left = 0;
    actual_right = 0;

    // 关闭VOFA+调试
    motor_vofa_enable = 0;

    printf("[MOTOR] Init OK - PWM:17kHz Left:CH14/13 Right:CH51/50\r\n");
}

/*********************************************************************************************************************
* 函数名称: motor_set_target_left
* 功能说明: 设置左电机目标值
* 参数说明: target - 目标编码器增量值(带符号,负数表示反转)
* 返回值:   无
********************************************************************************************************************/
void motor_set_target_left(int16 target)
{
    target_left = target;
}

/*********************************************************************************************************************
* 函数名称: motor_set_target_right
* 功能说明: 设置右电机目标值
* 参数说明: target - 目标编码器增量值(带符号,负数表示反转)
* 返回值:   无
********************************************************************************************************************/
void motor_set_target_right(int16 target)
{
    target_right = target;
}

/*********************************************************************************************************************
* 函数名称: motor_process
* 功能说明: 电机控制周期(应在10ms定时器中断中调用)
* 参数说明: 无
* 返回值:   无
* 备注:     完成一个控制周期: 读编码器→PID计算→设置PWM→VOFA+输出
********************************************************************************************************************/
void motor_process(void)
{
    // 速度环调试模式：设置目标速度为basic_speed
    if (motor_vofa_enable)
    {
        target_left = motor_basic_speed;
        target_right = motor_basic_speed;
    }

    // 读取编码器实际值(已滤波)
    actual_left = encoder_get_left();
    actual_right = encoder_get_right();

    // PID计算(输入和输出都是int16→float转换)
    float pid_out_left = pid_calc(&pid_left, (float)target_left, (float)actual_left);
    float pid_out_right = pid_calc(&pid_right, (float)target_right, (float)actual_right);

    // 设置电机PWM(自动处理正反转)
    motor_set_pwm_left((int16)pid_out_left);
    motor_set_pwm_right((int16)pid_out_right);

    // VOFA+调试输出
    if (motor_vofa_enable)
    {
        motor_vofa_send();
    }
}

/*********************************************************************************************************************
* 函数名称: motor_set_pwm_left
* 功能说明: 设置左电机PWM(带方向控制)
* 参数说明: pwm - PWM值(正数=正转, 负数=反转, 0=停止)
* 返回值:   无
* 备注:     自动限幅到[-MOTOR_PWM_MAX_DUTY, +MOTOR_PWM_MAX_DUTY]
********************************************************************************************************************/
static void motor_set_pwm_left(int16 pwm)
{
    // 限幅
    if (pwm > MOTOR_PWM_MAX_DUTY)
        pwm = MOTOR_PWM_MAX_DUTY;
    else if (pwm < -MOTOR_PWM_MAX_DUTY)
        pwm = -MOTOR_PWM_MAX_DUTY;

    // 根据符号控制方向
    if (pwm >= 0)  // 正转
    {
        pwm_set_duty(MOTOR_LEFT_A, pwm);
        pwm_set_duty(MOTOR_LEFT_B, 0);
    }
    else  // 反转
    {
        pwm_set_duty(MOTOR_LEFT_A, 0);
        pwm_set_duty(MOTOR_LEFT_B, -pwm);
    }
}

/*********************************************************************************************************************
* 函数名称: motor_set_pwm_right
* 功能说明: 设置右电机PWM(带方向控制)
* 参数说明: pwm - PWM值(正数=正转, 负数=反转, 0=停止)
* 返回值:   无
* 备注:     自动限幅到[-MOTOR_PWM_MAX_DUTY, +MOTOR_PWM_MAX_DUTY]
********************************************************************************************************************/
static void motor_set_pwm_right(int16 pwm)
{
    // 限幅
    if (pwm > MOTOR_PWM_MAX_DUTY)
        pwm = MOTOR_PWM_MAX_DUTY;
    else if (pwm < -MOTOR_PWM_MAX_DUTY)
        pwm = -MOTOR_PWM_MAX_DUTY;

    // 根据符号控制方向
    if (pwm >= 0)  // 正转
    {
        pwm_set_duty(MOTOR_RIGHT_A, pwm);
        pwm_set_duty(MOTOR_RIGHT_B, 0);
    }
    else  // 反转
    {
        pwm_set_duty(MOTOR_RIGHT_A, 0);
        pwm_set_duty(MOTOR_RIGHT_B, -pwm);
    }
}

/*********************************************************************************************************************
* 函数名称: motor_vofa_send
* 功能说明: 发送VOFA+ JustFloat调试数据(速度环调试)
* 参数说明: 无
* 返回值:   无
* 备注:     发送3个通道: actual_left, actual_right, target_speed
*           协议: 3×float32 + 尾巴(0x00 0x00 0x80 0x7f)
********************************************************************************************************************/
void motor_vofa_send(void)
{
    // VOFA+ JustFloat协议: 3个float + 4字节尾巴
    uint8 vofa_buffer[16];  // 3×4 + 4 = 16字节
    float data[3];

    // 准备数据(int16→float转换)
    data[0] = (float)actual_left;       // 左编码器实际速度
    data[1] = (float)actual_right;      // 右编码器实际速度
    data[2] = (float)motor_basic_speed; // 目标速度

    // 拷贝float数据到缓冲区
    memcpy(vofa_buffer, data, 12);

    // 添加JustFloat协议尾巴
    vofa_buffer[12] = 0x00;
    vofa_buffer[13] = 0x00;
    vofa_buffer[14] = 0x80;
    vofa_buffer[15] = 0x7f;

    // 通过调试串口发送(二进制数据)
    uart_write_buffer(DEBUG_UART_INDEX, vofa_buffer, 16);
}

/*********************************************************************************************************************
* 函数名称: motor_set_pid
* 功能说明: 设置PID参数(左右电机共享)
* 参数说明: kp - 比例系数
*           ki - 积分系数
*           kd - 微分系数
* 返回值:   无
********************************************************************************************************************/
void motor_set_pid(float kp, float ki, float kd)
{
    motor_kp = kp;
    motor_ki = ki;
    motor_kd = kd;

    // 重新初始化PID控制器，使新参数立即生效
    pid_reset(&pid_left);
    pid_reset(&pid_right);

    printf("[MOTOR] PID updated - Kp=%.2f Ki=%.2f Kd=%.2f\r\n", motor_kp, motor_ki, motor_kd);
}

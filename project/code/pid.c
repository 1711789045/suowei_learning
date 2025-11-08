/*********************************************************************************************************************
* 文件名称: pid.c
* 功能说明: PID控制模块 - 增量式PID算法
* 作者: Claude Code
* 日期: 2025-10-27
********************************************************************************************************************/

#include "pid.h"


// ==================== 速度环PID参数（左右轮共用）====================
float speed_kp = 0.0f;     // 速度环比例系数(初值=0)
float speed_ki = 0.0f;     // 速度环积分系数(初值=0)
float speed_kd = 0.0f;     // 速度环微分系数(初值=0)

// ==================== 方向环PID参数 ====================
float direction_kp2 = 0.0f;
float direction_kp = 0.0f;      // 比例系数(初值=0)
float direction_ki = 0.0f;      // 积分系数(初值=0)
float direction_kd_img = 0.0f;      // 微分系数(初值=0)
float direction_kd_g = 0.0f;      // 微分系数(初值=0)
/*********************************************************************************************************************
* 函数名称: pid_init
* 功能说明: 初始化PID参数
* 参数说明: 无
* 返回值:   无
* 备注:     参数初始值为0,需通过菜单调整
********************************************************************************************************************/
void pid_init(void)
{
    // 速度环PID（左右轮共用）
    speed_kp = 0.0f;
    speed_ki = 0.0f;
    speed_kd = 0.0f;

    // 方向环PID
    direction_kp2 = 0.0f;
    direction_kp = 0.0f;
    direction_ki = 0.0f;
    direction_kd_img = 0.0f;
    direction_kd_g = 0.0f;

    printf("[PID] Init OK - Speed(Kp=%.2f Ki=%.2f Kd=%.2f)\r\n",
           speed_kp, speed_ki, speed_kd);
    printf("[PID] Direction: Kp2=%.3f Kp=%.2f Ki=%.2f Kd_img=%.2f Kd_g=%.2f\r\n",
           direction_kp2, direction_kp, direction_ki, direction_kd_img, direction_kd_g);
}

/*********************************************************************************************************************
* 函数名称: pid_reset
* 功能说明: 重置PID状态(清空历史误差和输出)
* 参数说明: pid - PID状态结构体指针
* 返回值:   无
* 备注:     切换控制目标或启动时调用,避免历史误差影响
********************************************************************************************************************/
void pid_reset(pid_t *pid)
{
    pid->error_last = 0.0f;
    pid->error_last2 = 0.0f;
    pid->output = 0.0f;
    pid->integral = 0.0f;
}

/*********************************************************************************************************************
* 函数名称: pid_calc_incremental
* 功能说明: 计算PID输出(增量式) - 用于速度环
* 参数说明: pid    - PID状态结构体指针
*           kp     - 比例系数
*           ki     - 积分系数
*           kd     - 微分系数
*           target - 目标值
*           actual - 实际值
* 返回值:   float - PID控制输出
* 备注:     增量式PID公式: Δu(k) = Kp*[e(k)-e(k-1)] + Ki*e(k) + Kd*[e(k)-2*e(k-1)+e(k-2)]
*           输出累加: u(k) = u(k-1) + Δu(k)
********************************************************************************************************************/
float pid_calc_incremental(pid_t *pid, float kp, float ki, float kd, float target, float actual)
{
    // 计算当前误差 e(k)
    float error = target - actual;

    // 计算增量式PID各项（使用传入的参数）
    float delta_p = kp * (error - pid->error_last);                          // 比例项增量
    float delta_i = ki * error;                                               // 积分项（误差本身）
    float delta_d = kd * (error - 2.0f * pid->error_last + pid->error_last2); // 微分项增量

    // 计算总增量
    float delta_output = delta_p + delta_i + delta_d;

    // 累加到输出
    pid->output += delta_output;

    // 限幅（防止积分饱和）
    if (pid->output > 10000.0f)
        pid->output = 10000.0f;
    else if (pid->output < -10000.0f)
        pid->output = -10000.0f;

    // 更新历史误差
    pid->error_last2 = pid->error_last;
    pid->error_last = error;

    return pid->output;
}

/*********************************************************************************************************************
* 函数名称: pid_calc_position_speed
* 功能说明: 计算PID输出(位置式) - 用于速度环
* 参数说明: pid    - PID状态结构体指针
*           kp     - 比例系数
*           ki     - 积分系数
*           kd     - 微分系数
*           target - 目标值
*           actual - 实际值
* 返回值:   float - PID控制输出
* 备注:     位置式PID公式: u(k) = Kp*e(k) + Ki*Σe(k) + Kd*[e(k)-e(k-1)]
*           相比增量式PID，位置式PID每次重新计算输出，不累积历史输出误差
*           优势：左右轮速度差异不会因为历史输出累积而放大
********************************************************************************************************************/
float pid_calc_position_speed(pid_t *pid, float kp, float ki, float kd, float target, float actual)
{
    // 计算当前误差 e(k)
    float error = target - actual;

    // 积分累加
    pid->integral += error;

    // 积分限幅（防止积分饱和）
    if (pid->integral > 5000.0f)
        pid->integral = 5000.0f;
    else if (pid->integral < -5000.0f)
        pid->integral = -5000.0f;

    // 微分项（一阶差分，对噪声不敏感）
    float derivative = error - pid->error_last;

    // 位置式PID输出
    float output = kp * error + ki * pid->integral + kd * derivative;

    // 输出限幅
    if (output > 10000.0f)
        output = 10000.0f;
    else if (output < -10000.0f)
        output = -10000.0f;

    // 更新历史误差
    pid->error_last = error;

    return output;
}

/*********************************************************************************************************************
* 函数名称: pid_calc_position
* 功能说明: 计算PID输出(位置式) - 用于方向环
* 参数说明: pid    - PID状态结构体指针
*           target - 目标值
*           actual - 实际值
* 返回值:   float - PID控制输出
* 备注:     位置式PID公式: u(k) = Kp*e(k) + Ki*Σe(k) + Kd*[e(k)-e(k-1)]
********************************************************************************************************************/
float pid_calc_position(pid_t *pid, float target, float actual)
{
    imu660ra_get_gyro();
    // 计算当前误差 e(k)
    float error = actual - target;

    // 积分累加
    pid->integral += error;

    // 积分限幅（防止积分饱和）
    if (pid->integral > 5000.0f)
        pid->integral = 5000.0f;
    else if (pid->integral < -5000.0f)
        pid->integral = -5000.0f;

    // 微分项
    float derivative = error - pid->error_last;

    // 位置式PID输出
    float output = direction_kp2 * fabs(error) * error + direction_kp * error + direction_ki * pid->integral + direction_kd_img * derivative + direction_kd_g * imu660ra_gyro_z *0.01;

    // 输出限幅
    if (output > 10000.0f)
        output = 10000.0f;
    else if (output < -10000.0f)
        output = -10000.0f;

    // 更新历史误差
    pid->error_last = error;

    return output;
}

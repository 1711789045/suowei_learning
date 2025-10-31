/*********************************************************************************************************************
* 文件名称: motor.c
* 功能说明: 电机控制模块 - PWM驱动+编码器反馈+串级PID闭环(方向环+速度环)
* 作者: Claude Code
* 日期: 2025-10-27
* 修改: 2025-10-31 - 添加方向环控制，实现串级PID
********************************************************************************************************************/

#include "motor.h"
#include "image.h"
#include <string.h>

// ==================== 速度环PID状态 ====================
static pid_t pid_speed_left;        // 左轮速度环PID
static pid_t pid_speed_right;       // 右轮速度环PID

// ==================== 方向环PID状态 ====================
static pid_t pid_direction;         // 方向环PID

// 速度环目标值(编码器增量)
static int16 target_left = 0;
static int16 target_right = 0;

// 速度环实际值(编码器增量)
static int16 actual_left = 0;
static int16 actual_right = 0;

// ==================== 速度环参数 ====================
uint8 speed_debug_enable = 0;       // 速度环调试开关(1=开启, 0=关闭)
int16 basic_speed = 100;            // 基础速度(默认100，编码器增量目标值)

// ==================== 方向环参数 ====================
uint8 direction_debug_enable = 0;   // 方向环调试开关(1=开启, 0=关闭)
float inner_wheel_ratio = 1.0f;     // 内轮减速系数(默认1.0)
float outer_wheel_ratio = 1.0f;     // 外轮加速系数(默认1.0)

// 方向环控制周期计数器(5ms中断，每2次执行1次方向环=10ms)
static uint8 direction_counter = 0;

// 方向环调试状态记录（用于检测0→1跳变）
static uint8 last_direction_debug_enable = 0;

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
    // 初始化DIR引脚 (DRV8701E驱动方向控制, 默认高电平=正转)
    gpio_init(MOTOR_LEFT_DIR, GPO, GPIO_HIGH, GPO_PUSH_PULL);
    gpio_init(MOTOR_RIGHT_DIR, GPO, GPIO_HIGH, GPO_PUSH_PULL);
    
    // 初始化PWM (DRV8701E驱动速度控制, 17kHz, 初始占空比0)
    pwm_init(MOTOR_LEFT_PWM, MOTOR_PWM_FREQ, 0);
    pwm_init(MOTOR_RIGHT_PWM, MOTOR_PWM_FREQ, 0);

    // 初始化编码器
    encoder_init();

    // 初始化PID参数
    pid_init();

    // 重置速度环PID状态
    pid_reset(&pid_speed_left);
    pid_reset(&pid_speed_right);
    
    // 重置方向环PID状态
    pid_reset(&pid_direction);

    // 清空目标值
    target_left = 0;
    target_right = 0;
    actual_left = 0;
    actual_right = 0;
    
    // 清空方向环计数器
    direction_counter = 0;

    // 关闭调试开关
    speed_debug_enable = 0;
    direction_debug_enable = 0;

    printf("[MOTOR] Init OK - DRV8701E Left:P18_6/CH50 Right:P00_2/CH13\r\n");
    printf("[MOTOR] Speed Loop: 5ms | Direction Loop: 10ms (Cascade PID)\r\n");
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
* 功能说明: 电机控制周期(应在5ms定时器中断中调用)
* 参数说明: 无
* 返回值:   无
* 备注:     串级PID控制: 方向环(10ms)→速度环(5ms)
*           - 速度环: 每5ms执行一次
*           - 方向环: 每10ms执行一次(计数器控制)
********************************************************************************************************************/
void motor_process(void)
{
    // ==================== 方向环调试状态检测 ====================
    // 检测direction_debug_enable从0变为1（新开启）
    if (direction_debug_enable == 1 && last_direction_debug_enable == 0)
    {
        // 清除方向环PID状态
        pid_reset(&pid_direction);
        
        // 清除速度环PID状态
        pid_reset(&pid_speed_left);
        pid_reset(&pid_speed_right);
        
        printf("[MOTOR] Direction debug enabled - PID states reset\r\n");
    }
    
    // 更新状态记录
    last_direction_debug_enable = direction_debug_enable;
    
    // ==================== 方向环控制 (10ms周期) ====================
    if (direction_debug_enable)
    {
        // 计数器递增(5ms中断，每2次执行1次方向环)
        direction_counter++;
        
        if (direction_counter >= 2)  // 10ms到了
        {
            direction_counter = 0;
            
            // 计算中线偏差 (左正右负)
            float mid_error = (float)(final_mid_line - IMAGE_W / 2);
            
            // 方向环PID计算 (位置式PID)
            // 目标值=0(保持中线), 实际值=mid_error
            float direction_output = pid_calc_position(&pid_direction, 0.0f, mid_error);
            
            // 差速控制计算
            // 偏左(mid_error>0) → 需要右转 → 左轮加速，右轮减速
            // 偏右(mid_error<0) → 需要左转 → 右轮加速，左轮减速
            if (mid_error > 0)  // 偏左，需要右转
            {
                target_left  = basic_speed + (int16)(direction_output * outer_wheel_ratio);  // 左轮是外轮，加速
                target_right = basic_speed - (int16)(direction_output * inner_wheel_ratio);  // 右轮是内轮，减速
            }
            else  // 偏右或居中，需要左转或直行
            {
                target_left  = basic_speed + (int16)(direction_output * inner_wheel_ratio);  // 左轮是内轮
                target_right = basic_speed - (int16)(direction_output * outer_wheel_ratio);  // 右轮是外轮
            }
        }
    }
    // ==================== 速度环调试模式 ====================
    else if (speed_debug_enable)
    {
        // 速度环调试：直接设置目标速度
        target_left = basic_speed;
        target_right = basic_speed;
    }
    
    // ==================== 速度环控制 (5ms周期) ====================
    // 读取编码器实际值(已滤波)
    actual_left = encoder_get_left();
    actual_right = encoder_get_right();
    
    // 速度环PID计算(增量式PID)
    float speed_out_left = pid_calc_incremental(&pid_speed_left, (float)target_left, (float)actual_left);
    float speed_out_right = pid_calc_incremental(&pid_speed_right, (float)target_right, (float)actual_right);
    
    // 设置电机PWM(自动处理正反转)
    motor_set_pwm_left((int16)speed_out_left);
    motor_set_pwm_right((int16)speed_out_right);
    
    // ==================== 调试输出 ====================
    if (speed_debug_enable || direction_debug_enable)
    {
        static uint8 debug_cnt = 0;
        debug_cnt++;
        
        if (debug_cnt >= 20)  // 每100ms打印一次（20*5ms）
        {
            debug_cnt = 0;
            
            if (direction_debug_enable)
            {
                // 方向环调试输出
                float mid_error = (float)(final_mid_line - IMAGE_W / 2);
                printf("[DIRECTION] MidLine=%d Error=%.1f | L_target=%d R_target=%d\r\n",
                       final_mid_line, mid_error, target_left, target_right);
                printf("[SPEED] L(T=%d A=%d PWM=%d) R(T=%d A=%d PWM=%d)\r\n",
                       target_left, actual_left, (int16)speed_out_left,
                       target_right, actual_right, (int16)speed_out_right);
            }
            else if (speed_debug_enable)
            {
                // 速度环调试输出
                printf("[SPEED] L(T=%d A=%d Err=%.1f PWM=%d) R(T=%d A=%d Err=%.1f PWM=%d)\r\n",
                       target_left, actual_left, (float)target_left - actual_left, (int16)speed_out_left,
                       target_right, actual_right, (float)target_right - actual_right, (int16)speed_out_right);
            }
        }
        
        // VOFA+输出
        motor_vofa_send();
    }
}

/*********************************************************************************************************************
* 函数名称: motor_set_pwm_left
* 功能说明: 设置左电机PWM(DRV8701E驱动方式)
* 参数说明: pwm - PWM值(正数=正转, 负数=反转, 0=停止)
* 返回值:   无
* 备注:     DRV8701E: DIR引脚控制方向, PWM通道控制速度
********************************************************************************************************************/
static void motor_set_pwm_left(int16 pwm)
{
    // 限幅
    if (pwm > MOTOR_PWM_MAX_DUTY)
        pwm = MOTOR_PWM_MAX_DUTY;
    else if (pwm < -MOTOR_PWM_MAX_DUTY)
        pwm = -MOTOR_PWM_MAX_DUTY;

    // DRV8701E控制方式
    if (pwm >= 0)  // 正转
    {
        gpio_set_level(MOTOR_LEFT_DIR, GPIO_LOW);  // DIR=HIGH 正转
        pwm_set_duty(MOTOR_LEFT_PWM, pwm);          // PWM设置速度
    }
    else  // 反转
    {
        gpio_set_level(MOTOR_LEFT_DIR, GPIO_HIGH);   // DIR=LOW 反转
        pwm_set_duty(MOTOR_LEFT_PWM, -pwm);         // PWM设置速度(取绝对值)
    }
}

/*********************************************************************************************************************
* 函数名称: motor_set_pwm_right
* 功能说明: 设置右电机PWM(DRV8701E驱动方式)
* 参数说明: pwm - PWM值(正数=正转, 负数=反转, 0=停止)
* 返回值:   无
* 备注:     DRV8701E: DIR引脚控制方向, PWM通道控制速度
********************************************************************************************************************/
static void motor_set_pwm_right(int16 pwm)
{
    // 限幅
    if (pwm > MOTOR_PWM_MAX_DUTY)
        pwm = MOTOR_PWM_MAX_DUTY;
    else if (pwm < -MOTOR_PWM_MAX_DUTY)
        pwm = -MOTOR_PWM_MAX_DUTY;

    // DRV8701E控制方式
    if (pwm >= 0)  // 正转
    {
        gpio_set_level(MOTOR_RIGHT_DIR, GPIO_LOW);  // DIR=HIGH 正转
        pwm_set_duty(MOTOR_RIGHT_PWM, pwm);          // PWM设置速度
    }
    else  // 反转
    {
        gpio_set_level(MOTOR_RIGHT_DIR, GPIO_HIGH);   // DIR=LOW 反转
        pwm_set_duty(MOTOR_RIGHT_PWM, -pwm);         // PWM设置速度(取绝对值)
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
    // VOFA+ JustFloat协议: 4个float + 4字节尾巴
    uint8 vofa_buffer[20];  // 4×4 + 4 = 20字节
    float data[4];

    // 准备数据(int16→float转换)
    data[0] = (float)actual_left;       // 左编码器实际速度
    data[1] = (float)actual_right;      // 右编码器实际速度
    data[2] = (float)basic_speed;       // 目标速度
    data[3] = (float)final_mid_line;    // 中线位置

    // 拷贝float数据到缓冲区
    memcpy(vofa_buffer, data, 16);

    // 添加JustFloat协议尾巴
    vofa_buffer[16] = 0x00;
    vofa_buffer[17] = 0x00;
    vofa_buffer[18] = 0x80;
    vofa_buffer[19] = 0x7f;

    // 通过调试串口发送(二进制数据)
    uart_write_buffer(DEBUG_UART_INDEX, vofa_buffer, 20);
}

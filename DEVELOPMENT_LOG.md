# 缩微车模开发日志

本文档记录Car_project项目的开发历史和当前进度,方便后续开发时快速了解项目状态。

---

## 2025-11-02

### 发车时序再优化 - 原地调整方向后再前进

**工作内容**:

#### ✅ 发车时序优化（control.c）

**需求背景**:
- 之前的发车时序：倒计时3秒 → 立即以预设速度前进
- 问题：车辆可能未对准中线就开始前进，容易冲出赛道
- 解决：先让车辆原地调整方向1秒，对准中线后再前进

**新发车时序**:
```
倒计时3秒
    ↓
保存预设basic_speed → basic_speed置为0
    ↓
开启速度环和方向环（车辆原地调整方向）
    ↓
延时1秒（只有方向控制，无前进速度）
    ↓
恢复basic_speed为预设值（开始沿中线前进）
```

**核心代码**:
```c
// 1. 保存预设的basic_speed
int16 target_speed = basic_speed;

// 2. 将basic_speed置为0（原地调整方向阶段）
basic_speed = 0;

// 3. 设置运行状态并开启控制环
car_running = 1;
speed_debug_enable = 1;
direction_debug_enable = 1;

// 4. 退出菜单
menu_exit();

// 5. 延时1秒（原地调整方向，对准中线）
system_delay_ms(1000);

// 6. 恢复basic_speed为预设值（开始前进）
basic_speed = target_speed;
```

**技术优势**:
- ✅ **姿态预校准**: 发车前1秒自动对准中线
- ✅ **平滑启动**: 避免突然启动导致的方向偏差
- ✅ **提升成功率**: 减少发车时冲出赛道的概率
- ✅ **保留原有优势**: 仍有图像数据预热（倒计时3秒）

**时间线对比**:

| 阶段 | 旧时序 | 新时序 |
|------|--------|--------|
| 倒计时 | 3秒（图像采集） | 3秒（图像采集） ✅ |
| 启动方式 | 立即全速前进 | basic_speed=0，原地调方向 ✅ |
| 对准中线 | ❌ 无 | ✅ 1秒原地调整 |
| 开始前进 | 第0秒 | 第1秒（已对准中线）✅ |

**编译验证**:
```
Total number of errors: 0
Total number of warnings: 24 (已存在)
Build succeeded
```

**Git提交**:
- Commit: `d420e3f`
- 提交信息: "优化发车时序：先原地调整方向1秒，再前进"
- 推送到: `origin/main`

**下一步计划**:
- 实车测试新发车时序效果
- 观察1秒调整时间是否足够
- 如需要可调整原地调整时间（0.5s - 2s）

---

### 编码器滤波算法升级 - 从低通滤波改为卡尔曼滤波

**工作内容**:

#### 1. ✅ 新增通用滤波算法库

**实现内容**:
- 创建`filter.c/h`模块，包含7种常用滤波算法：
  1. **一阶低通滤波** - 简单高效，平滑数据
  2. **滑动平均滤波** - 消除随机噪声
  3. **中值滤波** - 去除脉冲干扰，保留边缘
  4. **限幅滤波** - 限制变化速率，防止突变
  5. **一阶互补滤波** - 融合两种传感器数据
  6. **二阶巴特沃斯低通滤波** - 平滑性好，相位延迟小
  7. **一阶卡尔曼滤波** ⭐ - 最优估计，自适应调节

**代码特点**:
- 统一的结构体定义和API接口
- 详细的注释说明每种滤波器的特点和适用场景
- 支持float类型数据处理
- 易于扩展和使用

#### 2. ✅ 编码器模块升级为卡尔曼滤波

**改动说明**:

**修改前（低通滤波）**:
```c
// encoder.c - 旧版本
static int16 encoder_left_last = 0;
static int16 encoder_right_last = 0;

// 低通滤波: filtered = 0.05 × new + 0.95 × old
int16 filtered = (int16)(current * 0.05f + encoder_left_last * 0.95f);
```

**修改后（卡尔曼滤波）**:
```c
// encoder.c - 新版本
static KalmanFilter_t encoder_left_kalman;
static KalmanFilter_t encoder_right_kalman;

// 初始化卡尔曼滤波器
KalmanFilter_Init(&encoder_left_kalman, 0.01f, 2.0f, 0.0f);

// 卡尔曼滤波更新
float filtered = KalmanFilter_Update(&encoder_left_kalman, (float)current);
```

**参数选择依据**:
- 参考《滤波算法快速选择指南》
- 编码器速度推荐使用卡尔曼滤波
- Q = 0.01 (过程噪声协方差)
- R = 2.0 (测量噪声协方差)
- Q/R = 0.005 → 平衡平滑度和响应速度

#### 3. ✅ IAR工程配置更新

**修改文件**:
- `project/iar/project_config/cyt2bl3.ewp`
- 在encoder.h之后添加filter.c和filter.h
- 确保filter.c被正确编译和链接

#### 4. ✅ 编译验证通过

**编译结果**:
```
Total number of errors: 0
Total number of warnings: 0
Build succeeded
```

**Git提交**:
- Commit: `ec0cf17`
- 提交信息: "编码器滤波升级：从低通滤波改为卡尔曼滤波"
- 推送到: `origin/main`

---

**技术优势对比**:

| 特性 | 低通滤波 (旧) | 卡尔曼滤波 (新) |
|------|--------------|----------------|
| 平滑度 | ★★★ | ★★★★ |
| 响应速度 | ★★★★ | ★★★★ |
| 抗噪声能力 | ★★★ | ★★★★★ |
| 自适应性 | ❌ 固定系数 | ✅ 自适应调节 |
| 理论基础 | 经验公式 | 最优估计理论 |
| 计算复杂度 | ★ (最低) | ★★ (较低) |
| 参数调节 | 1个 (alpha) | 2个 (Q, R) |

**卡尔曼滤波工作原理**:
1. **预测阶段**: 根据上一时刻的状态预测当前状态
2. **更新阶段**: 融合预测值和测量值，得到最优估计
3. **自适应性**: 根据噪声特性自动调整融合权重
4. **最优性**: 在最小均方误差意义下的最优估计

**效果预期**:
- ✅ 编码器速度估计更准确
- ✅ 抗突变噪声能力更强
- ✅ 速度环控制更稳定
- ✅ 电机响应更平滑

**参考资料**:
- 参考代码: `/其他资源/另一个缩微代码/project/code/filter/`
- 快速选择指南: `/其他资源/另一个缩微代码/project/code/filter/快速选择指南.md`
- 编码器速度推荐: 卡尔曼滤波 ⭐ (Q=0.01, R=2.0)

**下一步计划**:
- 实车测试卡尔曼滤波效果
- 如需调整，可微调Q和R参数：
  * 太平滑 → 增大Q或减小R (更信任测量)
  * 太抖动 → 减小Q或增大R (更信任预测)
- 考虑为陀螺仪数据添加巴特沃斯滤波

**模块状态更新**:
| 模块名称 | 状态 | 完成日期 | 说明 |
|---------|------|---------|------|
| 通用滤波算法库 | ✅ 完成 | 2025-11-02 | 7种滤波算法实现 |
| 编码器卡尔曼滤波 | ✅ 完成 | 2025-11-02 | 替换低通滤波 |

---

## 2025-11-01

### 发车时序优化和左右轮PID参数分离

**工作内容**:

#### 1. ✅ 发车时序优化（control.c）

**问题描述**:
- 车辆在赛道上静止发车时出现剧烈抖动或原地旋转
- 发车后容易冲出赛道触发停车
- 需要"运气好"才能正常巡线

**根本原因**:
1. 图像数据未准备好：`final_mid_line`初始值为0，导致方向环计算巨大误差（-94）
2. 发车时序混乱：PID重置后立即启动控制，没有给图像处理系统缓冲时间
3. 缺少倒计时：控制环立即全速运行，启动冲击大

**解决方案**:
1. ✅ 添加3-2-1倒计时显示
   - 倒计时3秒，期间图像处理系统持续运行采集稳定数据
   - 用户可看到明确的倒计时提示
   
2. ✅ 图像数据有效性检查
   ```c
   if(final_mid_line < 20 || final_mid_line > IMAGE_W - 20)
   {
       final_mid_line = IMAGE_W / 2;  // 强制设为中点（94）
   }
   ```
   - 防止使用无效的中线数据
   - 如果数据异常，强制设为图像中心

3. ✅ 渐进式发车（分3个阶段）
   - 第1阶段：30%速度，持续300ms（校正车身姿态）
   - 第2阶段：60%速度，持续200ms（平滑加速）
   - 第3阶段：100%速度（正常巡线）
   - 每个阶段都有足够的稳定时间

**发车流程优化**:
```
旧时序：PID重置 → 立即启动 → 抖动
新时序：倒计时(采集图像) → 检查数据 → PID重置 → 渐进启动 → 平稳运行
```

#### 2. ✅ 左右轮速度环PID参数分离

**需求背景**:
- 两个电机性能可能存在差异
- 左右轮共用PID参数无法精确调整
- 难以应对电机不一致导致的跑偏问题

**实现方案**:

1. ✅ 修改pid.h/pid.c
   - 将`speed_kp/ki/kd`分离为左右轮独立参数：
     * `speed_left_kp/ki/kd`（左轮）
     * `speed_right_kp/ki/kd`（右轮）
   - 修改`pid_calc_incremental()`函数签名：
     ```c
     // 旧版：使用全局参数
     float pid_calc_incremental(pid_t *pid, float target, float actual);
     
     // 新版：支持自定义参数传入
     float pid_calc_incremental(pid_t *pid, float kp, float ki, float kd, 
                                float target, float actual);
     ```

2. ✅ 修改motor.c
   - 左右轮使用各自的PID参数计算：
     ```c
     // 左轮使用左轮参数
     pid_calc_incremental(&pid_speed_left, speed_left_kp, speed_left_ki, speed_left_kd, ...);
     
     // 右轮使用右轮参数
     pid_calc_incremental(&pid_speed_right, speed_right_kp, speed_right_ki, speed_right_kd, ...);
     ```

3. ✅ 修改menu.c
   - 在SpeedPID菜单下添加7个参数：
     1. Left Kp - 左轮比例系数
     2. Left Ki - 左轮积分系数
     3. Left Kd - 左轮微分系数
     4. Right Kp - 右轮比例系数
     5. Right Ki - 右轮积分系数
     6. Right Kd - 右轮微分系数
     7. Basic Speed - 基础速度
   - 所有参数注册到Flash配置系统，支持保存/加载

**使用方法**:
```
菜单路径：
Main Menu → Parameter → SpeedPID → 
├─ Left Kp     (左轮比例系数)
├─ Left Ki     (左轮积分系数)
├─ Left Kd     (左轮微分系数)
├─ Right Kp    (右轮比例系数)
├─ Right Ki    (右轮积分系数)
├─ Right Kd    (右轮微分系数)
└─ Basic Speed (基础速度)
```

**调试技巧**:
1. 初始调试：先把左右轮参数设置为相同值
2. 精细调优：如果车辆跑偏，可单独调整弱侧电机参数
3. 常见情况：
   - 车辆向左偏 → 增加左轮Kp或减小右轮Kp
   - 车辆向右偏 → 增加右轮Kp或减小左轮Kp

**修改的文件**:
- `project/code/control.c` - 发车时序优化
- `project/code/control.h` - 添加头文件引用
- `project/code/pid.h` - 添加左右轮独立参数
- `project/code/pid.c` - 实现左右轮独立PID
- `project/code/motor.c` - 调用左右轮独立参数
- `project/code/menu.c` - 添加菜单项

**编译状态**:
- ✅ Total number of errors: 0
- ⚠️ Total number of warnings: 20
- ✅ Build succeeded

**Git提交**:
- Commit: `60a723c`
- 提交信息: "优化发车时序和左右轮PID参数分离"
- 推送到: `origin/main`

**技术亮点**:
- ✨ **渐进式发车**：避免启动冲击，提升稳定性
- ✨ **数据校验**：防止异常中线值导致的失控
- ✨ **左右轮独立调整**：适应电机差异，提升直线性
- ✨ **参数持久化**：Flash保存，掉电不丢失

**效果预期**:
1. 发车平稳，无抖动和原地旋转
2. 车辆能够稳定启动并进入巡线状态
3. 可针对性调整左右轮参数，改善跑偏问题

**下一步计划**:
- 实车测试发车流程
- 调试左右轮PID参数差异
- 优化渐进式发车的速度比例和持续时间

---

## 2025-10-31

### 串级PID控制系统实现与硬件驱动修复

**工作内容**:
1. ✅ 修正电机驱动芯片类型
   - 识别实际硬件为 **DRV8701E**（不是HIP4082）
   - DRV8701E控制方式：DIR(GPIO方向) + PWM(速度)
   - 通过引脚测试确定正确的引脚映射：
     * 左电机：DIR=P18_6, PWM=CH50_P18_7
     * 右电机：DIR=P00_2, PWM=CH13_P00_3
   - 重写motor.c的PWM控制函数（motor_set_pwm_left/right）

2. ✅ 修正编码器方向问题
   - 发现左右编码器方向都反了（正转读到负值）
   - 在encoder_get_left()中添加取反操作
   - 解决速度环正反馈导致电机疯转的问题

3. ✅ 实现串级PID控制系统
   - **外环（方向环）**: 10ms周期，位置式PID
     * 输入：final_mid_line - IMAGE_W/2（中线偏差）
     * 输出：差速控制量
     * 根据偏差计算左右轮差速目标值
   - **内环（速度环）**: 5ms周期，增量式PID
     * 输入：编码器速度
     * 输出：PWM控制量
     * 跟踪方向环设定的差速目标

4. ✅ PID模块重构（pid.h/pid.c）
   - 变量重命名：
     * motor_kp/ki/kd → speed_kp/ki/kd（速度环）
     * 新增：direction_kp/ki/kd（方向环）
   - 添加位置式PID函数：pid_calc_position()
   - 重命名增量式PID函数：pid_calc() → pid_calc_incremental()
   - PID状态结构体添加integral字段（位置式PID用）

5. ✅ 电机模块重构（motor.h/motor.c）
   - 变量重命名：
     * motor_vofa_enable → speed_debug_enable
     * motor_basic_speed → basic_speed
   - 新增变量：
     * direction_debug_enable（方向环调试开关）
     * inner_wheel_ratio（内轮减速系数）
     * outer_wheel_ratio（外轮加速系数）
   - 重写motor_process()函数：
     * 实现串级PID控制逻辑
     * 方向环10ms计算差速
     * 速度环5ms跟踪目标

6. ✅ 差速转向控制逻辑
   - 偏左（mid_error>0）→ 需要右转：
     * 左轮（外轮）加速：basic_speed + output × outer_ratio
     * 右轮（内轮）减速：basic_speed - output × inner_ratio
   - 偏右（mid_error<0）→ 需要左转：
     * 左轮（内轮）减速：basic_speed + output × inner_ratio
     * 右轮（外轮）加速：basic_speed - output × outer_ratio

7. ✅ 菜单系统更新（menu.c）
   - 子菜单重命名：
     * "Servo" → "DirectionPID"（方向环参数）
     * "Motor" → "SpeedPID"（速度环参数）
   - 参数重新注册：
     * 方向环：direction_kp/ki/kd, inner_ratio, outer_ratio
     * 速度环：speed_kp/ki/kd, basic_speed
   - Debug菜单新增：
     * SpeedDebug（速度环调试开关）
     * DirectionDebug（方向环调试开关）

8. ✅ 控制周期调整
   - 定时器中断：10ms → 5ms (main_cm4.c, cm4_isr.c)
   - 速度环：每5ms执行一次
   - 方向环：每10ms执行一次（计数器控制）

9. ✅ 调试逻辑优化
   - 速度环调试模式：只执行速度环，双轮同速
   - 方向环调试模式：自动启用速度环（串级PID）
   - 调试输出：100ms周期，显示中线偏差、目标值、实际值、PWM

10. ✅ 编码规范
    - 更新CLAUDE.md：添加UTF-8编码规范章节
    - 包含编码检查清单、转换方法、常见问题等

**技术亮点**:
- ✨ **串级PID架构**：外环控制方向，内环控制速度，解耦控制
- ✨ **差速转向**：内外轮独立比例系数，精确控制转弯
- ✨ **双周期控制**：方向环10ms，速度环5ms，提升响应速度
- ✨ **灵活调试**：支持独立调试速度环或串级调试方向环
- ✨ **位置式+增量式**：方向环无累积误差，速度环抗干扰

**参数说明**:
```
速度环（增量式PID）:
├── speed_kp: 比例系数
├── speed_ki: 积分系数
├── speed_kd: 微分系数
└── basic_speed: 基础速度（编码器增量）

方向环（位置式PID）:
├── direction_kp: 比例系数
├── direction_ki: 积分系数
├── direction_kd: 微分系数
├── inner_wheel_ratio: 内轮减速系数
└── outer_wheel_ratio: 外轮加速系数
```

**调试流程**:
1. 先调试速度环：speed_debug=1, direction_debug=0
2. 再调试方向环：direction_debug=1（自动启用速度环）
3. PID参数从0开始，逐步增大调试

**下一步计划**:
- 在实车上测试速度环响应
- 调试方向环PID参数
- 优化差速系数（inner_ratio, outer_ratio）
- 测试循迹效果

**已解决的问题**:
1. ✅ 硬件驱动识别错误（HIP4082 → DRV8701E）
2. ✅ 引脚映射错误（通过4阶段测试确定）
3. ✅ 编码器方向反向（取反修正）
4. ✅ 速度环正反馈问题（编码器方向导致）

---

## 2025-10-26

### 菜单系统移植与集成

**工作内容**:
1. ✅ 从龙芯C车代码移植菜单系统到CYT2BL3平台
2. ✅ 创建menu_system模块,包含5个子模块:
   - **menu_config**: Flash配置管理(支持4个存档位)
     - 实现了基于Flash的参数持久化存储
     - 支持4个独立存档位,可保存/加载不同配置
     - 开机自动加载存档位0
     - 支持float/double/int/int16/int8/uint32/uint16/uint8等多种参数类型
     - 魔数校验机制防止读取未初始化Flash
   - **menu_key**: 按键处理模块
     - 基于逐飞zf_device_key驱动封装
     - 支持4个按键: 上/下/确认/返回
     - 支持短按和长按检测(约600ms)
     - 20ms扫描周期
   - **menu_display**: IPS114显示适配
     - 适配IPS114屏幕(240x135分辨率)
     - 每页显示6个参数(原IPS200为8个,适配小屏幕)
     - 选中项蓝色高亮,编辑模式红色高亮
     - 支持参数值实时显示
   - **menu_core**: 菜单核心逻辑
     - 链表结构的多级菜单系统
     - 支持菜单导航(上/下/进入/返回)
     - 支持参数编辑模式
     - 支持功能函数调用
     - 局部刷新优化,减少屏幕闪烁
   - **menu_example**: 菜单使用示例
     - 演示如何创建参数菜单
     - 演示如何创建功能菜单
     - 演示存档位保存/加载功能
3. ✅ 实现了完整的菜单API:
   - `menu_init()`: 初始化菜单系统
   - `menu_create_param()`: 创建参数单元
   - `menu_create_function()`: 创建功能单元
   - `menu_link()`: 链接菜单单元
   - `menu_process()`: 菜单处理(在主循环调用)
   - `config_save_slot()`: 保存到存档位
   - `config_load_slot()`: 从存档位加载
4. ✅ 编译验证通过
   - 编译结果: **0 errors, 0 warnings**
   - 新增代码: 10个文件,共2047行
5. ✅ 提交到GitHub
   - 提交ID: a65f7e5
   - 推送到远程仓库成功

**技术特点**:
- ✨ **易扩展**: 添加新参数只需一行代码
- ✨ **多存档位**: 支持4个独立配置存档,方便不同场景切换
- ✨ **Flash持久化**: 参数掉电不丢失
- ✨ **局部刷新**: 只刷新变化部分,减少闪烁
- ✨ **类型安全**: 支持多种参数类型,自动类型转换

**文件列表**:
```
Car_project/project/user/
├── inc/menu_system/
│   ├── menu_config.h      # Flash配置管理头文件
│   ├── menu_core.h        # 菜单核心逻辑头文件
│   ├── menu_display.h     # IPS114显示适配头文件
│   ├── menu_key.h         # 按键处理头文件
│   └── menu_example.h     # 使用示例头文件
└── src/menu_system/
    ├── menu_config.c      # Flash配置管理实现
    ├── menu_core.c        # 菜单核心逻辑实现
    ├── menu_display.c     # IPS114显示适配实现
    ├── menu_key.c         # 按键处理实现
    └── menu_example.c     # 使用示例实现
```

**使用方法**:
```c
// 在main函数中初始化
menu_init();
menu_example_create();

// 在需要时进入菜单
menu_example_enter();

// 在主循环中处理菜单
while(1)
{
    if(menu_is_active())
    {
        menu_process();  // 20ms调用一次
    }
}
```

**下一步计划**:
- 将菜单系统文件添加到IAR项目中
- 集成到实际应用代码
- 添加更多功能函数(如参数导出,实时调试等)
- 完善存档位选择界面

---

### 项目初始化与构建环境配置

**工作内容**:
1. ✅ 分析了整个CYT2BL3代码库的结构和架构
2. ✅ 创建了`CLAUDE.md`文档,记录了:
   - 项目概述和硬件平台信息
   - 代码库三层架构(SDK层/驱动层/设备层)
   - 开发环境配置(IAR 9.40.1)
   - 常用API参考
   - 示例程序说明
   - 调试和故障排除指南
3. ✅ 在`CLAUDE.md`中明确标记`Car_project/`为主开发项目
4. ✅ 创建了本开发日志文档`DEVELOPMENT_LOG.md`
5. ✅ 配置了IAR命令行编译环境
   - IAR路径: `D:\IAR\common\bin\iarbuild.exe`
   - 成功编译Car_project工程
   - 编译结果: **0 errors, 0 warnings**
   - 生成文件: `cyt2bl3.out`, `cyt2bl3.hex`
6. ✅ 完善了`CLAUDE.md`文档中的编译规范
   - 添加了"Claude Code 开发工作流程"章节
   - 明确规定每次写代码后必须自动编译
   - 规定必须检查编译错误,如有错误必须修改后重新编译
   - 要求确保0 errors才能完成开发任务
   - 添加了详细的命令行编译说明
   - 添加了编译参数说明和输出文件说明
7. ✅ 配置了Git版本控制和GitHub集成
   - 在Car_project目录初始化Git仓库
   - 创建了`.gitignore`文件排除IAR编译产物
   - 配置远程仓库: `git@github.com:1711789045/suowei_learning.git`
   - 完成首次提交(569个文件,289356行代码)
   - 成功推送到GitHub main分支
   - SSH认证配置正常
8. ✅ 更新`CLAUDE.md`添加Git工作流程
   - 添加了Git提交和推送流程说明
   - 明确规定每次任务完成后必须提交到GitHub
   - 添加了GitHub仓库信息
   - 规定提交信息格式

**当前项目状态**:
- 项目目录: `Car_project/`
- 工程文件: `Car_project/project/iar/cyt2bl3.eww`
- 项目配置: `Car_project/project/iar/project_config/cyt2bl3.ewp`
- 用户代码目录: `Car_project/project/user/src/`
- 开发环境: IAR EW for ARM 9.40.1
- 目标硬件: CYT2BL3核心板 + 主板
- **编译状态**: ✅ 正常通过
- **Git仓库**: ✅ 已初始化
- **GitHub远程**: `git@github.com:1711789045/suowei_learning.git`
- **当前分支**: `main`

**编译命令**:
```bash
"D:\IAR\common\bin\iarbuild.exe" "<project_path>\cyt2bl3.ewp" -make Debug -log info -parallel 4
```

**下一步计划**:
- 待用户明确具体开发任务
- 可能的任务包括:
  - 电机控制系统
  - 传感器集成(IMU/编码器)
  - 路径规划和控制算法
  - 显示屏界面
  - 无线通信

**备注**:
- `Example/`和`Seekfree_CYT2BL3_Opensource_Library/`仅供参考,不进行修改
- 每次开发完成后需要更新本日志
- 可以使用命令行编译,无需打开IAR IDE
- **每次任务完成后必须提交并推送到GitHub**
- Git工作流程已集成到开发流程中

---

## 开发规范

### 文件组织
- 用户代码放在: `Car_project/project/user/src/`
- 头文件放在: `Car_project/project/user/inc/`
- 所有代码包含: `#include "zf_common_headfile.h"`

### 代码规范
- 变量命名: 小写字母+下划线 (如: `motor_speed`, `sensor_data`)
- 函数命名: 小写字母+下划线 (如: `motor_init()`, `get_sensor_value()`)
- 宏定义: 大写字母+下划线 (如: `MAX_SPEED`, `MOTOR_PIN`)
- 每个功能模块独立成文件

### 开发流程
1. 打开IAR工程: `Car_project/project/iar/cyt2bl3.eww`
2. 编写代码
3. 编译: `Project -> Make` (F7)
4. 下载调试: `Project -> Download and Debug` (Ctrl+D)
5. 更新开发日志

### 参考资源
- 基础外设示例: `Example/Coreboard_Demo/`
- 设备驱动示例: `Example/Motherboard_Demo/`
- API文档: `CLAUDE.md`中的"常用API参考"部分

---

## 模块开发状态

| 模块名称 | 状态 | 完成日期 | 说明 |
|---------|------|---------|------|
| 项目初始化 | ✅ 完成 | 2025-10-26 | 创建开发文档 |
| 编译环境配置 | ✅ 完成 | 2025-10-26 | IAR命令行编译成功(0错误0警告) |
| 开发工作流程规范 | ✅ 完成 | 2025-10-26 | 建立自动编译检查机制 |
| Git版本控制 | ✅ 完成 | 2025-10-26 | Git仓库初始化和配置 |
| GitHub集成 | ✅ 完成 | 2025-10-26 | 远程仓库配置和首次推送 |
| 菜单系统 | ✅ 完成 | 2025-10-26 | 从龙芯C车移植,支持4存档位,2047行代码 |
| 配置管理 | ✅ 完成 | 2025-10-26 | Flash持久化,多类型参数支持 |
| 按键处理 | ✅ 完成 | 2025-10-26 | 4键导航,长短按检测 |
| 显示适配 | ✅ 完成 | 2025-10-26 | IPS114屏幕适配,6项/页 |
| 待补充 | ⏳ 待开始 | - | - |

---

## 问题和解决方案

### 问题记录

暂无

---

## 版本历史

### v0.1 (2025-10-26)
- 初始化项目
- 创建开发文档和日志系统

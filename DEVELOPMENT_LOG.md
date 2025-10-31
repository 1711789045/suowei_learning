# 缩微车模开发日志

本文档记录Car_project项目的开发历史和当前进度,方便后续开发时快速了解项目状态。

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

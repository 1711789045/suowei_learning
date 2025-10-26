# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## ⚠️ 重要：当前开发项目

**主开发项目**: `Car_project/`
- 这是用户正在开发的缩微车模代码项目
- **所有代码开发工作默认在此目录下进行**
- 工程路径: `Car_project/project/iar/cyt2bl3.eww`
- 用户代码: `Car_project/project/user/src/`

**其他目录说明**:
- `Example/` - 仅供参考的示例代码
- `Seekfree_CYT2BL3_Opensource_Library/` - 逐飞开源库,仅供参考
- **不要修改Example和Seekfree_CYT2BL3_Opensource_Library中的代码**

**开发日志**: 查看 `DEVELOPMENT_LOG.md` 了解当前开发进度和历史记录

---

## 🤖 Claude Code 开发工作流程（重要）

**每次修改/编写代码后的强制流程**:

1. ✅ **编写代码** - 在 `Car_project/project/user/src/` 中编写或修改代码
2. ✅ **自动编译** - 使用命令行编译工程
3. ✅ **检查错误** - 查看编译输出,确认是否有error
4. ⚠️ **如果有错误** - 分析错误信息,修改代码,重新编译
5. ✅ **确保通过** - 重复步骤2-4,直到编译成功(0 errors)
6. ✅ **Git提交和推送** - 编译通过后提交代码到GitHub
7. ✅ **更新日志** - 在 `DEVELOPMENT_LOG.md` 中记录完成的工作

**编译命令** (Car_project):
```bash
"D:\IAR\common\bin\iarbuild.exe" "C:\Users\ASUS\Desktop\code\CYT2BL3_Library-master\CYT2BL3_Library-master\Car_project\project\iar\project_config\cyt2bl3.ewp" -make Debug -log info -parallel 4
```

**编译成功标准**:
- ✅ Total number of errors: **0**
- ⚠️ Warnings可以存在,但error必须为0
- ✅ Build succeeded

**Git提交和推送流程** (在Car_project目录下):
```bash
# 1. 查看修改状态
cd Car_project && git status

# 2. 添加修改的文件
git add .

# 3. 提交（使用描述性的提交信息）
git commit -m "提交信息

详细说明修改内容

🤖 Generated with Claude Code (https://claude.ai/code)

Co-Authored-By: Claude <noreply@anthropic.com>"

# 4. 推送到GitHub
git push origin main
```

**GitHub仓库信息**:
- 仓库地址: `git@github.com:1711789045/suowei_learning.git`
- 分支: `main`
- 认证方式: SSH

**注意事项**:
- ⛔ **禁止提交有编译错误的代码**
- ⚠️ 每次代码修改后必须验证编译通过
- 📝 编译错误必须立即修复,不能遗留
- 🔒 **每次任务完成后必须提交并推送到GitHub**
- 📋 提交信息要清晰描述修改内容

---

## 项目概述

这是逐飞科技基于英飞凌CYT2BL3（Traveo II）MCU开发的开源库,专为智能车竞赛和产品开发设计。该库在英飞凌官方SDL基础上进行了封装,简化了API使用。

**硬件平台**: Infineon CYT2BL3 (Traveo II系列, ARM Cortex-M4/M7双核)
**开发工具**: IAR Embedded Workbench for ARM 9.40.1
**许可协议**: GPL 3.0

## 代码库结构

```
CYT2BL3_Library-master/
├── Example/                                  # 示例程序
│   ├── Coreboard_Demo/                      # 核心板基础示例(GPIO/UART/ADC/PWM/PIT等)
│   └── Motherboard_Demo/                    # 主板外设示例(电机/IMU/显示屏/摄像头/无线通信等)
├── Seekfree_CYT2BL3_Opensource_Library/     # 逐飞开源库(推荐用于新项目)
│   ├── libraries/
│   │   ├── sdk/                             # 英飞凌官方SDK
│   │   │   ├── common/                      # 通用驱动(GPIO/UART/SPI/I2C/ADC/PWM/Timer等)
│   │   │   └── tviibh4m/                    # CYT2BL3芯片特定配置
│   │   ├── zf_driver/                       # 逐飞底层驱动封装
│   │   ├── zf_device/                       # 逐飞外设设备驱动
│   │   ├── zf_components/                   # 逐飞组件(上位机通信等)
│   │   └── zf_common/                       # 逐飞公共模块(时钟/调试/中断等)
│   └── project/                             # 空白工程模板
└── Car_project/                              # 智能车项目模板
```

## 架构说明

### 三层架构
1. **SDK层** (`libraries/sdk/`): 英飞凌官方底层驱动
   - 直接操作寄存器的HAL层函数
   - 位于 `sdk/common/src/drivers/`
   - 包含所有外设的原始驱动

2. **逐飞驱动层** (`libraries/zf_driver/`): 对SDK的高层封装
   - 统一的API接口,简化初始化和使用
   - 命名规则: `zf_driver_xxx.h/c`
   - 常用驱动: GPIO, UART, SPI, I2C, ADC, PWM, Timer, Encoder, Flash等

3. **逐飞设备层** (`libraries/zf_device/`): 外设设备驱动
   - 基于驱动层实现的硬件模块
   - 命名规则: `zf_device_xxx.h/c`
   - 包含: 显示屏(IPS200/IPS114/TFT180), IMU(IMU660RA/IMU963RA), 摄像头(MT9V03X), 无线模块等

### 双核架构
CYT2BL3是双核MCU (CM4核 + CM7核):
- 主代码运行在CM4核 (`main_cm4.c`)
- 中断服务程序在 `cm4_isr.c`
- 需要通过IPC进行核间通信(如使用CM7核)

### 头文件包含
所有用户代码只需包含一个头文件:
```c
#include "zf_common_headfile.h"
```
此头文件自动包含了所有SDK、驱动层、设备层的头文件。

## 开发环境

### IDE配置
- **推荐版本**: IAR EW for ARM 9.40.1
- **工程文件**: `*.eww` (工作区文件)和 `*/iar/project_config/cyt2bl3.ewp` (项目文件)
- **调试器**: CMSIS-DAP ARM调试器

### 打开工程
1. 使用IAR打开对应示例目录下的 `iar/cyt2bl3.eww` 文件
2. 例如打开GPIO示例: `Example/Coreboard_Demo/E01_gpio_demo/iar/cyt2bl3.eww`

### 编译和下载

#### IAR IDE编译
- **编译**: 在IAR中点击 `Project -> Make` 或按 F7
- **下载**: 在IAR中点击 `Project -> Download and Debug` 或按 Ctrl+D
- **清理**: 在IAR中点击 `Project -> Clean` 清理编译缓存

#### 命令行编译（推荐用于Claude Code开发）

**IAR Build工具路径**:
```
D:\IAR\common\bin\iarbuild.exe
```

**编译Car_project工程**:
```bash
"D:\IAR\common\bin\iarbuild.exe" "C:\Users\ASUS\Desktop\code\CYT2BL3_Library-master\CYT2BL3_Library-master\Car_project\project\iar\project_config\cyt2bl3.ewp" -make Debug -log info -parallel 4
```

**命令参数说明**:
- `<project>.ewp` - IAR项目文件路径
- `-make Debug` - 编译Debug配置
- `-log info` - 显示详细日志
- `-parallel 4` - 使用4个并行任务加速编译

**清理项目**:
```bash
"D:\IAR\common\bin\iarbuild.exe" "<project>.ewp" -clean Debug
```

**编译输出说明**:
- 成功: `Build succeeded` + `Total number of errors: 0`
- 失败: 会显示具体的error和warning信息
- 生成文件位于: `car_project/project/iar/Debug/Exe/`
  - `cyt2bl3.out` - ELF可执行文件
  - `cyt2bl3.hex` - 十六进制固件文件

### 重要注意事项
**切换工程或移动工程目录后必须执行以下步骤**:
1. 关闭所有打开的文件
2. 执行 `Project -> Clean` 清理编译产物
3. 等待清理完成后重新编译

## 常用API参考

### GPIO操作
```c
// 初始化GPIO
gpio_init(P23_7, GPO, GPIO_LOW, GPO_PUSH_PULL);  // 输出模式
gpio_init(P11_0, GPI, GPIO_HIGH, GPI_PULL_UP);   // 输入模式

// 读写GPIO
gpio_set_level(P23_7, GPIO_HIGH);
uint8 level = gpio_get_level(P11_0);
```

### UART通信
```c
// 初始化调试串口
debug_init();
// 使用printf输出(已重定向到调试串口)
printf("Hello CYT2BL3\n");
```

### 系统时钟
```c
// 在main函数开始时初始化系统时钟(必须)
clock_init(SYSTEM_CLOCK_160M);  // 设置系统时钟为160MHz
```

### 延时函数
```c
system_delay_ms(1000);  // 延时1000ms
system_delay_us(100);   // 延时100us
```

## 示例程序说明

### 核心板基础示例 (Coreboard_Demo)
- **E01_gpio_demo**: GPIO输入输出示例
- **E02_uart_demo**: UART串口通信
- **E03_adc_demo**: ADC模数转换
- **E04_pwm_demo**: PWM输出
- **E05_pit_demo**: 周期中断定时器
- **E06_exit_demo**: 外部中断
- **E07_encoder_demo**: 编码器接口
- **E08_flash_demo**: Flash读写
- **E09_timer_demo**: 定时器
- **E10_printf_debug_log_demo**: 调试日志输出
- **E11_interrupt_priority_set_demo**: 中断优先级设置

### 主板外设示例 (Motherboard_Demo)
- **E2_encoder**: 编码器(方向型/正交型)
- **E3_motor**: 电机驱动(HIP4082/DRV8701E)
- **E4_imu**: 惯性测量单元(IMU660RA/IMU963RA)
- **E5_display**: 各类显示屏(TFT180/IPS114/IPS200/IPS200Pro)
- **E6_wireless**: 无线通信(蓝牙/LoRa/WiFi)
- **E7_gnss_range**: GNSS定位模块
- **E8_camera**: 摄像头(MT9V03X/TSL1401)

## 新建项目

推荐从以下模板开始:
1. **空白工程**: 复制 `Seekfree_CYT2BL3_Opensource_Library/project/` 目录
2. **智能车项目**: 复制 `Car_project/` 目录
3. **参考示例**: 复制 `Example/` 中相关示例

## 调试和故障排除

### 常见问题
1. **LED不闪烁**: 检查GPIO引脚配置,测量引脚电压
2. **按键无响应**: 检查按键引脚配置,测量按键引脚电压
3. **编译错误**: 执行 `Project -> Clean` 后重新编译
4. **下载失败**: 检查调试器连接,检查IDE版本是否为9.40.1

### 引脚定义
引脚命名格式: `Pxx_y` (如 `P23_7` 表示Port 23的第7个引脚)
具体引脚功能参考核心板原理图和芯片数据手册。

## 参考资料

- 芯片数据手册: `【文档】说明书 芯片手册等/核心板文档/`
- 开源库文档: `libraries/doc/`
- 逐飞科技淘宝店: https://seekfree.taobao.com/
- 许可证: 查看 `libraries/LICENSE` 文件

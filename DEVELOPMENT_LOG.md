# 缩微车模开发日志

本文档记录Car_project项目的开发历史和当前进度,方便后续开发时快速了解项目状态。

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

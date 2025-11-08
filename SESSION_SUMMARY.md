# 智能车项目对话总结

本文档记录了项目开发过程中的重要对话内容和系统状态，方便新对话窗口快速了解项目当前状态。

**最后更新日期**: 2025-11-07

---

## ? 本次完成的主要任务

### 1. 十字识别优化（低角度摄像头适配）

**问题**：原代码适配高角度摄像头，图像上端是赛道；现摄像头低角度，图像上端是杂物

**解决方案**：

#### 简单版十字（cross_enable=1）
- 搜索范围改为 `CROSS_TOP_LIMIT-115`行

#### 复杂版老十字（cross_enable=2）
- 所有边界检查改为 `≥CROSS_TOP_LIMIT`

#### 统一宏定义
```c
#define CROSS_TOP_LIMIT 40  // 可调整，原50
```

### 2. 三档十字开关

**功能**：
- `cross_enable = 0`：关闭十字识别
- `cross_enable = 1`：简单版十字（MM32版本，基于宽度判断）
- `cross_enable = 2`：复杂版老十字（状态机版本，四点检测法）

**实现**：
```c
if(cross_enable == 1 && !circle_flag){
    image_cross_analysis();  // 简单版
}
else if(cross_enable == 2 && !circle_flag){
    image_cross_analysis_OLD();  // 复杂版
}
```

### 3. 出界检测优化

**问题**：旧方案 `prospect < 5` 太严格，经常误触发

**解决方案**：
- **动态白点参考**：从 `reference_col` 列底部往上取5个点平均
- **检测区域**：底部2行×中央40列（80像素）
- **对比度计算**：使用项目统一的差比和公式
- **黑点占比**：>70% (56/80) 判定出界
- **连续性判断**：连续5帧才停车（约40ms）
- **新增开关**：`stop_enable`（可通过菜单关闭）
- **核心函数**：`image_out_of_bounds()`

### 4. basic_speed参数保护

**问题**：发车状态机直接修改 `basic_speed`（0→加速→目标），中途停车时参数丢失

**解决方案**：
- 引入 `current_running_speed` 变量（运行速度）
- `basic_speed`：用户配置参数（只在菜单中修改，发车时不变）
- `current_running_speed`：实际运行速度（发车状态机控制）
- 配置与运行状态完全解耦

### 5. 十字角点可视化

**功能**：在图像显示时用橙色圆圈标记四个角点

**实现**：
- 橙色定义：`#define RGB565_ORANGE 0xFD20`
- 显示函数：`draw_corner_circle()`（半径3像素）
- 角点：`Left_Up_Find`, `Left_Down_Find`, `Right_Up_Find`, `Right_Down_Find`
- 两种十字都显示：简单版存储角点到全局变量

**图例**：
- ? 红色点   - 左边线
- ? 蓝色点   - 右边线
- ? 黄色点   - 参考线
- ? 绿色点   - 中线
- ? 橙色圆圈 - 十字角点

### 6. 简单版十字鲁棒性优化

#### 6.1 动态限制上拐点搜索范围

**问题**：上下拐点搜索范围重叠，可能找到同一个点

**解决**：
- 先找下拐点（从下往上）
- 如果下拐点 > `CROSS_TOP_LIMIT+10`：上拐点搜索 = (下拐点-10) → `CROSS_TOP_LIMIT`
- 距离检查：上下拐点距离必须 ≥10行

#### 6.2 延伸补线优化

**问题**：十字接弯道或双十字时，参考点在异常区域，斜率反向

**解决**：
- **近端多点采样**：从拐点上方取3个点（-2, -3, -4行）
- **斜率方向检查**：
  - 左边线：slope ∈ [-2, -0.4]（向左延伸）
  - 右边线：slope ∈ [0.4, 2]（向右延伸）
- **异常斜率剔除**，有效斜率平均
- **兜底连线**：全部异常时连接到图像下角

**新函数签名**：
```c
void image_stretch_point(array, num, direction, is_left)
```

---

## ? 工作流程规则（必须遵循）

### 每次修改代码的强制流程

1. ? **编写代码**
2. ? **编译验证**（IAR命令行）
   ```bash
   cd "C:\Users\ASUS\Desktop\code\CYT2BL3_Library-master\CYT2BL3_Library-master\Car_project"
   & "D:\IAR\common\bin\iarbuild.exe" "project\iar\project_config\cyt2bl3.ewp" -make Debug -log info -parallel 4
   ```
3. ? **检查编译错误**（必须0 errors）
4. ?? **如果有错误** → 修改代码 → 重新编译
5. ? **编译通过后提交到Git**
6. ? **推送到GitHub**
7. ? **更新DEVELOPMENT_LOG.md**（可选）

### Git提交规范

```bash
git add <files>
git commit -m "标题

详细说明

Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>"
git push origin main
```

---

## ? Init操作（新对话窗口必读）

### 第一优先级（项目规则和历史）

1. `CYT2BL3_Library-master/Car_project/CLAUDE.md` - 开发工作流程和编译命令
2. `CYT2BL3_Library-master/Car_project/DEVELOPMENT_LOG.md` - 项目当前状态
3. `CYT2BL3_Library-master/Car_project/SESSION_SUMMARY.md` - 本文档

### 第二优先级（根据任务需要读取）

- `project/code/control.c/h` - 发车控制（状态机）
- `project/code/motor.c/h` - 电机控制（串级PID）
- `project/code/pid.c/h` - PID参数
- `project/code/encoder.c/h` - 编码器（卡尔曼滤波）
- `project/code/filter.c/h` - 滤波算法库
- `project/code/image.c/h` - 图像处理（十字识别）
- `project/code/menu.c/h` - 菜单系统
- `project/code/flash.c/h` - Flash配置
- `project/user/main_cm4.c` - 主函数
- `project/user/cm4_isr.c` - 中断服务

---

## ? 当前系统状态

### Git仓库

- **地址**：`git@github.com:1711789045/suowei_learning.git`
- **分支**：`main`
- **最新Commit**：`1a0ea7e` - 统一CROSS_TOP_LIMIT宏定义

### Flash配置版本

- **CONFIG_VERSION** = 4
- **配置参数总数**：16项

### 核心配置参数（16项）

1. `direction_kp2`（方向环二次项）
2. `direction_kp`（方向环比例系数）
3. `direction_ki`（方向环积分系数）
4. `direction_kd_img`（方向环微分系数）
5. `direction_kd_g`（陀螺仪系数）
6. `inner_ratio`（内轮减速系数）
7. `outer_ratio`（外轮加速系数）
8. `speed_left_kp`（左轮比例系数）
9. `speed_left_ki`（左轮积分系数）
10. `speed_left_kd`（左轮微分系数）
11. `basic_speed`（基础速度）
12. `mid_weight_select`（中线权重选择）
13. `cross_enable`（十字开关）
14. `speed_right_kp`（右轮比例系数）
15. `speed_right_ki`（右轮积分系数）
16. `speed_right_kd`（右轮微分系数）

---

## ?? 核心系统架构

### 发车状态机（非阻塞）

```
├── START_IDLE: 空闲
├── START_ALIGN: 原地调整（1秒，current_running_speed=0）
├── START_ACCEL: 线性加速（1秒，0%→100%）
└── START_RUNNING: 正常运行
```

### 串级PID控制

```
├── 方向环（外环，10ms，位置式PID）
│   └── 非线性控制：Kp2×|e|×e + Kd_g×gyro_z
└── 速度环（内环，5ms，增量式PID，左右轮独立）
    └── 使用 current_running_speed（不修改basic_speed）
```

### 图像处理

```
├── 十字识别：
│   ├── 简单版（cross_enable=1）：宽度判断+角点检测
│   │   ├── 上方限制：CROSS_TOP_LIMIT（可调，当前40）
│   │   ├── 动态搜索：先下后上，限制上拐点范围
│   │   └── 延伸优化：近端多点平均+斜率检查+兜底连线
│   └── 复杂版（cross_enable=2）：状态机+四点检测
│       └── 上方限制：CROSS_TOP_LIMIT
├── 出界检测：动态白点参考+对比度计算+连续5帧
└── 编码器滤波：卡尔曼（Q=0.01, R=2.0）
```

### 系统计时

```
└── system_time_ms（PIT中断每5ms递增）
```

---

## ? 重要全局变量

### 控制模块

- `car_running`：运行状态
- `stop_enable`：停车检测开关
- `current_running_speed`：实际运行速度（发车状态机控制）
- `basic_speed`：用户配置参数（只在菜单中修改）

### 图像模块

- `cross_enable`：十字开关（0/1/2）
- `CROSS_TOP_LIMIT`：十字上方限制（宏定义，可调）
- `Left/Right_Up/Down_Find`：四个角点变量

---

## ?? 重要规则和注意事项

1. **禁止提交有编译错误的代码**
2. **每次任务完成后必须提交并推送到GitHub**
3. **修改配置参数数量时版本号+1**
4. **使用状态机而非延时阻塞主循环**
5. **编码器硬件计数器必须在motor_reset()中清零**
6. **VOFA数据只在调试模式发送（!car_running）**
7. **电机DIR配置不要修改（已与硬件确认）**
8. **十字角点只在mode=1时显示（调试模式）**
9. **basic_speed只读，运行速度用current_running_speed**

---

## ? 快速参考

- **编译状态**：? 0 errors, 23 warnings
- **Flash版本**：4
- **参数总数**：16项
- **十字上方限制**：`CROSS_TOP_LIMIT = 40`（可调）
- **边线延长数**：`stretch_num = 60`

### 最新功能

- ? 十字识别从CROSS_TOP_LIMIT行开始
- ? 三档十字开关（0/1/2）
- ? 动态出界检测+连续性判断
- ? basic_speed参数保护
- ? 角点可视化（橙色圆圈）
- ? 简单版十字鲁棒性优化（动态搜索+延伸斜率检查）

---

## ? 新窗口开始时执行

1. 读取 `CLAUDE.md`（工作流程）
2. 读取 `DEVELOPMENT_LOG.md`（项目历史）
3. 读取本总结（当前状态）
4. 根据任务读取相关代码文件

---

## ? 更新日志

| 日期 | 更新内容 | 更新人 |
|------|---------|--------|
| 2025-11-07 | 创建对话总结文档 | Claude |

---

**注意**：本文档应在每次重大对话结束后更新，保持与项目实际状态同步。



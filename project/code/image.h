/*********************************************************************************************************************
* CYT2BL3 Opensourec Library 即（CYT2BL3 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是CYT2BL3 开源库的一部分
*
* CYT2BL3 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          image
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-01-28       Claude             图像处理模块（从龙芯C车移植）
********************************************************************************************************************/

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "zf_common_headfile.h"

// ==================== 图像分辨率配置 ====================
#define IMAGE_W            MT9V03X_W       // 188（MT9V03X摄像头宽度）
#define IMAGE_H            MT9V03X_H       // 120（MT9V03X摄像头高度）

// ==================== 图像处理配置参数声明 ====================
// 这些参数在 image.c 中定义，可被 config_flash 等模块引用
extern uint8 reference_row;        // 参考点统计行数
extern uint8 cfg_reference_col;    // 参考列统计列数（配置参数）
extern uint8 whitemaxmul;          // 白点最大值相对参考点的倍数 10为倍数单位
extern uint8 whiteminmul;          // 白点最小值相对参考点的倍数 10为倍数单位
extern uint8 blackpoint;           // 黑点值
extern uint8 contrastoffset;       // 对比度偏移
extern uint8 stoprow;              // 搜索停止行
extern uint8 searchrange;          // 搜索半径
extern uint16 circle_1_time;       // 环岛状态一延时时间，单位10ms
extern uint16 circle_2_time;       // 环岛状态二延时时间，单位10ms
extern uint16 circle_4_time;       // 环岛状态四延时时间，单位10ms
extern uint16 circle_5_time;       // 环岛状态五延时时间，单位10ms
extern uint8 stop_analyse_line;    // 停止线分析行（从底部数）
extern uint8 stop_threshold;       // 停止线检测阈值
extern uint8 stretch_num;          // 边线延长数
extern uint8 mid_calc_center_row;  // 中线计算中心行（从底部数）
extern uint16 mid_weight_select;   // 权重数组选择（1-5）
extern uint16 cross_enable;        // 十字识别开关（0=关闭, 1=开启）

// ==================== 动态前瞻权重配置参数声明 ====================
extern uint16 dynamic_weight_enable;    // 动态权重开关（0=关闭, 1=开启）
extern uint16 curvature_far_threshold;  // 曲率远阈值（小于此值切换到远前瞻）
extern uint16 curvature_near_threshold; // 曲率近阈值（大于此值切换到近前瞻）
extern uint16 weight_hold_time;         // 权重保持时间（帧数，防止频繁切换）
extern uint16 weight_shift_speed;       // 权重切换速度（1-10，值越大切换越快）
extern float curvature_filter_ratio;   // 曲率滤波系数（0-1，越大越平滑）

// ==================== 动态前瞻权重运行时变量声明 ====================
extern float current_curvature;        // 当前赛道曲率（滤波后）
extern float raw_curvature;            // 原始曲率值（未滤波）
extern uint8 dynamic_weight_status;    // 当前权重状态（0=远前瞻 1=近前瞻）
extern uint8 dynamic_weight_target[IMAGE_H];  // 目标权重数组

// 向后兼容的宏定义（引用全局变量）
#define REFERENCE_ROW       reference_row
#define REFERENCE_COL       cfg_reference_col
#define WHITEMAXMUL         whitemaxmul
#define WHITEMINMUL         whiteminmul
#define BLACKPOINT          blackpoint
#define CONTRASTOFFSET      contrastoffset
#define STOPROW             stoprow
#define SEARCHRANGE         searchrange
#define CIRCLE_1_TIME       circle_1_time
#define CIRCLE_2_TIME       circle_2_time
#define CIRCLE_4_TIME       circle_4_time
#define CIRCLE_5_TIME       circle_5_time
#define STOP_ANALYSE_LINE   (IMAGE_H-stop_analyse_line)
#define STOP_THRESHOLD      stop_threshold
#define STRETCH_NUM         stretch_num
#define MID_CALC_CENTER_ROW mid_calc_center_row
#define MID_WEIGHT_SELECT   mid_weight_select

extern uint8 reference_point;
extern uint8 white_max_point;          // 动态白点最大值
extern uint8 white_min_point;          // 动态白点最小值
extern uint8 remote_distance[IMAGE_W]; // 远距离数组
extern uint8 reference_col;            // 运行时变量（每帧动态更新）
extern uint8 reference_contrast_ratio; // 参考点对比度
extern uint8 stop_search_row;          // 搜线截止行（边线搜索的截止行号）
extern uint16 reference_line[IMAGE_H];      // 存储参考线
extern uint16 left_edge_line[IMAGE_H];      // 存储左边线
extern uint16 right_edge_line[IMAGE_H];     // 存储右边线
extern uint8 final_mid_line;
extern uint8 prospect;                     // 前瞻值
extern uint8 single_edge_err[IMAGE_H];
extern uint8 circle_flag;                  // 环岛标志位
extern uint16 circle_time;
extern uint8 cross_flag;                   // 十字标志位
extern uint8 mid_mode;                    // 循迹模式
extern uint8 if_circle;                     // 1为进入圆环，0为不进入圆环

// 预设权重数组（1-5，用于不同场景）
extern uint8 mid_weight_1[IMAGE_H];
extern uint8 mid_weight_2[IMAGE_H];
extern uint8 mid_weight_3[IMAGE_H];
extern uint8 mid_weight_4[IMAGE_H];
extern uint8 mid_weight_5[IMAGE_H];
extern uint8 mid_weight[IMAGE_H];          // 当前使用的权重数组

// ==================== 新十字补线相关变量声明 ====================
extern int16 continuity_pointLeft[2];      // 左不连续点[0]存某行，[1]存某列
extern int16 continuity_pointRight[2];     // 右不连续点[0]存某行，[1]存某列
extern int16 Right_Down_Find;              // 右下点
extern int16 Left_Down_Find;               // 左下点
extern int16 Right_Up_Find;                // 右上点
extern int16 Left_Up_Find;                 // 左上点
extern uint16 leftfollowline[IMAGE_H];     // 左跟随线（补线后的边线）
extern uint16 rightfollowline[IMAGE_H];    // 右跟随线（补线后的边线）
extern float dx1[5];                       // 左边线滑动平均滤波数组
extern float dx2[5];                       // 右边线滑动平均滤波数组

// ==================== 核心API函数 ====================
void get_image(void);
void get_reference_point(const uint8 image[][IMAGE_W]);
void search_reference_col(const uint8 image[][IMAGE_W]);
void search_line(const uint8 image[][IMAGE_W]);
void image_display_edge_line(const uint8 image[][IMAGE_W], uint16 width, uint16 height);
void image_process(uint16 display_width, uint16 display_height, uint8 mode);
void select_mid_weight(uint16 select);     // 选择使用的权重数组（1-5）

// 动态前瞻权重函数
float calculate_curvature(void);                  // 计算赛道曲率
void adjust_weight_by_curvature(float curvature); // 根据曲率调整权重
void dynamic_weight_adjust(void);                 // 动态前瞻权重主函数

#endif

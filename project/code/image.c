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
* 2025-01-28       Claude             图像处理模块（从龙芯C车移植，适配MT9V03X+IPS114）
********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "image.h"

// ==================== 图像处理配置参数（在本模块中定义）====================
uint8 reference_row = 5;        // 参考点统计行数
uint8 cfg_reference_col = 60;   // 参考列统计列数（配置参数）
uint8 whitemaxmul = 13;         // 白点最大值相对参考点的倍数 10为倍数单位
uint8 whiteminmul = 7;          // 白点最小值相对参考点的倍数 10为倍数单位
uint8 blackpoint = 50;          // 黑点值
uint8 contrastoffset = 3;       // 对比度偏移
uint8 stoprow = 0;              // 搜索停止行
uint8 searchrange = 10;         // 搜索半径
uint16 circle_1_time = 15;      // 环岛状态一延时时间，单位10ms
uint16 circle_2_time = 50;      // 环岛状态二延时时间，单位10ms
uint16 circle_4_time = 25;      // 环岛状态四延时时间，单位10ms
uint16 circle_5_time = 25;      // 环岛状态五延时时间，单位10ms
uint8 stop_analyse_line = 40;   // 停止线分析行（从底部数）
uint8 stop_threshold = 30;      // 停止线检测阈值
uint8 stretch_num = 80;         // 边线延长数
uint8 mid_calc_center_row = 90; // 中线计算中心行（从底部数）
uint16 mid_weight_select = 2;   // 权重数组选择（1-5，默认2）
uint16 cross_enable = 0;        // 十字识别开关（默认关闭）

// ==================== 动态前瞻权重配置参数 ====================
uint16 dynamic_weight_enable = 1;       // 动态权重开关（0=关闭使用固定权重, 1=开启）
uint16 curvature_far_threshold = 3;     // 曲率远阈值（小于此值切换到远前瞻）
uint16 curvature_near_threshold = 12;   // 曲率近阈值（大于此值切换到近前瞻）
uint16 weight_hold_time = 30;           // 权重保持时间（帧数，30帧≈0.3秒）
uint16 weight_shift_speed = 3;          // 权重切换速度（1-10，值越大切换越快）
float curvature_filter_ratio = 0.7;    // 曲率滤波系数（0-1，越大越平滑）

uint8 reference_point = 0;
uint8 white_max_point = 0;
uint8 white_min_point = 0;
uint8 remote_distance[IMAGE_W] = {0};
uint8 reference_col = 0;        // 运行时变量（每帧动态更新）
uint8 reference_contrast_ratio = 0.1*200;
uint16 reference_line[IMAGE_H] = {0};
uint16 left_edge_line[IMAGE_H] = {0};
uint16 right_edge_line[IMAGE_H] = {0};
uint8 user_image[IMAGE_H][IMAGE_W];

uint8 mid_line[IMAGE_H] = {0};

// 预设权重数组1：峰值在第60-70行（默认，中间偏远）
uint8 mid_weight_1[IMAGE_H] = {
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	7 ,8 ,9 ,10,11,12,13,14,15,16,
	17,18,19,20,20,20,20,19,18,17,
	16,15,14,13,12,11,10,9 ,8 ,7 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1
};

// 预设权重数组2：峰值在第50-70行（稍近）
uint8 mid_weight_2[IMAGE_H] = {
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	7 ,8 ,9 ,10,11,12,13,14,15,16,
	17,18,19,20,20,20,20,19,18,17,
	16,15,14,13,12,11,10,9 ,8 ,7 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1
};

// 预设权重数组3：峰值在第40-60行（中间）
uint8 mid_weight_3[IMAGE_H] = {
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	7 ,8 ,9 ,10,11,12,13,14,15,16,
	17,18,19,20,20,20,20,19,18,17,
	16,15,14,13,12,11,10,9 ,8 ,7 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1
};

// 预设权重数组4：峰值在第30-50行（较近）
uint8 mid_weight_4[IMAGE_H] = {
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	7 ,8 ,9 ,10,11,12,13,14,15,16,
	17,18,19,20,20,20,20,19,18,17,
	16,15,14,13,12,11,10,9 ,8 ,7 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1
};

// 预设权重数组5：峰值在第20-40行（最近）
uint8 mid_weight_5[IMAGE_H] = {
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	7 ,8 ,9 ,10,11,12,13,14,15,16,
	17,18,19,20,20,20,20,19,18,17,
	16,15,14,13,12,11,10,9 ,8 ,7 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1
};

// 当前使用的权重数组（默认使用权重数组2）
uint8 mid_weight[IMAGE_H] = {
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	7 ,8 ,9 ,10,11,12,13,14,15,16,
	17,18,19,20,20,20,20,19,18,17,
	16,15,14,13,12,11,10,9 ,8 ,7 ,
	6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1
};

uint8 single_edge_err[IMAGE_H] = {
	11,11,12,13,13,14,15,15,16,17,
	17,18,19,19,20,20,21,22,23,23,
	24,25,25,27,27,28,28,29,30,30,
	31,32,32,33,34,34,35,35,36,37,
	37,38,39,40,40,42,42,42,43,44,
	45,45,46,47,47,48,49,49,50,50,
	51,52,52,53,54,54,55,56,57,57,
	58,59,60,60,61,62,62,63,64,64,
	65,65,66,67,67,68,69,69,70,72,
	71,72,73,74,74,75,76,76,77,78,
	78,79,79,80,80,81,82,82,83,84,
	85,85,86,87,87,88,89,89,90,91
};
uint8 final_mid_line = 0;
uint8 last_final_mid_line = 0;
uint8 prospect = 100;                     // 前瞻值
uint8 cross_flag = 0;                     // 十字标志位(与cross_status同步: 0=直道 1=十字)
uint8 mid_mode = 0;
uint8 circle_flag = 0;
uint16 circle_time = 0;
uint8 if_circle = 0;
uint8 stop_search_row = 0;

// ==================== 动态前瞻权重运行时变量 ====================
float current_curvature = 0.0f;           // 当前赛道曲率（滤波后）
float raw_curvature = 0.0f;               // 原始曲率值（未滤波）
uint8 dynamic_weight_status = 0;          // 当前权重状态（0=远前瞻 1=中等 2=近前瞻）
uint8 dynamic_weight_target[IMAGE_H];     // 目标权重数组（动态计算）

// ==================== 新十字补线相关变量定义 ====================
int16 continuity_pointLeft[2] = {0, 0};    // 左不连续点[0]存某行，[1]存某列
int16 continuity_pointRight[2] = {0, 0};   // 右不连续点[0]存某行，[1]存某列
int16 Right_Down_Find = 0;                 // 右下点
int16 Left_Down_Find = 0;                  // 左下点
int16 Right_Up_Find = 0;                   // 右上点
int16 Left_Up_Find = 0;                    // 左上点
uint16 leftfollowline[IMAGE_H] = {0};      // 左跟随线（补线后的边线）
uint16 rightfollowline[IMAGE_H] = {0};     // 右跟随线（补线后的边线）
float dx1[5] = {0};                        // 左边线滑动平均滤波数组
float dx2[5] = {0};                        // 右边线滑动平均滤波数组

// 十字状态枚举定义
enum CrossStatus {
    CROSS_STRAIGHT,    // 直道行驶
    CROSS_ROAD,        // 十字路口
    CROSS_ROAD_L,      // 左斜入十字
    CROSS_ROAD_R       // 右斜入十字
};
static enum CrossStatus cross_status = CROSS_STRAIGHT;

/**
 * @brief 从MT9V03X摄像头获取图像
 * @param 无
 * @return 无
 * @note 由外部main循环检查mt9v03x_finish_flag，然后调用此函数复制图像
 */
void get_image(void){
	// 直接复制MT9V03X图像数据
	for(uint8 i = 0; i < IMAGE_H; i++){
		for(uint8 j = 0; j < IMAGE_W; j++){
			user_image[i][j] = mt9v03x_image[i][j];
		}
	}
}

/**
 * @brief 选择使用的权重数组
 * @param select 选择编号（1-5）
 *
 * 权重数组说明：
 * - mid_weight_1: 峰值在第60-70行（中间偏远，适合高速）
 * - mid_weight_2: 峰值在第50-70行（稍近，默认）
 * - mid_weight_3: 峰值在第40-60行（中间，平衡）
 * - mid_weight_4: 峰值在第30-50行（较近，适合弯道）
 * - mid_weight_5: 峰值在第20-40行（最近，适合急弯）
 */
void select_mid_weight(uint16 select){
	// 限制选择范围在1-5
	select = func_limit_ab(select, 1, 5);
	mid_weight_select = select;

	// 根据选择复制对应的权重数组
	switch(select){
		case 1:
			memcpy(mid_weight, mid_weight_1, IMAGE_H);
			break;
		case 2:
			memcpy(mid_weight, mid_weight_2, IMAGE_H);
			break;
		case 3:
			memcpy(mid_weight, mid_weight_3, IMAGE_H);
			break;
		case 4:
			memcpy(mid_weight, mid_weight_4, IMAGE_H);
			break;
		case 5:
			memcpy(mid_weight, mid_weight_5, IMAGE_H);
			break;
		default:
			// 默认使用权重数组2
			memcpy(mid_weight, mid_weight_2, IMAGE_H);
			break;
	}
}

void get_reference_point(const uint8 image[][IMAGE_W]){
	uint16 temp = 0;
	uint32 temp1 = 0;
	uint16 temp_j1 = (IMAGE_W-REFERENCE_COL)/2;
	uint16 temp_j2 = (IMAGE_W+REFERENCE_COL)/2;

	temp = REFERENCE_ROW * REFERENCE_COL;
	for(int i = IMAGE_H-REFERENCE_ROW;i <= IMAGE_H-1;i++){
		for(int j = temp_j1;j <= temp_j2;j++){
			temp1 += image[i][j];
		}
	}
	reference_point = (uint8)func_limit_ab((temp1/temp),BLACKPOINT,255);
	white_max_point = (uint8)func_limit_ab((int32)reference_point * WHITEMAXMUL/10,BLACKPOINT,255);
	white_min_point = (uint8)func_limit_ab((int32)reference_point * WHITEMINMUL/10,BLACKPOINT,255);
}

void search_reference_col(const uint8 image[][IMAGE_W]){
	int col,row;
	int16 temp1 = 0,temp2 = 0,temp3 = 0;
	for(col = 0;col < IMAGE_W;col++){
		remote_distance[col] = IMAGE_H-1;
	}
	for(col = 0;col < IMAGE_W;col+=CONTRASTOFFSET){
		for	(row = IMAGE_H-1;row >= STOPROW+CONTRASTOFFSET;row-=CONTRASTOFFSET){
			temp1 = image[row][col];
			temp2 = image[row-CONTRASTOFFSET][col];

			if(row == 5){
				remote_distance[col] = (uint8)row;
				break;
			}

			if(temp2 > white_max_point){
				continue;
			}
			if(temp1 < white_min_point){
				remote_distance[col] = (uint8)row;
				break;
 			}

			// 防止除零错误
			if(temp1 + temp2 == 0){
				continue;
			}
			temp3 = (temp1 - temp2)*200/(temp1 + temp2);

			if(temp3 >reference_contrast_ratio ){
				remote_distance[col] = (uint8)row;
				break;
			}
		}
	}

	for(col = 0;col < IMAGE_W;col+=CONTRASTOFFSET){
		if(remote_distance[reference_col] > remote_distance[col])
			reference_col = col;
	}


	for(row = 0;row < IMAGE_H;row++){
		reference_line[row] =reference_col;
	}
}

void search_line(const uint8 image[][IMAGE_W]){
    uint8 row_max = IMAGE_H - 1;
    uint8 row_min = STOPROW;
    uint8 col_max = IMAGE_W - CONTRASTOFFSET-1;
    uint8 col_min = CONTRASTOFFSET;
    int16 leftstartcol  = reference_col;
    int16 rightstartcol = reference_col;
    int16 leftendcol    = col_min;
    int16 rightendcol   = col_max;
    uint8 search_time   = 0;
    uint8 temp1 = 0, temp2 = 0;
    int  temp3 = 0;
    int  leftstop = 0, rightstop = 0, stoppoint = 0;

    int col, row;

    for(row = row_max; row >= row_min; row --){
		left_edge_line[row]  = col_min - CONTRASTOFFSET;
        right_edge_line[row] = col_max + CONTRASTOFFSET;
    }
	stop_search_row = 0;

	for(row = row_max;row >= row_min;row--){
		if(!leftstop){
			// 边缘跟随优化控制:
			// - cross_flag=0(直道): search_time=2, 优先跟随上一行位置搜索,提高效率
			// - cross_flag=1(十字): search_time=1, 禁用跟随直接整行搜索,确保找到远处上拐点
			search_time = 2-cross_flag;
			do{
				if(search_time == 1){
					leftstartcol = reference_col;
					leftendcol = col_min;
				}

				search_time--;

				for(col = leftstartcol; col >= leftendcol ;col--){
					if(col - CONTRASTOFFSET <0)
						continue;
					temp1 = image[row][col];
					temp2 = image[row][col - CONTRASTOFFSET];

					// 判断参考列是否为黑点，若是则左右边线都不再搜索
					if(temp1 < white_min_point && col == leftstartcol && leftstartcol == reference_col){
						leftstop = 1;
						rightstop = 1;  // 左右对称：同时停止右边搜索
						stop_search_row = row;

						for(stoppoint = row;stoppoint >= 0;stoppoint--){
							left_edge_line[stoppoint ] = col_min;
							right_edge_line[stoppoint ] = col_max;  // 同时填充右边线
						}
						search_time = 0;
						break;
					}

					if(temp1 < white_min_point){
						left_edge_line[row] = col;
						break;
					}
					if(temp2 > white_max_point){
						continue;
					}

					// 防止除零错误
					if(temp1 + temp2 == 0){
						continue;
					}
					temp3 = (temp1 - temp2)*200/(temp1 + temp2);

					if(temp3 >reference_contrast_ratio || col == col_min){
						left_edge_line[row] = func_limit_ab(col - CONTRASTOFFSET, 0, IMAGE_W-1);
						leftstartcol = (uint8)func_limit_ab(col+SEARCHRANGE,col,col_max);
						leftendcol = (uint8)func_limit_ab(col-SEARCHRANGE,col_min,col);
						search_time = 0;
						break;
					}
				}
			}while(search_time);
		}

		if(!rightstop){
			// 边缘跟随优化控制(同左边):
			// - cross_flag=0(直道): search_time=2, 优先跟随上一行位置搜索,提高效率
			// - cross_flag=1(十字): search_time=1, 禁用跟随直接整行搜索,确保找到远处上拐点
			search_time = 2-cross_flag;
			do{
				if(search_time == 1){
					rightstartcol = reference_col;
					rightendcol = col_max;
				}

				search_time--;

				for(col = rightstartcol; col <= rightendcol ;col++){
					if(col + CONTRASTOFFSET >= IMAGE_W)
						continue;
					temp1 = image[row][col];
					temp2 = image[row][col + CONTRASTOFFSET];

					// 判断参考列是否为黑点，若是则左右边线都不再搜索
					if(temp1 < white_min_point && col == rightstartcol && rightstartcol == reference_col){
						leftstop = 1;  // 左右对称：同时停止左边搜索
						rightstop = 1;
						stop_search_row = row;

						for(stoppoint = row;stoppoint >= 0;stoppoint--){
							left_edge_line[stoppoint ] = col_min;  // 同时填充左边线
							right_edge_line[stoppoint ] = col_max;
						}
						search_time = 0;
						break;
					}

					if(temp1 < white_min_point){
						right_edge_line[row] = col;
						break;
					}
					if(temp2 > white_max_point){
						continue;
					}

					// 防止除零错误
					if(temp1 + temp2 == 0){
						continue;
					}
					temp3 = (temp1 - temp2)*200/(temp1 + temp2);

					if(temp3 >reference_contrast_ratio || col == col_max){
						right_edge_line[row] = func_limit_ab(col + CONTRASTOFFSET, 0, IMAGE_W-1);
						rightstartcol = (uint8)func_limit_ab(col-SEARCHRANGE,col_min,col);
						rightendcol = (uint8)func_limit_ab(col+SEARCHRANGE,col,col_max);
						search_time = 0;
						break;
					}
				}
			}while(search_time);
		}
	}
}

uint8 image_find_jump_point(uint16 *edge_line,uint8 down_num,uint8 up_num,uint8 jump_num,uint8 model){
	uint8 temp_jump_point = 0;
	uint8 temp_data;

	if(model){
		temp_jump_point = down_num;
		for(int i = 0;i <down_num-up_num;i++){
			temp_data = func_abs(edge_line[down_num-i]-edge_line[down_num-i-1]);
			if(temp_data>jump_num){
				temp_jump_point = (uint8)(down_num-i);
				return temp_jump_point;
			}
		}
	}
	else{
		temp_jump_point = up_num;
		for(int i = 0;i <down_num-up_num;i++){
			temp_data = func_abs(edge_line[up_num+i]-edge_line[up_num+i+1]);
			if(temp_data>jump_num){
				temp_jump_point = (uint8)(up_num+i);
				return temp_jump_point;
			}
		}
	}
	return 0;
}

uint8 image_find_left_jump_point(uint8 down_num,uint8 up_num,uint8 model){
	uint8 temp_jump_point = 0;
	uint8 temp_data;

	if(model){
		temp_jump_point = down_num;
		for(int i = 0;i <down_num-up_num;i++){
			if(
				left_edge_line[down_num-i]-left_edge_line[down_num-i-5]>=8&&
				left_edge_line[down_num-i]-left_edge_line[down_num-i-6]>=8&&
				left_edge_line[down_num-i]-left_edge_line[down_num-i-7]>=8
			&&
				func_abs(left_edge_line[down_num-i]-left_edge_line[down_num-i+1])<=7&&
				func_abs(left_edge_line[down_num-i]-left_edge_line[down_num-i+2])<=10&&
				func_abs(left_edge_line[down_num-i]-left_edge_line[down_num-i+3])<=15
			){
				temp_jump_point = (uint8)(down_num-i)+3;
				return temp_jump_point;
			}
		}
	}
	else{
		temp_jump_point = up_num;
		for(int i = 0;i <down_num-up_num;i++){
			if(
				left_edge_line[up_num+i]-left_edge_line[up_num+i+5]>=8&&
				left_edge_line[up_num+i]-left_edge_line[up_num+i+6]>=8&&
				left_edge_line[up_num+i]-left_edge_line[up_num+i+7]>=8
			&&
				func_abs(left_edge_line[up_num+i]-left_edge_line[up_num+i+1])<=7&&
				func_abs(left_edge_line[up_num+i]-left_edge_line[up_num+i+2])<=10&&
				func_abs(left_edge_line[up_num+i]-left_edge_line[up_num+i+3])<=15
			){
				temp_jump_point = (uint8)(up_num+i)-3;
				return temp_jump_point;
			}
		}
	}
	return 0;
}

uint8 image_find_right_jump_point(uint8 down_num,uint8 up_num,uint8 model){
	uint8 temp_jump_point = 0;
	uint8 temp_data;

	if(model){
		temp_jump_point = down_num;
		for(int i = 0;i <down_num-up_num;i++){
			if(
				right_edge_line[down_num-i]-right_edge_line[down_num-i-5]<=-8&&
				right_edge_line[down_num-i]-right_edge_line[down_num-i-6]<=-8&&
				right_edge_line[down_num-i]-right_edge_line[down_num-i-7]<=-8
			&&
				func_abs(right_edge_line[down_num-i]-right_edge_line[down_num-i+1])<=7&&
				func_abs(right_edge_line[down_num-i]-right_edge_line[down_num-i+2])<=10&&
				func_abs(right_edge_line[down_num-i]-right_edge_line[down_num-i+3])<=15
			){
				temp_jump_point = (uint8)(down_num-i)+3;
				return temp_jump_point;
			}
		}
	}
	else{
		temp_jump_point = up_num;
		for(int i = 0;i <down_num-up_num;i++){
			if(
				right_edge_line[up_num+i]-right_edge_line[up_num+i+5]<=-8&&
				right_edge_line[up_num+i]-right_edge_line[up_num+i+6]<=-8&&
				right_edge_line[up_num+i]-right_edge_line[up_num+i+7]<=-8
			&&
				func_abs(right_edge_line[up_num+i]-right_edge_line[up_num+i+1])<=7&&
				func_abs(right_edge_line[up_num+i]-right_edge_line[up_num+i+2])<=10&&
				func_abs(right_edge_line[up_num+i]-right_edge_line[up_num+i+3])<=15
			){
				temp_jump_point = (uint8)(up_num+i)-3;
				return temp_jump_point;
			}
		}
	}
	return 0;
}

void image_connect_point(uint16 *array_value, uint8 num0, uint8 num1)
{
    float point_1 = (float)array_value[num0];
    float point_2 = (float)array_value[num1];
    float temp_slope = (point_2 - point_1) / (num0 - num1);

    for (int i = 0; i < (num0 - num1); i++)
    {
        array_value[num0 - i] = (int8)(temp_slope * i) + array_value[num0];
    }
}

void image_stretch_point(uint16 *array_value, uint8 num ,uint8 direction){

	if((num+5>=IMAGE_H) ||(num-5<=0))
		return;

	float temp_slope = 0;
    float point_1 = (float)array_value[num];

	if(direction){
		float point_2 = (float)array_value[num+5];
		temp_slope = (point_1 - point_2) / 5;
		for (int i = 0; i < STRETCH_NUM&& num-i>=5; i++)
		{
			array_value[num - i] = func_limit_ab((int8)(temp_slope * i) + array_value[num],0,IMAGE_W-1);
		}
	}
	else{
		float point_2 = (float)array_value[num-5];
		temp_slope = (point_1 - point_2) / 5;
		for (int i = 0; i < STRETCH_NUM&& num+i<=IMAGE_H-1; i++)
		{
			array_value[num + i] = func_limit_ab((int8)(temp_slope * i) + array_value[num],0,IMAGE_W-1);
		}
	}
}

uint8 image_find_circle_point(uint16 *edge_line,uint8 down_num,uint8 up_num){
	uint8 temp_jump_point = 0;
	temp_jump_point = down_num;
	for(int i = 0;i <down_num-up_num;i++){
		if(edge_line[down_num-i]<edge_line[down_num-i-1]&&
			edge_line[down_num-i]<edge_line[down_num-i-2]&&
			edge_line[down_num-i]<edge_line[down_num-i-3]&&
			edge_line[down_num-i]<edge_line[down_num-i+1]&&
			edge_line[down_num-i]<edge_line[down_num-i+2]&&
			edge_line[down_num-i]<edge_line[down_num-i+3]
		){
			temp_jump_point = (uint8)(down_num-i);

			return temp_jump_point;
		}
	}
	return 0;
}

void image_circle_analysis(void){

	if(circle_flag == 0){
		mid_mode = 0;
		for(int i = IMAGE_H-2;i>0;i--){
			if(func_abs(left_edge_line[i]-left_edge_line[i+1])>3)
				return;
		}
		for(int i = IMAGE_H-40;i>0;i--){
			if(left_edge_line[i]<3)
				return;
		}
		uint8 start_point = 0,end_point = 0;
		start_point = image_find_jump_point(right_edge_line,IMAGE_H - 5,5,10,1);
		if(start_point)
			end_point = image_find_circle_point(right_edge_line,start_point-10,10);
		if(end_point){
			if(start_point - end_point>30){
				circle_flag = 1;
				circle_time = 0;
				// beep_flag = 1;  // 蜂鸣器功能，CYT2BL3项目中未实现
			}
		}

		start_point = 0,end_point = 0;
		start_point = image_find_jump_point(right_edge_line,IMAGE_H - 80,5,10,0);
		if(start_point)
			end_point = image_find_circle_point(right_edge_line,IMAGE_H - 5,start_point-5);
		if(end_point){
			if(end_point- start_point>30){
				circle_flag = 1;
				circle_time = 5;
				// beep_flag = 1;
			}
		}
	}
	else if (circle_flag == 1){
		mid_mode = 1;
		if(circle_time >= CIRCLE_1_TIME){
			circle_time = 0;
			circle_flag = 2;
			// beep_flag = 1;
		}
	}
	else if (circle_flag == 2){
		mid_mode = 2;
		if(circle_time >= CIRCLE_2_TIME){
			circle_time = 0;
			circle_flag = 3;
			// beep_flag = 1;
		}
	}
	else if (circle_flag == 3){
		mid_mode = 0;
		if(left_edge_line[IMAGE_H/2]<3 && left_edge_line[IMAGE_H/2-1]<3 &&left_edge_line[IMAGE_H/2+1]<3){
			mid_mode = 2;
			circle_time = 0;
			circle_flag = 4;
			// beep_flag = 1;
		}
	}
	else if(circle_flag == 4){
		mid_mode = 0;
		for(int i = IMAGE_H-1;i>0;i--){
			left_edge_line[i] = IMAGE_W/3;
		}
		if(circle_time >= CIRCLE_4_TIME){
			circle_time = 0;
			circle_flag = 5;
			// beep_flag = 1;
		}
	}
	else if(circle_flag == 5){
		mid_mode = 1;
		if(circle_time >= CIRCLE_5_TIME){
			circle_time = 0;
			circle_flag = 0;
		}
	}
}

void image_calculate_mid(uint8 mode){
	uint32 mid_weight_sum = 0;
	uint32 weight_sum = 0;
	uint8 temp = 0;

	// 计算中线（根据模式）
	if(mode == 0){
		for(int i = 0;i<IMAGE_H;i++){
			mid_line[i] = (left_edge_line[i] + right_edge_line[i])/2;
		}
	}
	if(mode == 1){
		for(int i = 0;i<IMAGE_H;i++){
			mid_line[i] = func_limit_ab(left_edge_line[i] + single_edge_err[i],0,IMAGE_W);
		}
	}
	if(mode == 2){
		for(int i = 0;i<IMAGE_H;i++){
			mid_line[i] = func_limit_ab(right_edge_line[i] - single_edge_err[i],0,IMAGE_W);
		}
	}

	// 使用权重数组计算加权平均中线
	for(int i = 0;i<IMAGE_H;i++){
		if(i >= stop_search_row){
			weight_sum += mid_weight[i];
			mid_weight_sum += mid_line[i]*mid_weight[i];
		}
	}
	temp = (uint8)(mid_weight_sum/weight_sum);

	// 低通滤波
	if(!last_final_mid_line)
		final_mid_line = temp;
	else
		final_mid_line = temp*0.8+last_final_mid_line*0.2;

	last_final_mid_line = final_mid_line;
}

void image_calculate_prospect(const uint8 image[][IMAGE_W]){
	int col = IMAGE_W/2;
	int16 temp1 = 0,temp2 = 0,temp3 = 0;
	for	(int row = IMAGE_H-1;row > CONTRASTOFFSET;row--){
		temp1 = image[row][col];
		temp2 = image[row-CONTRASTOFFSET][col];

		if(row == 4){
			prospect = IMAGE_H-(uint8)row;
			break;
		}

		if(temp1 < white_min_point){
			prospect = IMAGE_H-(uint8)row;
			break;
		}

		if(temp2 > white_max_point){
			continue;
		}

		// 防止除零错误
		if(temp1 + temp2 == 0){
			continue;
		}
		temp3 = (temp1 - temp2)*200/(temp1 + temp2);

		if(temp3 >reference_contrast_ratio ){
			prospect = IMAGE_H-(uint8)row;
			break;
		}
	}
}

/**
 * @brief 显示边线和调试信息（适配IPS114）
 * @param image 图像数组
 * @param display_width 显示宽度
 * @param display_height 显示高度
 * @return 无
 */
void image_display_edge_line(const uint8 image[][IMAGE_W], uint16 display_width, uint16 display_height) {
    // 显示灰度图像（使用MT9V03X图像数组）
    ips114_displayimage03x((const uint8 *)mt9v03x_image, MT9V03X_W, MT9V03X_H);

    // 绘制边线和参考线
    for(uint16 i = 0; i < IMAGE_H; i++) {
        if(left_edge_line[i] < display_width && i < display_height) {
            ips114_draw_point((uint16)left_edge_line[i], i, RGB565_RED);
        }

        if(right_edge_line[i] < display_width && i < display_height) {
            ips114_draw_point((uint16)right_edge_line[i], i, RGB565_BLUE);
        }

        if(reference_line[i] < display_width && i < display_height) {
            ips114_draw_point((uint16)reference_line[i], i, RGB565_YELLOW);
        }

        if(mid_line[i] < display_width && i < display_height) {
            ips114_draw_point((uint16)mid_line[i], i, RGB565_GREEN);
        }
    }

    // ==================== 动态前瞻权重调试信息显示 ====================
    // IPS114屏幕宽度240，图像宽度188，调试信息从x=190开始显示
    if(dynamic_weight_enable) {
        ips114_show_string(190, 0, "DWgt:ON");             // 动态权重开关状态
        ips114_show_string(190, 16, "Curv:");              // 曲率标签
        ips114_show_float(190, 32, current_curvature, 1, 1); // 当前曲率（滤波后）
        ips114_show_string(190, 48, "Stat:");              // 状态标签
        ips114_show_uint(190, 64, dynamic_weight_status, 1); // 权重状态（0/1）

        // 状态说明文字
        if(dynamic_weight_status == 0) {
            ips114_show_string(190, 80, "FAR");  // 远前瞻（直道）
        } else {
            ips114_show_string(190, 80, "NEAR"); // 近前瞻（弯道）
        }
    } else {
        ips114_show_string(190, 0, "DWgt:OFF");            // 动态权重关闭
        ips114_show_string(190, 16, "FixWgt:");            // 固定权重标签
        ips114_show_uint(190, 32, mid_weight_select, 1);   // 固定权重编号（1-5）
    }
}

void image_get_left_err(void){
	for(int i = 0;i <= IMAGE_H-1;i++){
		single_edge_err[i] = mid_line[i] - left_edge_line[i];
	}
}

// ==================== 新十字补线辅助函数 ====================

/**
 * @brief 找下面的两个拐点，供十字使用
 * @param start 搜索的范围起点
 * @param end 搜索的范围终点
 * @note 运行完之后查看 Right_Down_Find 和 Left_Down_Find，没找到时为0
 */
void Find_Down_Point(int16 start, int16 end)
{
	int16 i, t;
	Right_Down_Find = 0;
	Left_Down_Find = 0;

	if(start < end) {  // 从下往上找，大的索引在图象下面
		t = start;
		start = end;
		end = t;
	}

	if(start >= IMAGE_H - 1 - 3)  // 下面5行数据不稳定，不能作为边界点来判断，舍弃
		start = IMAGE_H - 1 - 3;
	if(end >= IMAGE_H - 5)
		end = IMAGE_H - 5;
	if(end <= 5)
		end = 5;

	for(i = start; i >= end; i--)
	{
		// 查找左下拐点
		if(Left_Down_Find == 0 &&  // 只找第一个符合条件的点
			(left_edge_line[i] > 0) &&  // 左边界点不能为0
			func_abs(left_edge_line[i] - left_edge_line[i+1]) <= 5 &&
			func_abs(left_edge_line[i+1] - left_edge_line[i+2]) <= 5 &&
			func_abs(left_edge_line[i+2] - left_edge_line[i+3]) <= 5 &&
			((left_edge_line[i] - left_edge_line[i-2]) >= 3 || left_edge_line[i-2] <= 0) &&
			((left_edge_line[i] - left_edge_line[i-3]) >= 5 || left_edge_line[i-3] <= 0) &&
			((left_edge_line[i] - left_edge_line[i-4]) >= 5 || left_edge_line[i-4] <= 0))
		{
			Left_Down_Find = i + 2;  // 获取行数即可
			if(Left_Down_Find == start + 2) {
				Left_Down_Find = 0;  // 如果是起始行，说明没有找到
			}
		}

		// 查找右下拐点
		if(Right_Down_Find == 0 &&  // 只找第一个符合条件的点
			func_abs(right_edge_line[i] - right_edge_line[i+1]) <= 5 &&
			func_abs(right_edge_line[i+1] - right_edge_line[i+2]) <= 5 &&
			func_abs(right_edge_line[i+2] - right_edge_line[i+3]) <= 5 &&
			right_edge_line[i] < IMAGE_W - 1 &&  // 右边界点不能为IMAGE_W-1
			((right_edge_line[i] - right_edge_line[i-2]) <= -3 || right_edge_line[i-2] > IMAGE_W - 2) &&
			((right_edge_line[i] - right_edge_line[i-3]) <= -5 || right_edge_line[i-3] > IMAGE_W - 2) &&
			((right_edge_line[i] - right_edge_line[i-4]) <= -5 || right_edge_line[i-4] > IMAGE_W - 2))
		{
			Right_Down_Find = i + 2;
			if(Right_Down_Find == start + 2) {
				Right_Down_Find = 0;  // 如果是起始行，说明没有找到
			}
		}

		if(Left_Down_Find != 0 && Right_Down_Find != 0)  // 两个找到就退出
		{
			break;
		}
	}
}

/**
 * @brief 找上面的两个拐点，供十字使用，从上往下找
 * @param start 搜索的范围起点
 * @param end 搜索的范围终点
 * @note 运行完之后查看 Left_Up_Find 和 Right_Up_Find，没找到时为0
 */
void Find_Up_Point(int16 start, int16 end)
{
	int16 i, t;
	Left_Up_Find = 0;
	Right_Up_Find = 0;

	if(start > end) {
		t = start;
		start = end;
		end = t;
	}

	// start<end 由上往下
	if(end >= IMAGE_H - 5)
		end = IMAGE_H - 5;
	if(end <= 5)  // 即使最长白列非常长，也要舍弃部分点，防止数组越界
		end = 5;
	if(start <= 5)  // 下面5行数据不稳定，不能作为边界点来判断，舍弃
		start = 5;
	if(start < stop_search_row + 5) {
		start = stop_search_row + 5;
	}

	for(i = start; i <= end; i++)
	{
		// 查找左上拐点
		if(Left_Up_Find == 0 &&  // 只找第一个符合条件的点
			func_abs(left_edge_line[i] - left_edge_line[i-1]) <= 3 &&
			func_abs(left_edge_line[i-1] - left_edge_line[i-2]) <= 3 &&
			func_abs(left_edge_line[i-2] - left_edge_line[i-3]) <= 3 &&
			((left_edge_line[i] - left_edge_line[i+2]) >= 3) &&
			((left_edge_line[i] - left_edge_line[i+3]) >= 5) &&
			((left_edge_line[i] - left_edge_line[i+4]) >= 10))
		{
			Left_Up_Find = i - 2;  // 获取行数即可
			if(Left_Up_Find == start - 2) {
				Left_Up_Find = 0;  // 如果是起始行，说明没有找到
			}
		}

		// 查找右上拐点
		if(Right_Up_Find == 0 &&  // 只找第一个符合条件的点
			func_abs(right_edge_line[i] - right_edge_line[i-1]) <= 3 &&
			func_abs(right_edge_line[i-1] - right_edge_line[i-2]) <= 3 &&
			func_abs(right_edge_line[i-2] - right_edge_line[i-3]) <= 3 &&
			((right_edge_line[i] - right_edge_line[i+2]) <= -3) &&
			((right_edge_line[i] - right_edge_line[i+3]) <= -5) &&
			((right_edge_line[i] - right_edge_line[i+4]) <= -10))
		{
			Right_Up_Find = i - 2;  // 获取行数即可
			if(Right_Up_Find == start - 2) {
				Right_Up_Find = 0;  // 如果是起始行，说明没有找到
			}
		}

		if(Left_Up_Find != 0 && Right_Up_Find != 0)  // 下面两个找到就出去
		{
			break;
		}
	}
}

/**
 * @brief 确定连续性，用于处理右侧线
 * @param start 起点
 * @param end 终点
 * @return 不连续点的位置，0表示连续
 */
int16 continuity_right(uint8 start, uint8 end)
{
	int16 i;
	int16 continuity_change_flag = 0;

	if(start >= IMAGE_H - 2)  // 数组越界保护
		start = IMAGE_H - 2;
	if(end <= 1) {
		end = 1;
	}
	if(start < end) {
		uint8 t = start;
		start = end;
		end = t;
	}

	for(i = start; i >= end; i--)
	{
		if(func_abs(right_edge_line[i] - right_edge_line[i-1]) >= 5)  // 连续性阈值是5
		{
			continuity_change_flag = i;  // 在i处不连续了
			break;
		}
	}

	return continuity_change_flag;
}

/**
 * @brief 确定从start到end连续性，用于处理左侧线
 * @param start 起点
 * @param end 终点
 * @return 不连续点的位置，0表示连续
 */
int16 continuity_left(uint8 start, uint8 end)
{
	int16 i;
	int16 continuity_change_flag = 0;

	if(start >= IMAGE_H - 2)  // 数组越界保护
		start = IMAGE_H - 2;
	if(end <= 1) {
		end = 1;
	}
	if(start < end) {
		uint8 t = start;
		start = end;
		end = t;
	}

	for(i = start; i >= end; i--)
	{
		if(func_abs(left_edge_line[i] - left_edge_line[i-1]) >= 5)  // 连续性阈值是5
		{
			continuity_change_flag = i;  // 在i处不连续了
			break;
		}
	}

	return continuity_change_flag;
}

/**
 * @brief 补线右（带增长率）
 * @param startx 起点x
 * @param starty 起点y
 * @param endy 终点y
 * @param dx 增长率（x1-x2）/(y1-y2)
 */
void draw_Rline_k(int16 startx, int16 starty, int16 endy, float dx)
{
	int16 step = (starty < endy) ? 1 : -1;

	if(dx == 0) {
		for(int16 i = starty; i != endy; i += step) {
			rightfollowline[i] = startx;
		}
		return;
	}

	for(int16 i = starty; i != endy; i += step) {
		int16 temp = startx + (int16)((float)(i - starty) * dx);
		if(temp < 0) temp = 0;  // 防止越界
		if(temp > IMAGE_W - 1) temp = IMAGE_W - 1;  // 防止越界
		rightfollowline[i] = temp;
	}
}

/**
 * @brief 右两点间连线
 * @param startx 起点x
 * @param starty 起点y
 * @param endy 终点y
 * @param endx 终点x
 */
void add_Rline_k(int16 startx, int16 starty, int16 endy, int16 endx)
{
	if(endy != starty) {
		float dx = (float)(endx - startx) / (float)(endy - starty);
		draw_Rline_k(startx, starty, endy, dx);
	}
}

/**
 * @brief 补线左（带增长率）
 * @param startx 起点x
 * @param starty 起点y
 * @param endy 终点y
 * @param dx 增长率（x1-x2）/(y1-y2)
 */
void draw_Lline_k(int16 startx, int16 starty, int16 endy, float dx)
{
	int16 step = (starty < endy) ? 1 : -1;

	if(dx == 0) {
		for(int16 i = starty; i != endy; i += step) {
			leftfollowline[i] = startx;
		}
		return;
	}

	for(int16 i = starty; i != endy; i += step) {
		int16 temp = startx + (int16)((float)(i - starty) * dx);
		if(temp < 0) temp = 0;  // 防止越界
		if(temp > IMAGE_W - 1) temp = IMAGE_W - 1;  // 防止越界
		leftfollowline[i] = temp;
	}
}

/**
 * @brief 左两点间连线
 * @param startx 起点x
 * @param starty 起点y
 * @param endy 终点y
 * @param endx 终点x
 */
void add_Lline_k(int16 startx, int16 starty, int16 endy, int16 endx)
{
	if(endy != starty) {
		float dx = (float)(endx - startx) / (float)(endy - starty);
		draw_Lline_k(startx, starty, endy, dx);
	}
}

/**
 * @brief 滑动平均滤波左
 * @param dx 新的增长率
 */
void dx1_left_average(float dx)
{
	for(uint8 i = 1; i < 5; i++) {
		dx1[i-1] = dx1[i];
	}
	dx1[4] = dx;
}

/**
 * @brief 滑动平均滤波右
 * @param dx 新的增长率
 */
void dx2_right_average(float dx)
{
	for(uint8 i = 1; i < 5; i++) {
		dx2[i-1] = dx2[i];
	}
	dx2[4] = dx;
}

/**
 * @brief 自上而下补左线
 * @param start 起点
 */
void lenthen_Left_bondarise(int16 start)
{
	if(start < 7) start = 7;
	if(start > IMAGE_H - 1) start = IMAGE_H - 1;

	float dx = (float)(left_edge_line[start] - left_edge_line[start-5]) / 5;
	dx1_left_average(dx);
	float dx_average = (dx1[0] + dx1[1] + dx1[2] + dx1[3] + dx1[4]) / 5;

	for(int16 i = start; i < IMAGE_H - 1; i++)
	{
		float temp_value = (float)left_edge_line[start] + dx_average * (float)(i - start);

		if(temp_value < 0) {
			leftfollowline[i] = 0;
		} else if(temp_value > (float)IMAGE_W - 2) {
			leftfollowline[i] = IMAGE_W - 1;
		} else {
			leftfollowline[i] = (int16)temp_value;
		}
	}
}

/**
 * @brief 自上而下补右线
 * @param start 起点
 */
void lenthen_Right_bondarise(int16 start)
{
	if(start < 7) start = 7;
	if(start > IMAGE_H - 1) start = IMAGE_H - 1;

	float dx = (float)(right_edge_line[start] - right_edge_line[start-5]) / 5;
	dx2_right_average(dx);
	float dx_average = (dx2[0] + dx2[1] + dx2[2] + dx2[3] + dx2[4]) / 5;

	for(int16 i = start; i < IMAGE_H - 1; i++)
	{
		float temp_value = (float)right_edge_line[start] + dx_average * (float)(i - start);

		if(temp_value > IMAGE_W - 2) {
			rightfollowline[i] = IMAGE_W - 1;
		} else if(temp_value < 0) {
			rightfollowline[i] = 0;
		} else {
			rightfollowline[i] = (int16)temp_value;
		}
	}
}

/**
 * @brief 新十字补线主函数（完全替换旧的 image_cross_analysis）
 *
 * 功能说明：
 * 1. 使用四点检测法：查找左上、左下、右上、右下四个拐点
 * 2. 使用状态机管理十字识别：CROSS_STRAIGHT -> CROSS_ROAD/CROSS_ROAD_L/CROSS_ROAD_R
 * 3. 智能补线策略：根据拐点情况自适应补线
 *
 * 核心流程：
 * - 直道状态：检测是否进入十字（通过连续性和拐点判断）
 * - 十字状态：根据拐点情况进行智能补线
 * - 斜入十字：单边拐点处理
 */
void image_cross_analysis(void)
{
	// 初始化跟随线（补线后的边线）
	memcpy(leftfollowline, left_edge_line, sizeof(left_edge_line));
	memcpy(rightfollowline, right_edge_line, sizeof(right_edge_line));

	// 查找上下半段边界点
	Find_Up_Point(IMAGE_H - 1, stop_search_row);      // 查找上半段边界点（左上、右上）
	Find_Down_Point(IMAGE_H - 1, stop_search_row);    // 查找下半段边界点（左下、右下）

	// 左右连续性判断
	continuity_pointLeft[0] = continuity_left(IMAGE_H - 1, stop_search_row + 5);    // 左连续性判断
	continuity_pointRight[0] = continuity_right(IMAGE_H - 1, stop_search_row + 5);  // 右连续性判断
	continuity_pointLeft[1] = left_edge_line[continuity_pointLeft[0]];              // 左连续性点列
	continuity_pointRight[1] = right_edge_line[continuity_pointRight[0]];           // 右连续性点列

	// ==================== 调试信息显示（适配IPS114） ====================
	// 显示关键检测变量,帮助诊断识别失败原因
	// IPS114屏幕宽度240，图像宽度188，调试信息从x=190开始显示
	ips114_show_uint(190, 96, stop_search_row, 3);           // 搜线截止行
	ips114_show_uint(190, 112, (uint32)cross_status, 1);     // 十字状态

	// ==================== 直道状态：检测是否进入十字 ====================
	if(cross_status == CROSS_STRAIGHT) {
		// 只有在截止行较近时才检测十字
		if(stop_search_row < 50) {
			// 情况1：正入十字 - 左右不连续点都找到，且左右上拐点都找到
			if(continuity_pointLeft[0] != 0 && continuity_pointRight[0] != 0 &&
			   Right_Up_Find != 0 && Left_Up_Find != 0 &&
			   (Right_Up_Find > stop_search_row - 2 && Left_Up_Find > stop_search_row - 2))
			{
				cross_status = CROSS_ROAD;  // 进入十字路口状态
				cross_flag = 1;
				return;
			}

			// 情况2：左斜入十字 - 左不连续点找到且右不连续点未找到，且左上拐点找到
			if(continuity_pointLeft[0] != 0 && continuity_pointRight[0] == 0 &&
			   Left_Up_Find != 0 && Left_Up_Find > stop_search_row - 2)
			{
				cross_status = CROSS_ROAD_L;  // 进入左斜入十字状态
				cross_flag = 1;
				return;
			}

			// 情况3：右斜入十字 - 左不连续点未找到且右不连续点找到，且右上拐点找到
			if(continuity_pointLeft[0] == 0 && continuity_pointRight[0] != 0 &&
			   Right_Up_Find != 0 && Right_Up_Find > stop_search_row - 2)
			{
				cross_status = CROSS_ROAD_R;  // 进入右斜入十字状态
				cross_flag = 1;
				return;
			}
		}
	}

	// ==================== 十字路口状态处理（正入） ====================
	if(cross_status == CROSS_ROAD) {
		// 确保下拐点在上拐点下方
		if(Left_Down_Find <= Left_Up_Find) Left_Down_Find = 0;
		if(Right_Down_Find <= Right_Up_Find) Right_Down_Find = 0;

		/* 边界线拟合策略 */
		if(Left_Down_Find != 0 && Right_Down_Find != 0) {
			// 情况1：左右下点均有效 → 双边界直线拟合
			add_Rline_k(right_edge_line[Right_Down_Find], Right_Down_Find,
			           Right_Up_Find - 2, right_edge_line[Right_Up_Find - 2]);
			add_Lline_k(left_edge_line[Left_Down_Find], Left_Down_Find,
			           Left_Up_Find - 2, left_edge_line[Left_Up_Find - 2]);
		}
		else if(Left_Down_Find == 0 && Right_Down_Find != 0) {
			// 情况2：仅右下点有效 → 右边界拟合+左边界延长
			add_Rline_k(right_edge_line[Right_Down_Find], Right_Down_Find,
			           Right_Up_Find, right_edge_line[Right_Up_Find]);
			lenthen_Left_bondarise(Left_Up_Find);
		}
		else if(Left_Down_Find != 0 && Right_Down_Find == 0) {
			// 情况3：仅左下点有效 → 左边界拟合+右边界延长
			lenthen_Right_bondarise(Right_Up_Find);
			add_Lline_k(left_edge_line[Left_Down_Find], Left_Down_Find,
			           Left_Up_Find, left_edge_line[Left_Up_Find]);
		}
		else {
			// 情况4：无有效下点 → 双边界延长
			lenthen_Right_bondarise(Right_Up_Find);
			lenthen_Left_bondarise(Left_Up_Find);
		}

		// 异常处理：突变点失效时恢复原始边界
		if(Right_Up_Find == 0) memcpy(rightfollowline, right_edge_line, sizeof(right_edge_line));
		if(Left_Up_Find == 0) memcpy(leftfollowline, left_edge_line, sizeof(left_edge_line));

		// 退出十字状态判断
		if(Right_Up_Find == 0 || Left_Up_Find == 0) {
			cross_status = CROSS_STRAIGHT;
			cross_flag = 0;
			return;
		}

		// 将补线后的边线复制回原边线数组
		memcpy(left_edge_line, leftfollowline, sizeof(left_edge_line));
		memcpy(right_edge_line, rightfollowline, sizeof(right_edge_line));
	}

	// ==================== 左斜入十字状态处理 ====================
	if(cross_status == CROSS_ROAD_L) {
		// 如果左右上点都有效，转为正入十字
		if(Left_Up_Find && Right_Up_Find) {
			cross_status = CROSS_ROAD;
			return;
		}

		// 如果找到了左上点和左下点
		if(Left_Up_Find != 0 && Left_Down_Find != 0) {
			add_Lline_k(left_edge_line[Left_Down_Find], Left_Down_Find,
			           Left_Up_Find, left_edge_line[Left_Up_Find]);
		}

		// 如果只找到了左上点
		if(Left_Up_Find != 0 && Left_Down_Find == 0) {
			lenthen_Left_bondarise(Left_Up_Find);
		}

		// 如果左上点未找到，返回直道状态
		if(Left_Up_Find == 0) {
			cross_status = CROSS_STRAIGHT;
			cross_flag = 0;
		}

		// 将补线后的边线复制回原边线数组
		memcpy(left_edge_line, leftfollowline, sizeof(left_edge_line));
		memcpy(right_edge_line, rightfollowline, sizeof(right_edge_line));
	}

	// ==================== 右斜入十字状态处理 ====================
	if(cross_status == CROSS_ROAD_R) {
		// 如果左右上点都有效，转为正入十字
		if(Left_Up_Find && Right_Up_Find) {
			cross_status = CROSS_ROAD;
			return;
		}

		// 如果找到了右上点和右下点
		if(Right_Up_Find != 0 && Right_Down_Find != 0) {
			add_Rline_k(right_edge_line[Right_Down_Find], Right_Down_Find,
			           Right_Up_Find, right_edge_line[Right_Up_Find]);
		}

		// 如果只找到了右上点
		if(Right_Up_Find != 0 && Right_Down_Find == 0) {
			lenthen_Right_bondarise(Right_Up_Find);
		}

		// 如果右上点未找到，返回直道状态
		if(Right_Up_Find == 0) {
			cross_status = CROSS_STRAIGHT;
			cross_flag = 0;
		}

		// 将补线后的边线复制回原边线数组
		memcpy(left_edge_line, leftfollowline, sizeof(left_edge_line));
		memcpy(right_edge_line, rightfollowline, sizeof(right_edge_line));
	}
}

void image_process(uint16 display_width,uint16 display_height,uint8 mode){
	get_image();
	reference_point = 0; white_max_point = 0; white_min_point = 0; reference_col = 0;

	get_reference_point(user_image);
	search_reference_col(user_image);
	search_line(user_image);

	// 根据开关决定是否启用十字识别
	if(cross_enable && !circle_flag){
		image_cross_analysis();
	}

	// 动态前瞻权重调整（在计算加权中线之前调用）
	dynamic_weight_adjust();

	image_calculate_mid(mid_mode);
	image_calculate_prospect(user_image);

	if(mode)
		image_display_edge_line(user_image,display_width,display_height);
}

// ==================== 动态前瞻权重函数 ====================

/**
 * @brief 计算赛道曲率（基于中线变化率）
 * @return float 返回曲率值（0-100，值越大弯道越急）
 *
 * 算法说明：
 * 1. 分段计算中线偏移量：近处、中处、远处三段
 * 2. 加权计算总曲率：远处权重高（提前预判），近处权重低
 * 3. 低通滤波平滑曲率变化
 */
float calculate_curvature(void)
{
	float curvature = 0.0f;
	float near_offset = 0.0f;   // 近处偏移（行90-119）
	float mid_offset = 0.0f;    // 中处偏移（行60-89）
	float far_offset = 0.0f;    // 远处偏移（行30-59）
	uint8 center_col = IMAGE_W / 2;  // 图像中心列（94）

	// 计算近处平均偏移（底部30行）
	for(int i = IMAGE_H - 30; i < IMAGE_H; i++) {
		near_offset += func_abs(mid_line[i] - center_col);
	}
	near_offset /= 30.0f;

	// 计算中处平均偏移（中间30行）
	for(int i = IMAGE_H - 60; i < IMAGE_H - 30; i++) {
		mid_offset += func_abs(mid_line[i] - center_col);
	}
	mid_offset /= 30.0f;

	// 计算远处平均偏移（上面30行）
	for(int i = IMAGE_H - 90; i < IMAGE_H - 60; i++) {
		if(i < stop_search_row) continue;  // 跳过无效行
		far_offset += func_abs(mid_line[i] - center_col);
	}
	far_offset /= 30.0f;

	// 加权计算总曲率（远处权重最高）
	// 远处权重50%，中处30%，近处20%
	curvature = far_offset * 0.5f + mid_offset * 0.3f + near_offset * 0.2f;

	// 归一化到0-100范围（假设最大偏移为94列，即图像宽度的一半）
	curvature = (curvature / 94.0f) * 100.0f;
	curvature = func_limit_ab((int)curvature, 0, 100);

	// 存储原始曲率
	raw_curvature = curvature;

	// 低通滤波（避免曲率突变）
	if(current_curvature == 0.0f) {
		current_curvature = curvature;  // 首次初始化
	} else {
		current_curvature = current_curvature * curvature_filter_ratio +
		                    curvature * (1.0f - curvature_filter_ratio);
	}

	return current_curvature;
}

/**
 * @brief 根据曲率动态调整权重数组（滞回带 + 保持时间）
 * @param curvature 当前赛道曲率（0-100）
 *
 * 策略说明：
 * - 曲率 < far_threshold：切换到远前瞻（权重数组3，峰值在40-60行）
 * - 曲率 > near_threshold：切换到近前瞻（权重数组2，峰值在50-70行）
 * - far_threshold < 曲率 < near_threshold：死区，保持当前状态
 *
 * 滞回机制：
 * - 使用两个独立阈值形成滞回带，避免在阈值附近频繁切换
 * - 增加最小保持时间，状态切换后必须保持一定帧数才能再次切换
 *
 * 优化点：
 * - 渐变切换：避免权重突变导致控制抖动
 * - 速度可调：通过weight_shift_speed参数控制切换速度
 */
void adjust_weight_by_curvature(float curvature)
{
	static uint16 state_hold_counter = 0;  // 状态保持计数器
	static uint8 initialized = 0;          // 初始化标志
	uint8 target_status = dynamic_weight_status;  // 默认保持当前状态

	// ==================== 首次初始化 ====================
	if(!initialized) {
		// 根据当前状态初始化目标权重数组
		if(dynamic_weight_status == 0) {
			memcpy(dynamic_weight_target, mid_weight_3, IMAGE_H);
		} else {
			memcpy(dynamic_weight_target, mid_weight_2, IMAGE_H);
		}
		initialized = 1;
	}

	// ==================== 滞回带逻辑判断 ====================
	if(curvature < curvature_far_threshold) {
		// 曲率低于远阈值 → 切换到远前瞻
		target_status = 0;
	}
	else if(curvature > curvature_near_threshold) {
		// 曲率高于近阈值 → 切换到近前瞻
		target_status = 1;
	}
	// 在两阈值之间：死区，保持target_status不变（即当前状态）

	// ==================== 状态切换逻辑（带保持时间） ====================
	if(target_status != dynamic_weight_status) {
		// 需要切换状态，累加计数器
		state_hold_counter++;

		// 检查是否达到最小保持时间
		if(state_hold_counter >= weight_hold_time) {
			// 可以切换了
			dynamic_weight_status = target_status;
			state_hold_counter = 0;  // 重置计数器

			// 更新目标权重数组
			if(dynamic_weight_status == 0) {
				// 远前瞻（使用权重数组3）
				memcpy(dynamic_weight_target, mid_weight_3, IMAGE_H);
			} else {
				// 近前瞻（使用权重数组2）
				memcpy(dynamic_weight_target, mid_weight_2, IMAGE_H);
			}
		}
	}
	else {
		// 不需要切换，重置计数器
		state_hold_counter = 0;
	}

	// ==================== 渐变切换权重（避免突变） ====================
	// 限制 weight_shift_speed 范围，确保除数不会为 0
	uint16 safe_speed = func_limit_ab(weight_shift_speed, 1, 10);
	int divisor = 11 - safe_speed;  // divisor范围：1-10

	for(int i = 0; i < IMAGE_H; i++) {
		int diff = (int)dynamic_weight_target[i] - (int)mid_weight[i];
		if(diff > 0) {
			mid_weight[i] += func_limit_ab(diff / divisor, 1, diff);
		} else if(diff < 0) {
			mid_weight[i] -= func_limit_ab((-diff) / divisor, 1, -diff);
		}
	}
}

/**
 * @brief 动态前瞻权重主函数（在image_process中调用）
 *
 * 功能：
 * 1. 计算当前赛道曲率
 * 2. 根据曲率自适应调整权重数组
 * 3. 实现远前瞻（直道）和近前瞻（弯道）的自动切换
 */
void dynamic_weight_adjust(void)
{
	// 检查开关
	if(!dynamic_weight_enable) {
		return;  // 关闭时使用固定权重
	}

	// 计算曲率
	float curvature = calculate_curvature();

	// 调整权重
	adjust_weight_by_curvature(curvature);
}

#pragma once

#include <cstdint>

namespace plc {
// 寄存器起始地址,用于地址偏移
static constexpr std::int32_t MAddr = 0;
static constexpr std::int32_t XAddr = 20480;
static constexpr std::int32_t YAddr = 24576;
static constexpr std::int32_t SMAddr = 36864;

static constexpr std::int32_t DAddr = 0;
static constexpr std::int32_t HDAddr = 41088;
static constexpr std::int32_t HSDAddr = 47232;
static constexpr std::int32_t HSCDAddr = 50304;

// 系统控制参数
static constexpr std::int32_t M_Init = 0;  // 系统初始化
static constexpr std::int32_t SM_Run = 0;  // PLC运行线圈

// 地轨
static constexpr std::int32_t HD_ExAxis1Enable = 2201;          // 地轨伺服使能(低位)
static constexpr std::int32_t HD_ExAxis1ZRN = 2203;             // 地轨机械归零（低位）
static constexpr std::int32_t HD_ExAxis1MOVEA = 2215;           // 地轨绝对位置运动（低位）
static constexpr std::int32_t HD_ExAxis1MOVEAVelParams = 2228;  // 地轨绝对位置运动参数（位置）
static constexpr std::int32_t HD_ExAxis1MOVEAPosParams = 2232;  // 地轨绝对位置运动参数（速度）
static constexpr std::int32_t HD_ExAxis1Stop = 2254;            // 地轨停止（低位）
static constexpr std::int32_t HD_ExAxis1Rst = 2252;             // 地轨复位（低位）
static constexpr std::int32_t HD_ExAxis1Pos = 2284;             // 地轨实际运动位置
static constexpr std::int32_t HD_ExAxis1Vel = 2288;             // 地轨实际运动速度

// 对位平台总控（7个），对位平台0
static constexpr std::int32_t HD_Plt_0_Enable = 100;    // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_0_Back = 100;      // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_0_Location = 101;  // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_0_Rst = 101;       // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_0_Stop = 102;      // 平台一键停止（低位）

// 对位平台1
static constexpr std::int32_t HD_Plt_1_Enable = 400;    // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_1_Back = 400;      // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_1_Location = 401;  // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_1_Rst = 401;       // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_1_Stop = 402;      // 平台一键停止（低位）

// 对位平台2
static constexpr std::int32_t HD_Plt_2_Enable = 700;    // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_2_Back = 700;      // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_2_Location = 701;  // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_2_Rst = 701;       // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_2_Stop = 702;      // 平台一键停止（低位）

// 对位平台3
static constexpr std::int32_t HD_Plt_3_Enable = 1000;    // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_3_Back = 1000;      // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_3_Location = 1001;  // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_3_Rst = 1001;       // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_3_Stop = 1002;      // 平台一键停止（低位）

// 对位平台4
static constexpr std::int32_t HD_Plt_4_Enable = 1300;    // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_4_Back = 1300;      // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_4_Location = 1301;  // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_4_Rst = 1301;       // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_4_Stop = 1302;      // 平台一键停止（低位）

// 对位平台5
static constexpr std::int32_t HD_Plt_5_Enable = 1600;    // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_5_Back = 1600;      // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_5_Location = 1601;  // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_5_Rst = 1601;       // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_5_Stop = 1602;      // 平台一键停止（低位）

// 对位平台6
static constexpr std::int32_t HD_Plt_6_Enable = 1900;    // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_6_Back = 1900;      // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_6_Location = 1901;  // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_6_Rst = 1901;       // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_6_Stop = 1902;      // 平台一键停止（低位）

// 0号对位平台X轴
static constexpr std::int32_t HD_0_XAxis1Enable = 121;          // 0号对位平台X轴伺服使能（低位）
static constexpr std::int32_t HD_0_XAxis1MOVER = 145;           // 0号对位平台X轴相对位置运动（高位）
static constexpr std::int32_t HD_0_XAxis1MOVERPosParams = 148;  // 0号对位平台X轴相对位置运动参数（位置）
static constexpr std::int32_t HD_0_XAxis1MOVERVelParams = 152;  // 0号对位平台X轴相对位置运动参数（速度）
static constexpr std::int32_t HD_0_XAxis1MOVEA = 157;           // 0号对位平台X轴速度运动（高位）
static constexpr std::int32_t HD_0_XAxis1MOVEAVelParams = 160;  // 0号对位平台X轴速度运动参数（速度）
static constexpr std::int32_t HD_0_XAxis1Stop = 174;            // 0号对位平台X轴停止(低位)
static constexpr std::int32_t HD_0_XAxis1Rst = 172;             // 0号对位平台X轴复位（低位）
// 0号对位平台Y轴
static constexpr std::int32_t HD_0_YAxis1Enable = 213;          // 0号对位平台Y轴伺服使能（低位）
static constexpr std::int32_t HD_0_YAxis1MOVER = 237;           // 0号对位平台Y轴相对位置运动（高位）
static constexpr std::int32_t HD_0_YAxis1MOVERPosParams = 240;  // 0号对位平台Y轴相对位置运动参数（位置）
static constexpr std::int32_t HD_0_YAxis1MOVERVelParams = 244;  // 0号对位平台Y轴相对位置运动参数（速度）
static constexpr std::int32_t HD_0_YAxis1MOVEA = 249;           // 0号对位平台Y轴速度运动（高位）
static constexpr std::int32_t HD_0_YAxis1MOVEAVelParams = 252;  // 0号对位平台Y轴速度运动参数（速度）
static constexpr std::int32_t HD_0_YAxis1Stop = 266;            // 0号对位平台Y轴停止(低位)
static constexpr std::int32_t HD_0_YAxis1Rst = 264;             // 0号对位平台Y轴复位（低位）
// 0号对位平台旋转轴
static constexpr std::int32_t HD_0_RotAxis1Enable = 305;          // 0号对位平台旋转轴伺服使能（低位）
static constexpr std::int32_t HD_0_RotAxis1MOVER = 329;           // 0号对位平台旋转轴相对位置运动（高位）
static constexpr std::int32_t HD_0_RotAxis1MOVERPosParams = 332;  // 0号对位平台旋转轴相对位置运动参数（位置）
static constexpr std::int32_t HD_0_RotAxis1MOVERVelParams = 336;  // 0号对位平台旋转轴相对位置运动参数（速度）
static constexpr std::int32_t HD_0_RotAxis1Stop = 358;            // 0号对位平台旋转轴停止(低位)
static constexpr std::int32_t HD_0_RotAxis1Rst = 356;             // 0号对位平台旋转轴复位（低位）

// //1号对位平台X轴
// static constexpr std::int32_t HD_1_XAxis1Enable = 41088 +;           // 0号对位平台X轴伺服使能
// static constexpr std::int32_t HD_1_XAxis1MOVEA = 41088 +;            // 0号对位平台X轴相对位置运动
// static constexpr std::int32_t HD_1_XAxis1MOVEAParams = 41088 +;     // 0号对位平台X轴相对位置运动参数
// static constexpr std::int32_t HD_1_XAxis1Stop = 41088 +;             // 0号对位平台X轴停止
// static constexpr std::int32_t HD_1_XAxis1StopParams = 41088 +;      // 0号对位平台X轴停止参数
// static constexpr std::int32_t HD_1_XAxis1Rst = 41088 +;              // 0号对位平台X轴复位
// static constexpr std::int32_t HD_1_XAxis1Pos = 41088 +;  // 0号对位平台X轴实际运动位置
// static constexpr std::int32_t HD_1_XAxis1Vel = 41088 +;  // 0号对位平台X轴实际运动速度
// //1号对位平台Y轴
// static constexpr std::int32_t HD_1_YAxis1Enable =41088 +;           // 0号对位平台Y轴伺服使能
// static constexpr std::int32_t HD_1_YAxis1MOVEA = 41088 +;            // 0号对位平台Y轴相对位置运动
// static constexpr std::int32_t HD_1_YAxis1MOVEAParams = 41088 +;     // 0号对位平台Y轴相对位置运动参数
// static constexpr std::int32_t HD_1_YAxis1Stop = 41088 +;             // 0号对位平台Y轴停止
// static constexpr std::int32_t HD_1_YAxis1StopParams = 41088 +;      // 0号对位平台Y轴停止参数
// static constexpr std::int32_t HD_1_YAxis1Rst = 41088 +;              // 0号对位平台Y轴复位
// static constexpr std::int32_t HD_1_YAxis1Pos = 41088 +;  // 0号对位平台Y轴实际运动位置
// static constexpr std::int32_t HD_1_YAxis1Vel = 41088 +;  // 0号对位平台Y轴实际运动速度
// //1号对位平台旋转轴
// static constexpr std::int32_t HD_1_RotAxis1Enable = 41088 +;           // 0号对位平台旋转轴伺服使能
// static constexpr std::int32_t HD_1_RotAxis1MOVEA = 41088 +;            // 0号对位平台旋转轴相对位置运动
// static constexpr std::int32_t HD_1_RotAxis1MOVEAParams = 41088 +;     // 0号对位平台旋转轴相对位置运动参数
// static constexpr std::int32_t HD_1_RotAxis1Stop = 41088 +;             // 0号对位平台旋转轴停止
// static constexpr std::int32_t HD_1_RotAxis1StopParams = 41088 +;      // 0号对位平台旋转轴停止参数
// static constexpr std::int32_t HD_1_RotAxis1Rst = 41088 +;              // 0号对位平台旋转轴复位
// static constexpr std::int32_t HD_1_RotAxis1Pos = 41088 +;  // 0号对位平台旋转轴实际运动位置
// static constexpr std::int32_t HD_1_RotAxis1Vel = 41088 +;  // 0号对位平台旋转轴实际运动速度

// //2号对位平台X轴
// static constexpr std::int32_t HD_2_XAxis1Enable = 41088 +;           // 0号对位平台X轴伺服使能
// static constexpr std::int32_t HD_2_XAxis1MOVEA = 41088 +;            // 0号对位平台X轴相对位置运动
// static constexpr std::int32_t HD_2_XAxis1MOVEAParams = 41088 +;     // 0号对位平台X轴相对位置运动参数
// static constexpr std::int32_t HD_2_XAxis1Stop = 41088 +;             // 0号对位平台X轴停止
// static constexpr std::int32_t HD_2_XAxis1StopParams = 41088 +;      // 0号对位平台X轴停止参数
// static constexpr std::int32_t HD_2_XAxis1Rst = 41088 +;              // 0号对位平台X轴复位
// static constexpr std::int32_t HD_2_XAxis1Pos = 41088 +;  // 0号对位平台X轴实际运动位置
// static constexpr std::int32_t HD_2_XAxis1Vel = 41088 +;  // 0号对位平台X轴实际运动速度
// //2号对位平台Y轴
// static constexpr std::int32_t HD_2_YAxis1Enable = 41088 +;           // 0号对位平台Y轴伺服使能
// static constexpr std::int32_t HD_2_YAxis1MOVEA = 41088 +;            // 0号对位平台Y轴相对位置运动
// static constexpr std::int32_t HD_2_YAxis1MOVEAParams = 41088 +;     // 0号对位平台Y轴相对位置运动参数
// static constexpr std::int32_t HD_2_YAxis1Stop = 41088 +;             // 0号对位平台Y轴停止
// static constexpr std::int32_t HD_2_YAxis1StopParams = 41088 +;      // 0号对位平台Y轴停止参数
// static constexpr std::int32_t HD_2_YAxis1Rst = 41088 +;              // 0号对位平台Y轴复位
// static constexpr std::int32_t HD_2_YAxis1Pos = 41088 +;  // 0号对位平台Y轴实际运动位置
// static constexpr std::int32_t HD_2_YAxis1Vel = 41088 +;  // 0号对位平台Y轴实际运动速度
// //2号对位平台旋转轴
// static constexpr std::int32_t HD_2_RotAxis1Enable = 41088 +;           // 0号对位平台旋转轴伺服使能
// static constexpr std::int32_t HD_2_RotAxis1MOVEA = 41088 +;            // 0号对位平台旋转轴相对位置运动
// static constexpr std::int32_t HD_2_RotAxis1MOVEAParams = 41088 +;     // 0号对位平台旋转轴相对位置运动参数
// static constexpr std::int32_t HD_2_RotAxis1Stop = 41088 +;             // 0号对位平台旋转轴停止
// static constexpr std::int32_t HD_2_RotAxis1StopParams = 41088 +;      // 0号对位平台旋转轴停止参数
// static constexpr std::int32_t HD_2_RotAxis1Rst = 41088 +;              // 0号对位平台旋转轴复位
// static constexpr std::int32_t HD_2_RotAxis1Pos = 41088 +;  // 0号对位平台旋转轴实际运动位置
// static constexpr std::int32_t HD_2_RotAxis1Vel = 41088 +;  // 0号对位平台旋转轴实际运动速度

// //3号对位平台X轴
// static constexpr std::int32_t HD_3_XAxis1Enable = 41088 +;           // 0号对位平台X轴伺服使能
// static constexpr std::int32_t HD_3_XAxis1MOVEA = 41088 +;            // 0号对位平台X轴相对位置运动
// static constexpr std::int32_t HD_3_XAxis1MOVEAParams = 41088 +;     // 0号对位平台X轴相对位置运动参数
// static constexpr std::int32_t HD_3_XAxis1Stop = 41088 +;             // 0号对位平台X轴停止
// static constexpr std::int32_t HD_3_XAxis1StopParams = 41088 +;      // 0号对位平台X轴停止参数
// static constexpr std::int32_t HD_3_XAxis1Rst = 41088 +;              // 0号对位平台X轴复位
// static constexpr std::int32_t HD_3_XAxis1Pos = 41088 +;  // 0号对位平台X轴实际运动位置
// static constexpr std::int32_t HD_3_XAxis1Vel = 41088 +;  // 0号对位平台X轴实际运动速度
// //3号对位平台Y轴
// static constexpr std::int32_t HD_3_YAxis1Enable = 41088 +;           // 0号对位平台Y轴伺服使能
// static constexpr std::int32_t HD_3_YAxis1MOVEA = 41088 +;            // 0号对位平台Y轴相对位置运动
// static constexpr std::int32_t HD_3_YAxis1MOVEAParams = 41088 +;     // 0号对位平台Y轴相对位置运动参数
// static constexpr std::int32_t HD_3_YAxis1Stop = 41088 +;             // 0号对位平台Y轴停止
// static constexpr std::int32_t HD_3_YAxis1StopParams = 41088 +;      // 0号对位平台Y轴停止参数
// static constexpr std::int32_t HD_3_YAxis1Rst = 41088 +;              // 0号对位平台Y轴复位
// static constexpr std::int32_t HD_3_YAxis1Pos = 41088 +;  // 0号对位平台Y轴实际运动位置
// static constexpr std::int32_t HD_3_YAxis1Vel = 41088 +;  // 0号对位平台Y轴实际运动速度
// //3号对位平台旋转轴
// static constexpr std::int32_t HD_3_RotAxis1Enable = 41088 +;           // 0号对位平台旋转轴伺服使能
// static constexpr std::int32_t HD_3_RotAxis1MOVEA = 41088 +;            // 0号对位平台旋转轴相对位置运动
// static constexpr std::int32_t HD_3_RotAxis1MOVEAParams = 41088 +;     // 0号对位平台旋转轴相对位置运动参数
// static constexpr std::int32_t HD_3_RotAxis1Stop = 41088 +;             // 0号对位平台旋转轴停止
// static constexpr std::int32_t HD_3_RotAxis1StopParams = 41088 +;      // 0号对位平台旋转轴停止参数
// static constexpr std::int32_t HD_3_RotAxis1Rst = 41088 +;              // 0号对位平台旋转轴复位
// static constexpr std::int32_t HD_3_RotAxis1Pos = 41088 +;  // 0号对位平台旋转轴实际运动位置
// static constexpr std::int32_t HD_3_RotAxis1Vel = 41088 +;  // 0号对位平台旋转轴实际运动速度

// //4号对位平台X轴
// static constexpr std::int32_t HD_4_XAxis1Enable = 41088 +;           // 0号对位平台X轴伺服使能
// static constexpr std::int32_t HD_4_XAxis1MOVEA = 41088 +;            // 0号对位平台X轴相对位置运动
// static constexpr std::int32_t HD_4_XAxis1MOVEAParams = 41088 +;     // 0号对位平台X轴相对位置运动参数
// static constexpr std::int32_t HD_4_XAxis1Stop = 41088 +;             // 0号对位平台X轴停止
// static constexpr std::int32_t HD_4_XAxis1StopParams = 41088 +;      // 0号对位平台X轴停止参数
// static constexpr std::int32_t HD_4_XAxis1Rst = 41088 +;              // 0号对位平台X轴复位
// static constexpr std::int32_t HD_4_XAxis1Pos = 41088 +;  // 0号对位平台X轴实际运动位置
// static constexpr std::int32_t HD_4_XAxis1Vel = 41088 +;  // 0号对位平台X轴实际运动速度
// //4号对位平台Y轴
// static constexpr std::int32_t HD_4_YAxis1Enable = 41088 +;           // 0号对位平台Y轴伺服使能
// static constexpr std::int32_t HD_4_YAxis1MOVEA = 41088 +;            // 0号对位平台Y轴相对位置运动
// static constexpr std::int32_t HD_4_YAxis1MOVEAParams = 41088 +;     // 0号对位平台Y轴相对位置运动参数
// static constexpr std::int32_t HD_4_YAxis1Stop = 41088 +;             // 0号对位平台Y轴停止
// static constexpr std::int32_t HD_4_YAxis1StopParams = 41088 +;      // 0号对位平台Y轴停止参数
// static constexpr std::int32_t HD_4_YAxis1Rst = 41088 +;              // 0号对位平台Y轴复位
// static constexpr std::int32_t HD_4_YAxis1Pos = 41088 +;  // 0号对位平台Y轴实际运动位置
// static constexpr std::int32_t HD_4_YAxis1Vel = 41088 +;  // 0号对位平台Y轴实际运动速度
// //4号对位平台旋转轴
// static constexpr std::int32_t HD_4_RotAxis1Enable = 41088 +;           // 0号对位平台旋转轴伺服使能
// static constexpr std::int32_t HD_4_RotAxis1MOVEA = 41088 +;            // 0号对位平台旋转轴相对位置运动
// static constexpr std::int32_t HD_4_RotAxis1MOVEAParams = 41088 +;     // 0号对位平台旋转轴相对位置运动参数
// static constexpr std::int32_t HD_4_RotAxis1Stop = 41088 +;             // 0号对位平台旋转轴停止
// static constexpr std::int32_t HD_4_RotAxis1StopParams = 41088 +;      // 0号对位平台旋转轴停止参数
// static constexpr std::int32_t HD_4_RotAxis1Rst = 41088 +;              // 0号对位平台旋转轴复位
// static constexpr std::int32_t HD_4_RotAxis1Pos = 41088 +;  // 0号对位平台旋转轴实际运动位置
// static constexpr std::int32_t HD_4_RotAxis1Vel = 41088 +;  // 0号对位平台旋转轴实际运动速度

// //5号对位平台X轴
// static constexpr std::int32_t HD_5_XAxis1Enable = 41088 +;           // 0号对位平台X轴伺服使能
// static constexpr std::int32_t HD_5_XAxis1MOVEA = 41088 +;            // 0号对位平台X轴相对位置运动
// static constexpr std::int32_t HD_5_XAxis1MOVEAParams = 41088 +;     // 0号对位平台X轴相对位置运动参数
// static constexpr std::int32_t HD_5_XAxis1Stop = 41088 +;             // 0号对位平台X轴停止
// static constexpr std::int32_t HD_5_XAxis1StopParams = 41088 +;      // 0号对位平台X轴停止参数
// static constexpr std::int32_t HD_5_XAxis1Rst = 41088 +;              // 0号对位平台X轴复位
// static constexpr std::int32_t HD_5_XAxis1Pos = 41088 +;  // 0号对位平台X轴实际运动位置
// static constexpr std::int32_t HD_5_XAxis1Vel = 41088 +;  // 0号对位平台X轴实际运动速度
// //5号对位平台Y轴
// static constexpr std::int32_t HD_5_YAxis1Enable = 41088 +;           // 0号对位平台Y轴伺服使能
// static constexpr std::int32_t HD_5_YAxis1MOVEA = 41088 +;            // 0号对位平台Y轴相对位置运动
// static constexpr std::int32_t HD_5_YAxis1MOVEAParams = 41088 +;     // 0号对位平台Y轴相对位置运动参数
// static constexpr std::int32_t HD_5_YAxis1Stop = 41088 +;             // 0号对位平台Y轴停止
// static constexpr std::int32_t HD_5_YAxis1StopParams = 41088 +;      // 0号对位平台Y轴停止参数
// static constexpr std::int32_t HD_5_YAxis1Rst = 41088 +;              // 0号对位平台Y轴复位
// static constexpr std::int32_t HD_5_YAxis1Pos = 41088 +;  // 0号对位平台Y轴实际运动位置
// static constexpr std::int32_t HD_5_YAxis1Vel = 41088 +;  // 0号对位平台Y轴实际运动速度
// //5号对位平台旋转轴
// static constexpr std::int32_t HD_5_RotAxis1Enable = 41088 +;           // 0号对位平台旋转轴伺服使能
// static constexpr std::int32_t HD_5_RotAxis1MOVEA = 41088 +;            // 0号对位平台旋转轴相对位置运动
// static constexpr std::int32_t HD_5_RotAxis1MOVEAParams = 41088 +;     // 0号对位平台旋转轴相对位置运动参数
// static constexpr std::int32_t HD_5_RotAxis1Stop = 41088 +;             // 0号对位平台旋转轴停止
// static constexpr std::int32_t HD_5_RotAxis1StopParams = 41088 +;      // 0号对位平台旋转轴停止参数
// static constexpr std::int32_t HD_5_RotAxis1Rst = 41088 +;              // 0号对位平台旋转轴复位
// static constexpr std::int32_t HD_5_RotAxis1Pos = 41088 +;  // 0号对位平台旋转轴实际运动位置
// static constexpr std::int32_t HD_5_RotAxis1Vel = 41088 +;  // 0号对位平台旋转轴实际运动速度

// //6号对位平台X轴
// static constexpr std::int32_t HD_6_XAxis1Enable = 41088 +;           // 0号对位平台X轴伺服使能
// static constexpr std::int32_t HD_6_XAxis1MOVEA = 41088 +;            // 0号对位平台X轴相对位置运动
// static constexpr std::int32_t HD_6_XAxis1MOVEAParams = 41088 +;     // 0号对位平台X轴相对位置运动参数
// static constexpr std::int32_t HD_6_XAxis1Stop = 41088 +;             // 0号对位平台X轴停止
// static constexpr std::int32_t HD_6_XAxis1StopParams = 41088 +;      // 0号对位平台X轴停止参数
// static constexpr std::int32_t HD_6_XAxis1Rst = 41088 +;              // 0号对位平台X轴复位
// static constexpr std::int32_t HD_6_XAxis1Pos = 41088 +;  // 0号对位平台X轴实际运动位置
// static constexpr std::int32_t HD_6_XAxis1Vel = 41088 +;  // 0号对位平台X轴实际运动速度
// //6号对位平台Y轴
// static constexpr std::int32_t HD_6_YAxis1Enable = 41088 +;           // 0号对位平台Y轴伺服使能
// static constexpr std::int32_t HD_6_YAxis1MOVEA = 41088 +;            // 0号对位平台Y轴相对位置运动
// static constexpr std::int32_t HD_6_YAxis1MOVEAParams = 41088 +;     // 0号对位平台Y轴相对位置运动参数
// static constexpr std::int32_t HD_6_YAxis1Stop = 41088 +;             // 0号对位平台Y轴停止
// static constexpr std::int32_t HD_6_YAxis1StopParams = 41088 +;      // 0号对位平台Y轴停止参数
// static constexpr std::int32_t HD_6_YAxis1Rst = 41088 +;              // 0号对位平台Y轴复位
// static constexpr std::int32_t HD_6_YAxis1Pos = 41088 +;  // 0号对位平台Y轴实际运动位置
// static constexpr std::int32_t HD_6_YAxis1Vel = 41088 +;  // 0号对位平台Y轴实际运动速度
// //6号对位平台旋转轴
// static constexpr std::int32_t HD_6_RotAxis1Enable = 41088 +;           // 0号对位平台旋转轴伺服使能
// static constexpr std::int32_t HD_6_RotAxis1MOVEA = 41088 +;            // 0号对位平台旋转轴相对位置运动
// static constexpr std::int32_t HD_6_RotAxis1MOVEAParams = 41088 +;     // 0号对位平台旋转轴相对位置运动参数
// static constexpr std::int32_t HD_6_RotAxis1Stop = 41088 +;             // 0号对位平台旋转轴停止
// static constexpr std::int32_t HD_6_RotAxis1StopParams = 41088 +;      // 0号对位平台旋转轴停止参数
// static constexpr std::int32_t HD_6_RotAxis1Rst = 41088 +;              // 0号对位平台旋转轴复位
// static constexpr std::int32_t HD_6_RotAxis1Pos = 41088 +;  // 0号对位平台旋转轴实际运动位置
// static constexpr std::int32_t HD_6_RotAxis1Vel = 41088 +;  // 0号对位平台旋转轴实际运动速度

}  // namespace plc

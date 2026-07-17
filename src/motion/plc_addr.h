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
static constexpr std::int32_t HD_ExAxis1MOVEAPosParams = 2216;  // 地轨绝对位置运动参数（位置）
static constexpr std::int32_t HD_ExAxis1MOVEAVelParams = 2220;  // 地轨绝对位置运动参数（速度）
static constexpr std::int32_t HD_ExAxis1MOVEADone = 2224;       // 地轨绝对位置运动完成（低位）
static constexpr std::int32_t HD_ExAxis1Stop = 2254;            // 地轨停止（低位）
static constexpr std::int32_t HD_ExAxis1Rst = 2252;             // 地轨复位（低位）
static constexpr std::int32_t D_ExAxis1Stop = 2254;             // 地轨运动停止(低位)
static constexpr std::int32_t D_ExAxis1StopDone = 2254;         // 地轨运动停止完成(高位)
static constexpr std::int32_t D_ExAxis1Pos = 20044;             // 地轨实际运动位置
static constexpr std::int32_t D_ExAxis1Vel = 20048;             // 地轨实际运动速度

// 对位平台总控（7个），对位平台0
static constexpr std::int32_t HD_Plt_0_Enable = 100;        // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_0_Back = 100;          // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_0_Location = 101;      // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_0_Rst = 101;           // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_0_Stop = 102;          // 平台一键停止（低位）
static constexpr std::int32_t HD_Plt_0_LocationDone = 117;  // 平台一键定位完成（低位）
// 对位平台1
static constexpr std::int32_t HD_Plt_1_Enable = 400;        // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_1_Back = 400;          // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_1_Location = 401;      // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_1_Rst = 401;           // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_1_Stop = 402;          // 平台一键停止（低位）
static constexpr std::int32_t HD_Plt_1_LocationDone = 417;  // 平台一键定位完成（低位）
// 对位平台2
static constexpr std::int32_t HD_Plt_2_Enable = 700;        // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_2_Back = 700;          // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_2_Location = 701;      // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_2_Rst = 701;           // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_2_Stop = 702;          // 平台一键停止（低位）
static constexpr std::int32_t HD_Plt_2_LocationDone = 717;  // 平台一键定位完成（低位）
// 对位平台3
static constexpr std::int32_t HD_Plt_3_Enable = 1000;        // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_3_Back = 1000;          // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_3_Location = 1001;      // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_3_Rst = 1001;           // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_3_Stop = 1002;          // 平台一键停止（低位）
static constexpr std::int32_t HD_Plt_3_LocationDone = 1017;  // 平台一键定位完成（低位）
// 对位平台4
static constexpr std::int32_t HD_Plt_4_Enable = 1300;        // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_4_Back = 1300;          // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_4_Location = 1301;      // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_4_Rst = 1301;           // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_4_Stop = 1302;          // 平台一键停止（低位）
static constexpr std::int32_t HD_Plt_4_LocationDone = 1317;  // 平台一键定位完成（低位）
// 对位平台5
static constexpr std::int32_t HD_Plt_5_Enable = 1600;        // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_5_Back = 1600;          // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_5_Location = 1601;      // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_5_Rst = 1601;           // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_5_Stop = 1602;          // 平台一键停止（低位）
static constexpr std::int32_t HD_Plt_5_LocationDone = 1617;  // 平台一键定位完成（低位）
// 对位平台6
static constexpr std::int32_t HD_Plt_6_Enable = 1900;        // 平台一键使能（低位）
static constexpr std::int32_t HD_Plt_6_Back = 1900;          // 平台一键回参考点（高位）
static constexpr std::int32_t HD_Plt_6_Location = 1901;      // 平台一键定位（低位）
static constexpr std::int32_t HD_Plt_6_Rst = 1901;           // 平台一键复位（高位）
static constexpr std::int32_t HD_Plt_6_Stop = 1902;          // 平台一键停止（低位）
static constexpr std::int32_t HD_Plt_6_LocationDone = 1917;  // 平台一键定位完成（低位）

// 0号对位平台X轴
static constexpr std::int32_t HD_0_XAxis1Enable = 121;          // 0号对位平台X轴伺服使能（低位）
static constexpr std::int32_t HD_0_XAxis1EnableDone = 121;      // 0号对位平台X轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_0_XAxis1MOVER = 145;           // 0号对位平台X轴相对位置运动（高位）
static constexpr std::int32_t HD_0_XAxis1MOVERPosParams = 148;  // 0号对位平台X轴相对位置运动参数（位置）
static constexpr std::int32_t HD_0_XAxis1MOVERVelParams = 152;  // 0号对位平台X轴相对位置运动参数（速度）
static constexpr std::int32_t HD_0_XAxis1MOVERDone = 156;       // 0号对位平台X轴相对位置运动完成（低位）
static constexpr std::int32_t HD_0_XAxis1MOVEA = 157;           // 0号对位平台X轴速度运动（高位）
static constexpr std::int32_t HD_0_XAxis1MOVEAVelParams = 160;  // 0号对位平台X轴速度运动参数（速度）
static constexpr std::int32_t HD_0_XAxis1Stop = 174;            // 0号对位平台X轴停止(低位)
static constexpr std::int32_t HD_0_XAxis1Rst = 172;             // 0号对位平台X轴复位（低位）

// 0号对位平台Y轴
static constexpr std::int32_t HD_0_YAxis1Enable = 213;          // 0号对位平台Y轴伺服使能（低位）
static constexpr std::int32_t HD_0_YAxis1EnableDone = 213;      // 0号对位平台Y轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_0_YAxis1MOVER = 237;           // 0号对位平台Y轴相对位置运动（高位）
static constexpr std::int32_t HD_0_YAxis1MOVERPosParams = 240;  // 0号对位平台Y轴相对位置运动参数（位置）
static constexpr std::int32_t HD_0_YAxis1MOVERVelParams = 244;  // 0号对位平台Y轴相对位置运动参数（速度）
static constexpr std::int32_t HD_0_YAxis1MOVERDone = 248;       // 0号对位平台Y轴相对位置运动完成（低位）
static constexpr std::int32_t HD_0_YAxis1MOVEA = 249;           // 0号对位平台Y轴速度运动（高位）
static constexpr std::int32_t HD_0_YAxis1MOVEAVelParams = 252;  // 0号对位平台Y轴速度运动参数（速度）
static constexpr std::int32_t HD_0_YAxis1Stop = 266;            // 0号对位平台Y轴停止(低位)
static constexpr std::int32_t HD_0_YAxis1Rst = 264;             // 0号对位平台Y轴复位（低位）
// 0号对位平台旋转轴
static constexpr std::int32_t HD_0_RotAxis1Enable = 305;          // 0号对位平台旋转轴伺服使能（低位）
static constexpr std::int32_t HD_0_RotAxis1EnableDone = 305;      // 0号对位平台旋转轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_0_RotAxis1MOVER = 329;           // 0号对位平台旋转轴相对位置运动（高位）
static constexpr std::int32_t HD_0_RotAxis1MOVERPosParams = 332;  // 0号对位平台旋转轴相对位置运动参数（位置）
static constexpr std::int32_t HD_0_RotAxis1MOVERVelParams = 336;  // 0号对位平台旋转轴相对位置运动参数（速度）
static constexpr std::int32_t HD_0_RotAxis1MOVERDone = 340;       // 0号对位平台旋转轴相对位置运动完成（低位）
static constexpr std::int32_t HD_0_RotAxis1Stop = 358;            // 0号对位平台旋转轴停止(低位)
static constexpr std::int32_t HD_0_RotAxis1Rst = 356;             // 0号对位平台旋转轴复位（低位）

// 1号对位平台X轴（偏移 +300）
static constexpr std::int32_t HD_1_XAxis1Enable = 421;          // 1号对位平台X轴伺服使能（低位）
static constexpr std::int32_t HD_1_XAxis1EnableDone = 421;      // 1号对位平台X轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_1_XAxis1MOVER = 445;           // 1号对位平台X轴相对位置运动（高位）
static constexpr std::int32_t HD_1_XAxis1MOVERPosParams = 448;  // 1号对位平台X轴相对位置运动参数（位置）
static constexpr std::int32_t HD_1_XAxis1MOVERVelParams = 452;  // 1号对位平台X轴相对位置运动参数（速度）
static constexpr std::int32_t HD_1_XAxis1MOVERDone = 456;       // 1号对位平台X轴相对位置运动完成（低位）
static constexpr std::int32_t HD_1_XAxis1MOVEA = 457;           // 1号对位平台X轴速度运动（高位）
static constexpr std::int32_t HD_1_XAxis1MOVEAVelParams = 460;  // 1号对位平台X轴速度运动参数（速度）
static constexpr std::int32_t HD_1_XAxis1Stop = 474;            // 1号对位平台X轴停止(低位)
static constexpr std::int32_t HD_1_XAxis1Rst = 472;             // 1号对位平台X轴复位（低位）
// 1号对位平台Y轴
static constexpr std::int32_t HD_1_YAxis1Enable = 513;          // 1号对位平台Y轴伺服使能（低位）
static constexpr std::int32_t HD_1_YAxis1EnableDone = 513;      // 1号对位平台Y轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_1_YAxis1MOVER = 537;           // 1号对位平台Y轴相对位置运动（高位）
static constexpr std::int32_t HD_1_YAxis1MOVERPosParams = 540;  // 1号对位平台Y轴相对位置运动参数（位置）
static constexpr std::int32_t HD_1_YAxis1MOVERVelParams = 544;  // 1号对位平台Y轴相对位置运动参数（速度）
static constexpr std::int32_t HD_1_YAxis1MOVERDone = 548;       // 1号对位平台Y轴相对位置运动完成（低位）
static constexpr std::int32_t HD_1_YAxis1MOVEA = 549;           // 1号对位平台Y轴速度运动（高位）
static constexpr std::int32_t HD_1_YAxis1MOVEAVelParams = 552;  // 1号对位平台Y轴速度运动参数（速度）
static constexpr std::int32_t HD_1_YAxis1Stop = 566;            // 1号对位平台Y轴停止(低位)
static constexpr std::int32_t HD_1_YAxis1Rst = 564;             // 1号对位平台Y轴复位（低位）
// 1号对位平台旋转轴
static constexpr std::int32_t HD_1_RotAxis1Enable = 605;          // 1号对位平台旋转轴伺服使能（低位）
static constexpr std::int32_t HD_1_RotAxis1EnableDone = 605;      // 1号对位平台旋转轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_1_RotAxis1MOVER = 629;           // 1号对位平台旋转轴相对位置运动（高位）
static constexpr std::int32_t HD_1_RotAxis1MOVERPosParams = 632;  // 1号对位平台旋转轴相对位置运动参数（位置）
static constexpr std::int32_t HD_1_RotAxis1MOVERVelParams = 636;  // 1号对位平台旋转轴相对位置运动参数（速度）
static constexpr std::int32_t HD_1_RotAxis1MOVERDone = 640;       // 1号对位平台旋转轴相对位置运动完成（低位）
static constexpr std::int32_t HD_1_RotAxis1Stop = 658;            // 1号对位平台旋转轴停止(低位)
static constexpr std::int32_t HD_1_RotAxis1Rst = 656;             // 1号对位平台旋转轴复位（低位）

// 2号对位平台X轴（偏移 +600）
static constexpr std::int32_t HD_2_XAxis1Enable = 721;          // 2号对位平台X轴伺服使能（低位）
static constexpr std::int32_t HD_2_XAxis1EnableDone = 721;      // 2号对位平台X轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_2_XAxis1MOVER = 745;           // 2号对位平台X轴相对位置运动（高位）
static constexpr std::int32_t HD_2_XAxis1MOVERPosParams = 748;  // 2号对位平台X轴相对位置运动参数（位置）
static constexpr std::int32_t HD_2_XAxis1MOVERVelParams = 752;  // 2号对位平台X轴相对位置运动参数（速度）
static constexpr std::int32_t HD_2_XAxis1MOVERDone = 756;       // 2号对位平台X轴相对位置运动完成（低位）
static constexpr std::int32_t HD_2_XAxis1MOVEA = 757;           // 2号对位平台X轴速度运动（高位）
static constexpr std::int32_t HD_2_XAxis1MOVEAVelParams = 760;  // 2号对位平台X轴速度运动参数（速度）
static constexpr std::int32_t HD_2_XAxis1Stop = 774;            // 2号对位平台X轴停止(低位)
static constexpr std::int32_t HD_2_XAxis1Rst = 772;             // 2号对位平台X轴复位（低位）
// 2号对位平台Y轴
static constexpr std::int32_t HD_2_YAxis1Enable = 813;          // 2号对位平台Y轴伺服使能（低位）
static constexpr std::int32_t HD_2_YAxis1EnableDone = 813;      // 2号对位平台Y轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_2_YAxis1MOVER = 837;           // 2号对位平台Y轴相对位置运动（高位）
static constexpr std::int32_t HD_2_YAxis1MOVERPosParams = 840;  // 2号对位平台Y轴相对位置运动参数（位置）
static constexpr std::int32_t HD_2_YAxis1MOVERVelParams = 844;  // 2号对位平台Y轴相对位置运动参数（速度）
static constexpr std::int32_t HD_2_YAxis1MOVERDone = 848;       // 2号对位平台Y轴相对位置运动完成（低位）
static constexpr std::int32_t HD_2_YAxis1MOVEA = 849;           // 2号对位平台Y轴速度运动（高位）
static constexpr std::int32_t HD_2_YAxis1MOVEAVelParams = 852;  // 2号对位平台Y轴速度运动参数（速度）
static constexpr std::int32_t HD_2_YAxis1Stop = 866;            // 2号对位平台Y轴停止(低位)
static constexpr std::int32_t HD_2_YAxis1Rst = 864;             // 2号对位平台Y轴复位（低位）
// 2号对位平台旋转轴
static constexpr std::int32_t HD_2_RotAxis1Enable = 905;          // 2号对位平台旋转轴伺服使能（低位）
static constexpr std::int32_t HD_2_RotAxis1EnableDone = 905;      // 2号对位平台旋转轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_2_RotAxis1MOVER = 929;           // 2号对位平台旋转轴相对位置运动（高位）
static constexpr std::int32_t HD_2_RotAxis1MOVERPosParams = 932;  // 2号对位平台旋转轴相对位置运动参数（位置）
static constexpr std::int32_t HD_2_RotAxis1MOVERVelParams = 936;  // 2号对位平台旋转轴相对位置运动参数（速度）
static constexpr std::int32_t HD_2_RotAxis1MOVERDone = 940;       // 2号对位平台旋转轴相对位置运动完成（低位）
static constexpr std::int32_t HD_2_RotAxis1Stop = 958;            // 2号对位平台旋转轴停止(低位)
static constexpr std::int32_t HD_2_RotAxis1Rst = 956;             // 2号对位平台旋转轴复位（低位）

// 3号对位平台X轴（偏移 +900）
static constexpr std::int32_t HD_3_XAxis1Enable = 1021;          // 3号对位平台X轴伺服使能（低位）
static constexpr std::int32_t HD_3_XAxis1EnableDone = 1021;      // 3号对位平台X轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_3_XAxis1MOVER = 1045;           // 3号对位平台X轴相对位置运动（高位）
static constexpr std::int32_t HD_3_XAxis1MOVERPosParams = 1048;  // 3号对位平台X轴相对位置运动参数（位置）
static constexpr std::int32_t HD_3_XAxis1MOVERVelParams = 1052;  // 3号对位平台X轴相对位置运动参数（速度）
static constexpr std::int32_t HD_3_XAxis1MOVERDone = 1056;       // 3号对位平台X轴相对位置运动完成（低位）
static constexpr std::int32_t HD_3_XAxis1MOVEA = 1057;           // 3号对位平台X轴速度运动（高位）
static constexpr std::int32_t HD_3_XAxis1MOVEAVelParams = 1060;  // 3号对位平台X轴速度运动参数（速度）
static constexpr std::int32_t HD_3_XAxis1Stop = 1074;            // 3号对位平台X轴停止(低位)
static constexpr std::int32_t HD_3_XAxis1Rst = 1072;             // 3号对位平台X轴复位（低位）
// 3号对位平台Y轴
static constexpr std::int32_t HD_3_YAxis1Enable = 1113;          // 3号对位平台Y轴伺服使能（低位）
static constexpr std::int32_t HD_3_YAxis1EnableDone = 1113;      // 3号对位平台Y轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_3_YAxis1MOVER = 1137;           // 3号对位平台Y轴相对位置运动（高位）
static constexpr std::int32_t HD_3_YAxis1MOVERPosParams = 1140;  // 3号对位平台Y轴相对位置运动参数（位置）
static constexpr std::int32_t HD_3_YAxis1MOVERVelParams = 1144;  // 3号对位平台Y轴相对位置运动参数（速度）
static constexpr std::int32_t HD_3_YAxis1MOVERDone = 1148;       // 3号对位平台Y轴相对位置运动完成（低位）
static constexpr std::int32_t HD_3_YAxis1MOVEA = 1149;           // 3号对位平台Y轴速度运动（高位）
static constexpr std::int32_t HD_3_YAxis1MOVEAVelParams = 1152;  // 3号对位平台Y轴速度运动参数（速度）
static constexpr std::int32_t HD_3_YAxis1Stop = 1166;            // 3号对位平台Y轴停止(低位)
static constexpr std::int32_t HD_3_YAxis1Rst = 1164;             // 3号对位平台Y轴复位（低位）
// 3号对位平台旋转轴
static constexpr std::int32_t HD_3_RotAxis1Enable = 1205;          // 3号对位平台旋转轴伺服使能（低位）
static constexpr std::int32_t HD_3_RotAxis1EnableDone = 1205;      // 3号对位平台旋转轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_3_RotAxis1MOVER = 1229;           // 3号对位平台旋转轴相对位置运动（高位）
static constexpr std::int32_t HD_3_RotAxis1MOVERPosParams = 1232;  // 3号对位平台旋转轴相对位置运动参数（位置）
static constexpr std::int32_t HD_3_RotAxis1MOVERVelParams = 1236;  // 3号对位平台旋转轴相对位置运动参数（速度）
static constexpr std::int32_t HD_3_RotAxis1MOVERDone = 1240;       // 3号对位平台旋转轴相对位置运动完成（低位）
static constexpr std::int32_t HD_3_RotAxis1Stop = 1258;            // 3号对位平台旋转轴停止(低位)
static constexpr std::int32_t HD_3_RotAxis1Rst = 1256;             // 3号对位平台旋转轴复位（低位）

// 4号对位平台X轴（偏移 +1200）
static constexpr std::int32_t HD_4_XAxis1Enable = 1321;          // 4号对位平台X轴伺服使能（低位）
static constexpr std::int32_t HD_4_XAxis1EnableDone = 1321;      // 4号对位平台X轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_4_XAxis1MOVER = 1345;           // 4号对位平台X轴相对位置运动（高位）
static constexpr std::int32_t HD_4_XAxis1MOVERPosParams = 1348;  // 4号对位平台X轴相对位置运动参数（位置）
static constexpr std::int32_t HD_4_XAxis1MOVERVelParams = 1352;  // 4号对位平台X轴相对位置运动参数（速度）
static constexpr std::int32_t HD_4_XAxis1MOVERDone = 1356;       // 4号对位平台X轴相对位置运动完成（低位）
static constexpr std::int32_t HD_4_XAxis1MOVEA = 1357;           // 4号对位平台X轴速度运动（高位）
static constexpr std::int32_t HD_4_XAxis1MOVEAVelParams = 1360;  // 4号对位平台X轴速度运动参数（速度）
static constexpr std::int32_t HD_4_XAxis1Stop = 1374;            // 4号对位平台X轴停止(低位)
static constexpr std::int32_t HD_4_XAxis1Rst = 1372;             // 4号对位平台X轴复位（低位）
// 4号对位平台Y轴
static constexpr std::int32_t HD_4_YAxis1Enable = 1413;          // 4号对位平台Y轴伺服使能（低位）
static constexpr std::int32_t HD_4_YAxis1EnableDone = 1413;      // 4号对位平台Y轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_4_YAxis1MOVER = 1437;           // 4号对位平台Y轴相对位置运动（高位）
static constexpr std::int32_t HD_4_YAxis1MOVERPosParams = 1440;  // 4号对位平台Y轴相对位置运动参数（位置）
static constexpr std::int32_t HD_4_YAxis1MOVERVelParams = 1444;  // 4号对位平台Y轴相对位置运动参数（速度）
static constexpr std::int32_t HD_4_YAxis1MOVERDone = 1448;       // 4号对位平台Y轴相对位置运动完成（低位）
static constexpr std::int32_t HD_4_YAxis1MOVEA = 1449;           // 4号对位平台Y轴速度运动（高位）
static constexpr std::int32_t HD_4_YAxis1MOVEAVelParams = 1452;  // 4号对位平台Y轴速度运动参数（速度）
static constexpr std::int32_t HD_4_YAxis1Stop = 1466;            // 4号对位平台Y轴停止(低位)
static constexpr std::int32_t HD_4_YAxis1Rst = 1464;             // 4号对位平台Y轴复位（低位）
// 4号对位平台旋转轴
static constexpr std::int32_t HD_4_RotAxis1Enable = 1505;          // 4号对位平台旋转轴伺服使能（低位）
static constexpr std::int32_t HD_4_RotAxis1EnableDone = 1505;      // 4号对位平台旋转轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_4_RotAxis1MOVER = 1529;           // 4号对位平台旋转轴相对位置运动（高位）
static constexpr std::int32_t HD_4_RotAxis1MOVERPosParams = 1532;  // 4号对位平台旋转轴相对位置运动参数（位置）
static constexpr std::int32_t HD_4_RotAxis1MOVERVelParams = 1536;  // 4号对位平台旋转轴相对位置运动参数（速度）
static constexpr std::int32_t HD_4_RotAxis1MOVERDone = 1540;       // 4号对位平台旋转轴相对位置运动完成（低位）
static constexpr std::int32_t HD_4_RotAxis1Stop = 1558;            // 4号对位平台旋转轴停止(低位)
static constexpr std::int32_t HD_4_RotAxis1Rst = 1556;             // 4号对位平台旋转轴复位（低位）

// 5号对位平台X轴（偏移 +1500）
static constexpr std::int32_t HD_5_XAxis1Enable = 1621;          // 5号对位平台X轴伺服使能（低位）
static constexpr std::int32_t HD_5_XAxis1EnableDone = 1621;      // 5号对位平台X轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_5_XAxis1MOVER = 1645;           // 5号对位平台X轴相对位置运动（高位）
static constexpr std::int32_t HD_5_XAxis1MOVERPosParams = 1648;  // 5号对位平台X轴相对位置运动参数（位置）
static constexpr std::int32_t HD_5_XAxis1MOVERVelParams = 1652;  // 5号对位平台X轴相对位置运动参数（速度）
static constexpr std::int32_t HD_5_XAxis1MOVERDone = 1656;       // 5号对位平台X轴相对位置运动完成（低位）
static constexpr std::int32_t HD_5_XAxis1MOVEA = 1657;           // 5号对位平台X轴速度运动（高位）
static constexpr std::int32_t HD_5_XAxis1MOVEAVelParams = 1660;  // 5号对位平台X轴速度运动参数（速度）
static constexpr std::int32_t HD_5_XAxis1Stop = 1674;            // 5号对位平台X轴停止(低位)
static constexpr std::int32_t HD_5_XAxis1Rst = 1672;             // 5号对位平台X轴复位（低位）
// 5号对位平台Y轴
static constexpr std::int32_t HD_5_YAxis1Enable = 1713;          // 5号对位平台Y轴伺服使能（低位）
static constexpr std::int32_t HD_5_YAxis1EnableDone = 1713;      // 5号对位平台Y轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_5_YAxis1MOVER = 1737;           // 5号对位平台Y轴相对位置运动（高位）
static constexpr std::int32_t HD_5_YAxis1MOVERPosParams = 1740;  // 5号对位平台Y轴相对位置运动参数（位置）
static constexpr std::int32_t HD_5_YAxis1MOVERVelParams = 1744;  // 5号对位平台Y轴相对位置运动参数（速度）
static constexpr std::int32_t HD_5_YAxis1MOVERDone = 1748;       // 5号对位平台Y轴相对位置运动完成（低位）
static constexpr std::int32_t HD_5_YAxis1MOVEA = 1749;           // 5号对位平台Y轴速度运动（高位）
static constexpr std::int32_t HD_5_YAxis1MOVEAVelParams = 1752;  // 5号对位平台Y轴速度运动参数（速度）
static constexpr std::int32_t HD_5_YAxis1Stop = 1766;            // 5号对位平台Y轴停止(低位)
static constexpr std::int32_t HD_5_YAxis1Rst = 1764;             // 5号对位平台Y轴复位（低位）
// 5号对位平台旋转轴
static constexpr std::int32_t HD_5_RotAxis1Enable = 1805;          // 5号对位平台旋转轴伺服使能（低位）
static constexpr std::int32_t HD_5_RotAxis1EnableDone = 1805;      // 5号对位平台旋转轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_5_RotAxis1MOVER = 1829;           // 5号对位平台旋转轴相对位置运动（高位）
static constexpr std::int32_t HD_5_RotAxis1MOVERPosParams = 1832;  // 5号对位平台旋转轴相对位置运动参数（位置）
static constexpr std::int32_t HD_5_RotAxis1MOVERVelParams = 1836;  // 5号对位平台旋转轴相对位置运动参数（速度）
static constexpr std::int32_t HD_5_RotAxis1MOVERDone = 1840;       // 5号对位平台旋转轴相对位置运动完成（低位）
static constexpr std::int32_t HD_5_RotAxis1Stop = 1858;            // 5号对位平台旋转轴停止(低位)
static constexpr std::int32_t HD_5_RotAxis1Rst = 1856;             // 5号对位平台旋转轴复位（低位）

// 6号对位平台X轴（偏移 +1800）
static constexpr std::int32_t HD_6_XAxis1Enable = 1921;          // 6号对位平台X轴伺服使能（低位）
static constexpr std::int32_t HD_6_XAxis1EnableDone = 1921;      // 6号对位平台X轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_6_XAxis1MOVER = 1945;           // 6号对位平台X轴相对位置运动（高位）
static constexpr std::int32_t HD_6_XAxis1MOVERPosParams = 1948;  // 6号对位平台X轴相对位置运动参数（位置）
static constexpr std::int32_t HD_6_XAxis1MOVERVelParams = 1952;  // 6号对位平台X轴相对位置运动参数（速度）
static constexpr std::int32_t HD_6_XAxis1MOVERDone = 1956;       // 6号对位平台X轴相对位置运动完成（低位）
static constexpr std::int32_t HD_6_XAxis1MOVEA = 1957;           // 6号对位平台X轴速度运动（高位）
static constexpr std::int32_t HD_6_XAxis1MOVEAVelParams = 1960;  // 6号对位平台X轴速度运动参数（速度）
static constexpr std::int32_t HD_6_XAxis1Stop = 1974;            // 6号对位平台X轴停止(低位)
static constexpr std::int32_t HD_6_XAxis1Rst = 1972;             // 6号对位平台X轴复位（低位）
// 6号对位平台Y轴
static constexpr std::int32_t HD_6_YAxis1Enable = 2013;          // 6号对位平台Y轴伺服使能（低位）
static constexpr std::int32_t HD_6_YAxis1EnableDone = 2013;      // 6号对位平台Y轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_6_YAxis1MOVER = 2037;           // 6号对位平台Y轴相对位置运动（高位）
static constexpr std::int32_t HD_6_YAxis1MOVERPosParams = 2040;  // 6号对位平台Y轴相对位置运动参数（位置）
static constexpr std::int32_t HD_6_YAxis1MOVERVelParams = 2044;  // 6号对位平台Y轴相对位置运动参数（速度）
static constexpr std::int32_t HD_6_YAxis1MOVERDone = 2048;       // 6号对位平台Y轴相对位置运动完成（低位）
static constexpr std::int32_t HD_6_YAxis1MOVEA = 2049;           // 6号对位平台Y轴速度运动（高位）
static constexpr std::int32_t HD_6_YAxis1MOVEAVelParams = 2052;  // 6号对位平台Y轴速度运动参数（速度）
static constexpr std::int32_t HD_6_YAxis1Stop = 2066;            // 6号对位平台Y轴停止(低位)
static constexpr std::int32_t HD_6_YAxis1Rst = 2064;             // 6号对位平台Y轴复位（低位）
// 6号对位平台旋转轴
static constexpr std::int32_t HD_6_RotAxis1Enable = 2105;          // 6号对位平台旋转轴伺服使能（低位）
static constexpr std::int32_t HD_6_RotAxis1EnableDone = 2105;      // 6号对位平台旋转轴伺服使能完成信号位（高位）
static constexpr std::int32_t HD_6_RotAxis1MOVER = 2129;           // 6号对位平台旋转轴相对位置运动（高位）
static constexpr std::int32_t HD_6_RotAxis1MOVERPosParams = 2132;  // 6号对位平台旋转轴相对位置运动参数（位置）
static constexpr std::int32_t HD_6_RotAxis1MOVERVelParams = 2136;  // 6号对位平台旋转轴相对位置运动参数（速度）
static constexpr std::int32_t HD_6_RotAxis1MOVERDone = 2140;       // 6号对位平台旋转轴相对位置运动完成（低位）
static constexpr std::int32_t HD_6_RotAxis1Stop = 2158;            // 6号对位平台旋转轴停止(低位)
static constexpr std::int32_t HD_6_RotAxis1Rst = 2156;             // 6号对位平台旋转轴复位（低位）

}  // namespace plc

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

// 外部轴1
static constexpr std::int32_t M_ExAxis1Enable = 400;           // 外部轴1伺服使能
static constexpr std::int32_t M_ExAxis1ZRN = 410;              // 外部轴1机械归零
static constexpr std::int32_t M_ExAxis1MOVEA = 420;            // 外部轴1绝对位置运动
static constexpr std::int32_t D_ExAxis1MOVEAParams = 4200;     // 外部轴1绝对位置运动参数
static constexpr std::int32_t M_ExAxis1FOLOOW = 430;           // 外部轴1跟随运动
static constexpr std::int32_t D_ExAxis1FOLOOWParams = 4300;    // 外部轴1跟随运动参数
static constexpr std::int32_t M_ExAxis1Halt = 440;             // 外部轴1暂停
static constexpr std::int32_t D_ExAxis1HaltParams = 4400;      // 外部轴1暂停参数
static constexpr std::int32_t M_ExAxis1Stop = 450;             // 外部轴1停止
static constexpr std::int32_t D_ExAxis1StopParams = 4500;      // 外部轴1停止参数
static constexpr std::int32_t M_ExAxis1Rst = 460;              // 外部轴1复位
static constexpr std::int32_t D_ExAxis1Pos = 20044 + 200 * 0;  // 外部轴1实际运动位置
static constexpr std::int32_t D_ExAxis1Vel = 20048 + 200 * 0;  // 外部轴1实际运动速度

}  // namespace plc

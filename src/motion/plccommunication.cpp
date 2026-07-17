#include "plccommunication.h"

#include <QThread>
#include <cstring>
#include <iostream>

#include "plc_addr.h"
#include "plccommon.h"

using namespace plc;

// ============================================================
//  构造 / 析构
// ============================================================

PlcCommunication::PlcCommunication(plcCtrlParams *ctrl) : params_(ctrl) {
    if (!params_) return;
    modbusTcp_ = modbus_new_tcp(params_->plc.ip.c_str(), params_->plc.port);
    if (!modbusTcp_) return;
    modbus_set_response_timeout(modbusTcp_, 0, 200 * 1000);
}

PlcCommunication::~PlcCommunication() {
    if (modbusTcp_) {
        modbus_close(modbusTcp_);
        modbus_free(modbusTcp_);
        modbusTcp_ = nullptr;
    }
}

// ============================================================
//  连接
// ============================================================

bool PlcCommunication::plcConnect() {
    if (!modbusTcp_) return false;
    if (connectStatus_) return true;
    if (modbus_connect(modbusTcp_) == -1) {
        connectStatus_ = false;
        return false;
    }
    connectStatus_ = true;
    return true;
}

bool PlcCommunication::plcDisconnect() {
    if (!modbusTcp_) return true;
    if (connectStatus_) {
        modbus_close(modbusTcp_);
        connectStatus_ = false;
    }
    return true;
}

bool PlcCommunication::plcReset() {
    if (!modbusTcp_ || !connectStatus_) return false;
    // 清除工件位置数据和复位
    return writeRegisterBit(HDAddr + M_Init, 1) == 0;
}

// ============================================================
//  地轨
//  地址见 plc_addr.h，都是 HD 寄存器，基地址 HDAddr(41088)
//  位定义：低位(bit0) = 触发/使能，高位(bit8) = 回参/回零
// ============================================================

bool PlcCommunication::railEnable() { return writeHdLowBit(HD_ExAxis1Enable, true) == 0; }

bool PlcCommunication::railDisable() { return writeHdLowBit(HD_ExAxis1Enable, false) == 0; }

bool PlcCommunication::railAbsMove(double pos, double vel, double acc, double jerk) {
    if (!modbusTcp_ || !connectStatus_) return false;

    // 写参数: HD_ExAxis1MOVEAPosParams(2216)=pos, HD_ExAxis1MOVEAVelParams(2220)=vel
    // 每个 double 占 4 个 HD 寄存器
    uint16_t posData[4] = {0}, velData[4] = {0};
    dataTransDouble_UInt16(pos, posData);
    dataTransDouble_UInt16(vel, velData);

    // 位置(2216)、速度(2220) 依次写入（acc/jerk 占 2224/2228 留给完成标志位，不写）
    if (writeRegisters(HDAddr + HD_ExAxis1MOVEAPosParams, 4, posData) != 0) return false;
    if (writeRegisters(HDAddr + HD_ExAxis1MOVEAVelParams, 4, velData) != 0) return false;

    // 触发运动: HD_ExAxis1MOVEA(2215) 低位，延迟等 PLC 捕捉再清零
    writeHdLowBit(HD_ExAxis1MOVEA, true);
    QThread::msleep(50);
    return writeHdLowBit(HD_ExAxis1MOVEA, false) == 0;
}

bool PlcCommunication::railHome() { return writeHdLowBit(HD_ExAxis1ZRN, true) == 0; }

bool PlcCommunication::railStop(double /*dec*/, double /*jerk*/) {
    writeHdLowBit(HD_ExAxis1Stop, true);
    for (int i = 0; i < 50; ++i) {
        QThread::msleep(100);
        if (readHdHighBit(HD_ExAxis1Stop) == 1) break;
    }
    writeHdLowBit(HD_ExAxis1Stop, false);
    return true;
}

bool PlcCommunication::railReset() { return writeHdLowBit(HD_ExAxis1Rst, true) == 0; }

// ============================================================
//  对位平台总控 (pltIdx: 0~6)
//  地址通过 plc_addr.h 的 HD_Plt_{N}_* 常量数组索引
// ============================================================

// 辅助：获取平台 N 的总控地址
namespace {

struct PltAddr {
    int enable, back, locate, rst, stop, locateDone;
};

const PltAddr pltAddrs[7] = {
    {HD_Plt_0_Enable, HD_Plt_0_Back, HD_Plt_0_Location, HD_Plt_0_Rst, HD_Plt_0_Stop, HD_Plt_0_LocationDone},
    {HD_Plt_1_Enable, HD_Plt_1_Back, HD_Plt_1_Location, HD_Plt_1_Rst, HD_Plt_1_Stop, HD_Plt_1_LocationDone},
    {HD_Plt_2_Enable, HD_Plt_2_Back, HD_Plt_2_Location, HD_Plt_2_Rst, HD_Plt_2_Stop, HD_Plt_2_LocationDone},
    {HD_Plt_3_Enable, HD_Plt_3_Back, HD_Plt_3_Location, HD_Plt_3_Rst, HD_Plt_3_Stop, HD_Plt_3_LocationDone},
    {HD_Plt_4_Enable, HD_Plt_4_Back, HD_Plt_4_Location, HD_Plt_4_Rst, HD_Plt_4_Stop, HD_Plt_4_LocationDone},
    {HD_Plt_5_Enable, HD_Plt_5_Back, HD_Plt_5_Location, HD_Plt_5_Rst, HD_Plt_5_Stop, HD_Plt_5_LocationDone},
    {HD_Plt_6_Enable, HD_Plt_6_Back, HD_Plt_6_Location, HD_Plt_6_Rst, HD_Plt_6_Stop, HD_Plt_6_LocationDone},
};

}  // namespace

// ============================================================
//  单轴控制 — 7 个平台的轴地址数组
//  每个平台 3 轴 (X=0, Y=1, R=2)，间距 300
// ============================================================

namespace {

struct AxisAddr {
    int enable, enableDone, mover, moverPos, moverVel, moverDone, movea, moveaVel, stop, rst;
};

const AxisAddr allAxisAddrs[7][3] = {
    // 平台 0
    {{HD_0_XAxis1Enable, HD_0_XAxis1EnableDone, HD_0_XAxis1MOVER, HD_0_XAxis1MOVERPosParams, HD_0_XAxis1MOVERVelParams, HD_0_XAxis1MOVERDone,
      HD_0_XAxis1MOVEA, HD_0_XAxis1MOVEAVelParams, HD_0_XAxis1Stop, HD_0_XAxis1Rst},
     {HD_0_YAxis1Enable, HD_0_YAxis1EnableDone, HD_0_YAxis1MOVER, HD_0_YAxis1MOVERPosParams, HD_0_YAxis1MOVERVelParams, HD_0_YAxis1MOVERDone,
      HD_0_YAxis1MOVEA, HD_0_YAxis1MOVEAVelParams, HD_0_YAxis1Stop, HD_0_YAxis1Rst},
     {HD_0_RotAxis1Enable, HD_0_RotAxis1EnableDone, HD_0_RotAxis1MOVER, HD_0_RotAxis1MOVERPosParams, HD_0_RotAxis1MOVERVelParams,
      HD_0_RotAxis1MOVERDone, 0, 0, HD_0_RotAxis1Stop, HD_0_RotAxis1Rst}},
    // 平台 1
    {{HD_1_XAxis1Enable, HD_1_XAxis1EnableDone, HD_1_XAxis1MOVER, HD_1_XAxis1MOVERPosParams, HD_1_XAxis1MOVERVelParams, HD_1_XAxis1MOVERDone,
      HD_1_XAxis1MOVEA, HD_1_XAxis1MOVEAVelParams, HD_1_XAxis1Stop, HD_1_XAxis1Rst},
     {HD_1_YAxis1Enable, HD_1_YAxis1EnableDone, HD_1_YAxis1MOVER, HD_1_YAxis1MOVERPosParams, HD_1_YAxis1MOVERVelParams, HD_1_YAxis1MOVERDone,
      HD_1_YAxis1MOVEA, HD_1_YAxis1MOVEAVelParams, HD_1_YAxis1Stop, HD_1_YAxis1Rst},
     {HD_1_RotAxis1Enable, HD_1_RotAxis1EnableDone, HD_1_RotAxis1MOVER, HD_1_RotAxis1MOVERPosParams, HD_1_RotAxis1MOVERVelParams,
      HD_1_RotAxis1MOVERDone, 0, 0, HD_1_RotAxis1Stop, HD_1_RotAxis1Rst}},
    // 平台 2
    {{HD_2_XAxis1Enable, HD_2_XAxis1EnableDone, HD_2_XAxis1MOVER, HD_2_XAxis1MOVERPosParams, HD_2_XAxis1MOVERVelParams, HD_2_XAxis1MOVERDone,
      HD_2_XAxis1MOVEA, HD_2_XAxis1MOVEAVelParams, HD_2_XAxis1Stop, HD_2_XAxis1Rst},
     {HD_2_YAxis1Enable, HD_2_YAxis1EnableDone, HD_2_YAxis1MOVER, HD_2_YAxis1MOVERPosParams, HD_2_YAxis1MOVERVelParams, HD_2_YAxis1MOVERDone,
      HD_2_YAxis1MOVEA, HD_2_YAxis1MOVEAVelParams, HD_2_YAxis1Stop, HD_2_YAxis1Rst},
     {HD_2_RotAxis1Enable, HD_2_RotAxis1EnableDone, HD_2_RotAxis1MOVER, HD_2_RotAxis1MOVERPosParams, HD_2_RotAxis1MOVERVelParams,
      HD_2_RotAxis1MOVERDone, 0, 0, HD_2_RotAxis1Stop, HD_2_RotAxis1Rst}},
    // 平台 3
    {{HD_3_XAxis1Enable, HD_3_XAxis1EnableDone, HD_3_XAxis1MOVER, HD_3_XAxis1MOVERPosParams, HD_3_XAxis1MOVERVelParams, HD_3_XAxis1MOVERDone,
      HD_3_XAxis1MOVEA, HD_3_XAxis1MOVEAVelParams, HD_3_XAxis1Stop, HD_3_XAxis1Rst},
     {HD_3_YAxis1Enable, HD_3_YAxis1EnableDone, HD_3_YAxis1MOVER, HD_3_YAxis1MOVERPosParams, HD_3_YAxis1MOVERVelParams, HD_3_YAxis1MOVERDone,
      HD_3_YAxis1MOVEA, HD_3_YAxis1MOVEAVelParams, HD_3_YAxis1Stop, HD_3_YAxis1Rst},
     {HD_3_RotAxis1Enable, HD_3_RotAxis1EnableDone, HD_3_RotAxis1MOVER, HD_3_RotAxis1MOVERPosParams, HD_3_RotAxis1MOVERVelParams,
      HD_3_RotAxis1MOVERDone, 0, 0, HD_3_RotAxis1Stop, HD_3_RotAxis1Rst}},
    // 平台 4
    {{HD_4_XAxis1Enable, HD_4_XAxis1EnableDone, HD_4_XAxis1MOVER, HD_4_XAxis1MOVERPosParams, HD_4_XAxis1MOVERVelParams, HD_4_XAxis1MOVERDone,
      HD_4_XAxis1MOVEA, HD_4_XAxis1MOVEAVelParams, HD_4_XAxis1Stop, HD_4_XAxis1Rst},
     {HD_4_YAxis1Enable, HD_4_YAxis1EnableDone, HD_4_YAxis1MOVER, HD_4_YAxis1MOVERPosParams, HD_4_YAxis1MOVERVelParams, HD_4_YAxis1MOVERDone,
      HD_4_YAxis1MOVEA, HD_4_YAxis1MOVEAVelParams, HD_4_YAxis1Stop, HD_4_YAxis1Rst},
     {HD_4_RotAxis1Enable, HD_4_RotAxis1EnableDone, HD_4_RotAxis1MOVER, HD_4_RotAxis1MOVERPosParams, HD_4_RotAxis1MOVERVelParams,
      HD_4_RotAxis1MOVERDone, 0, 0, HD_4_RotAxis1Stop, HD_4_RotAxis1Rst}},
    // 平台 5
    {{HD_5_XAxis1Enable, HD_5_XAxis1EnableDone, HD_5_XAxis1MOVER, HD_5_XAxis1MOVERPosParams, HD_5_XAxis1MOVERVelParams, HD_5_XAxis1MOVERDone,
      HD_5_XAxis1MOVEA, HD_5_XAxis1MOVEAVelParams, HD_5_XAxis1Stop, HD_5_XAxis1Rst},
     {HD_5_YAxis1Enable, HD_5_YAxis1EnableDone, HD_5_YAxis1MOVER, HD_5_YAxis1MOVERPosParams, HD_5_YAxis1MOVERVelParams, HD_5_YAxis1MOVERDone,
      HD_5_YAxis1MOVEA, HD_5_YAxis1MOVEAVelParams, HD_5_YAxis1Stop, HD_5_YAxis1Rst},
     {HD_5_RotAxis1Enable, HD_5_RotAxis1EnableDone, HD_5_RotAxis1MOVER, HD_5_RotAxis1MOVERPosParams, HD_5_RotAxis1MOVERVelParams,
      HD_5_RotAxis1MOVERDone, 0, 0, HD_5_RotAxis1Stop, HD_5_RotAxis1Rst}},
    // 平台 6
    {{HD_6_XAxis1Enable, HD_6_XAxis1EnableDone, HD_6_XAxis1MOVER, HD_6_XAxis1MOVERPosParams, HD_6_XAxis1MOVERVelParams, HD_6_XAxis1MOVERDone,
      HD_6_XAxis1MOVEA, HD_6_XAxis1MOVEAVelParams, HD_6_XAxis1Stop, HD_6_XAxis1Rst},
     {HD_6_YAxis1Enable, HD_6_YAxis1EnableDone, HD_6_YAxis1MOVER, HD_6_YAxis1MOVERPosParams, HD_6_YAxis1MOVERVelParams, HD_6_YAxis1MOVERDone,
      HD_6_YAxis1MOVEA, HD_6_YAxis1MOVEAVelParams, HD_6_YAxis1Stop, HD_6_YAxis1Rst},
     {HD_6_RotAxis1Enable, HD_6_RotAxis1EnableDone, HD_6_RotAxis1MOVER, HD_6_RotAxis1MOVERPosParams, HD_6_RotAxis1MOVERVelParams,
      HD_6_RotAxis1MOVERDone, 0, 0, HD_6_RotAxis1Stop, HD_6_RotAxis1Rst}},
};

}  // namespace

bool PlcCommunication::pltEnable(int pltIdx) { return writeHdLowBit(pltAddrs[pltIdx].enable, true) == 0; }

bool PlcCommunication::pltDisable(int pltIdx) { return writeHdLowBit(pltAddrs[pltIdx].enable, false) == 0; }

bool PlcCommunication::pltHome(int pltIdx) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6) return false;

    // X/Y 轴: 速度运动寄存器写 -1，相对位置写 10，相对速度写 5
    uint16_t velMinus1[4] = {0}, val10[4] = {0}, vel5[4] = {0};
    dataTransDouble_UInt16(-1.0, velMinus1);
    dataTransDouble_UInt16(10.0, val10);
    dataTransDouble_UInt16(5.0, vel5);

    const auto &x = allAxisAddrs[pltIdx][0];
    const auto &y = allAxisAddrs[pltIdx][1];

    // X 轴
    writeRegisters(HDAddr + x.moveaVel, 4, velMinus1);  // 速度运动 = -1
    writeRegisters(HDAddr + x.moverPos, 4, val10);      // 相对位置 = 10
    writeRegisters(HDAddr + x.moverVel, 4, vel5);       // 相对速度 = 5

    // Y 轴
    writeRegisters(HDAddr + y.moveaVel, 4, velMinus1);
    writeRegisters(HDAddr + y.moverPos, 4, val10);
    writeRegisters(HDAddr + y.moverVel, 4, vel5);

    // 触发回参考点（PLC 程序内部已包含使能）
    return writeHdHighBit(pltAddrs[pltIdx].back, true) == 0;  // 位8: 回参考点
}

bool PlcCommunication::pltLocate(int pltIdx, double x, double y, double r, double vel, double acc, double jerk) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6) return false;

    uint16_t data[4] = {0};
    uint16_t vel5[4] = {0};
    dataTransDouble_UInt16(5.0, vel5);

    // 写好速度和
    const auto &ra = allAxisAddrs[pltIdx][2];  // R
    dataTransDouble_UInt16(r, data);
    writeRegisters(HDAddr + ra.moverPos, 4, data);
    writeRegisters(HDAddr + ra.moverVel, 4, vel5);

    const auto &xa = allAxisAddrs[pltIdx][0];  // X
    dataTransDouble_UInt16(x, data);
    writeRegisters(HDAddr + xa.moverPos, 4, data);
    writeRegisters(HDAddr + xa.moverVel, 4, vel5);

    const auto &ya = allAxisAddrs[pltIdx][1];  // Y
    dataTransDouble_UInt16(y, data);
    writeRegisters(HDAddr + ya.moverPos, 4, data);
    writeRegisters(HDAddr + ya.moverVel, 4, vel5);

    writeHdLowBit(ra.enable, true);
    writeHdLowBit(xa.enable, true);
    writeHdLowBit(ya.enable, true);
    writeHdHighBit(ra.mover, true);
    writeHdHighBit(xa.mover, true);
    writeHdHighBit(ya.mover, true);

    return writeHdLowBit(pltAddrs[pltIdx].locate, true) == 0;
}

bool PlcCommunication::pltLocatePos(int pltIdx, double x, double y, double r, double vel) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6) return false;

    uint16_t data[4] = {0};
    uint16_t velData[4] = {0};
    dataTransDouble_UInt16(vel, velData);

    const auto &xa = allAxisAddrs[pltIdx][0];
    dataTransDouble_UInt16(x, data);
    writeRegisters(HDAddr + xa.moverPos, 4, data);
    writeRegisters(HDAddr + xa.moverVel, 4, velData);

    const auto &ya = allAxisAddrs[pltIdx][1];
    dataTransDouble_UInt16(y, data);
    writeRegisters(HDAddr + ya.moverPos, 4, data);
    writeRegisters(HDAddr + ya.moverVel, 4, velData);

    const auto &ra = allAxisAddrs[pltIdx][2];
    dataTransDouble_UInt16(r, data);
    writeRegisters(HDAddr + ra.moverPos, 4, data);
    writeRegisters(HDAddr + ra.moverVel, 4, velData);

    writeHdLowBit(pltAddrs[pltIdx].locate, true);

    return true;
}

bool PlcCommunication::pltIsLocationDone(int pltIdx) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6) return false;
    return readHdLowBit(pltAddrs[pltIdx].locateDone) == 1;
}

bool PlcCommunication::pltStepAxis(int pltIdx, int axis, double pos, double vel) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6 || axis < 0 || axis > 2) return false;

    const auto &a = allAxisAddrs[pltIdx][axis];

    uint16_t posData[4] = {0}, velData[4] = {0};
    dataTransDouble_UInt16(pos, posData);
    dataTransDouble_UInt16(vel, velData);

    // 写使能（低位）
    writeHdLowBit(a.enable, true);

    // 写位置和速度（各4个寄存器，连续）
    if (writeRegisters(HDAddr + a.moverPos, 4, posData) != 0) return false;
    if (writeRegisters(HDAddr + a.moverVel, 4, velData) != 0) return false;

    // 触发运动（高位）
    return writeHdHighBit(a.mover, true) == 0;
}

bool PlcCommunication::pltIsMoverDone(int pltIdx, int axis) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6 || axis < 0 || axis > 2) return false;

    const auto &a = allAxisAddrs[pltIdx][axis];
    // moverDone 低位 = 1 表示完成
    return readHdLowBit(a.moverDone) == 1;
}

bool PlcCommunication::pltClearMover(int pltIdx, int axis) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6 || axis < 0 || axis > 2) return false;

    const auto &a = allAxisAddrs[pltIdx][axis];
    // 清除触发位（高位）
    return writeHdHighBit(a.mover, false) == 0;
}

QVector<QVector<bool>> PlcCommunication::readAllAxisEnableDone() {
    QVector<QVector<bool>> result(7, QVector<bool>(3, false));
    if (!modbusTcp_ || !connectStatus_) return result;

    static bool loggedOnce = false;
    for (int plt = 0; plt < 7; ++plt) {
        for (int axis = 0; axis < 3; ++axis) {
            const auto &a = allAxisAddrs[plt][axis];
            int val = readHdHighBit(a.enableDone);
            result[plt][axis] = (val == 1);
            if (!loggedOnce) {
                fprintf(stdout, "[PltStatus] plt%d axis%d enableDone addr=%d raw=%d\n",
                        plt, axis, a.enableDone, val);
            }
        }
    }
    if (!loggedOnce) {
        fflush(stdout);
        loggedOnce = true;
    }
    return result;
}

bool PlcCommunication::pltStop(int pltIdx) { return writeHdLowBit(pltAddrs[pltIdx].stop, true) == 0; }

bool PlcCommunication::pltReset(int pltIdx) { return writeHdHighBit(pltAddrs[pltIdx].rst, true) == 0; }

// ============================================================
//  单轴控制
// ============================================================

bool PlcCommunication::axisMoveR(int pltIdx, int axis, double pos, double vel, double acc, double jerk) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6 || axis < 0 || axis > 2) return false;

    const auto &a = allAxisAddrs[pltIdx][axis];

    uint16_t data[4] = {0};
    uint16_t vel5[4] = {0};
    dataTransDouble_UInt16(5.0, vel5);
    dataTransDouble_UInt16(pos, data);

    writeRegisters(HDAddr + a.moverPos, 4, data);
    writeRegisters(HDAddr + a.moverVel, 4, vel5);

    // 写使能，等待使能完成（高位=使能完成信号），超时 5 秒
    writeHdLowBit(a.enable, true);
    for (int i = 0; i < 50; ++i) {
        QThread::msleep(100);
        if (readHdHighBit(a.enableDone) == 1) break;
    }

    // 触发相对运动，等待完成（低位=完成信号），超时 5 秒，然后清零触发位
    writeHdHighBit(a.mover, true);
    for (int i = 0; i < 50; ++i) {
        QThread::msleep(100);
        if (readHdLowBit(a.moverDone) == 1) break;
    }
    writeHdHighBit(a.mover, false);

    return true;
}

bool PlcCommunication::axisStop(int pltIdx, int axis) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6 || axis < 0 || axis > 2) return false;
    const auto &a = allAxisAddrs[pltIdx][axis];
    return writeHdLowBit(a.stop, true) == 0;
}

bool PlcCommunication::axisReset(int pltIdx, int axis) {
    if (!modbusTcp_ || !connectStatus_) return false;
    if (pltIdx < 0 || pltIdx > 6 || axis < 0 || axis > 2) return false;
    const auto &a = allAxisAddrs[pltIdx][axis];
    return writeHdLowBit(a.enable, true) == 0;
}

// ============================================================
//  批量控制
// ============================================================

bool PlcCommunication::pltEnableAll() {
    for (int i = 0; i < 7; ++i) {
        pltEnable(i);
        if (i < 6) QThread::msleep(50);
    }
    return true;
}

bool PlcCommunication::pltDisableAll() {
    for (int i = 0; i < 7; ++i) {
        writeHdLowBit(pltAddrs[i].enable, false);
        if (i < 6) QThread::msleep(50);
    }
    return true;
}

bool PlcCommunication::pltHomeAll() {
    for (int i = 0; i < 7; ++i) {
        pltHome(i);
        if (i < 6) QThread::msleep(50);
    }
    return true;
}

bool PlcCommunication::pltResetAll() {
    for (int i = 0; i < 7; ++i) {
        pltReset(i);
        if (i < 6) QThread::msleep(50);
    }
    return true;
}

bool PlcCommunication::pltStopAll() {
    for (int i = 0; i < 7; ++i) {
        pltStop(i);
        if (i < 6) QThread::msleep(50);
    }
    return true;
}

// ============================================================
//  状态反馈
// ============================================================

bool PlcCommunication::plcStateFdbk(plcFdbkParams &data) {
    if (!modbusTcp_ || !connectStatus_) return false;

    data.connectStatus = connectStatus_ ? 1 : 0;
    data.runStatus = readRegisterBit(SMAddr + SM_Run);

    // 地轨
    data.rail.enable = readHdLowBit(HD_ExAxis1Enable);
    data.rail.pos = readRegisterDouble(DAddr + D_ExAxis1Pos);
    data.rail.vel = readRegisterDouble(DAddr + D_ExAxis1Vel);

    return true;
}

// ============================================================
//  内部辅助
// ============================================================

void PlcCommunication::tryToConnect() {
    if (!modbusTcp_) return;
    connectStatus_ = false;
    if (modbus_connect(modbusTcp_) != -1) connectStatus_ = true;
}

// ============================================================
//  HD 寄存器位操作
//  每个 HD 寄存器 16 位，位 0~7 为低位，位 8~15 为高位
//  写入时需读-改-写，避免影响同寄存器的其他位
// ============================================================

int PlcCommunication::writeHdBit(int hdReg, int bitIndex, bool val) {
    if (!modbusTcp_ || !connectStatus_) return -1;

    int addr = HDAddr + hdReg;

    uint16_t cur = 0;
    if (modbus_read_registers(modbusTcp_, addr, 1, &cur) == -1) {
        modbus_close(modbusTcp_);
        tryToConnect();
        return -1;
    }

    if (val)
        cur |= (1 << bitIndex);
    else
        cur &= ~(1 << bitIndex);

    return writeRegisters(addr, 1, &cur);
}

int PlcCommunication::writeHdLowBit(int hdReg, bool val) { return writeHdBit(hdReg, 0, val); }

int PlcCommunication::writeHdHighBit(int hdReg, bool val) { return writeHdBit(hdReg, 8, val); }

int PlcCommunication::readHdBit(int hdReg, int bitIndex) {
    if (!modbusTcp_ || !connectStatus_) return -1;

    int addr = HDAddr + hdReg;

    uint16_t cur = 0;
    if (modbus_read_registers(modbusTcp_, addr, 1, &cur) == -1) {
        modbus_close(modbusTcp_);
        tryToConnect();
        return -1;
    }
    return (cur >> bitIndex) & 1;
}

int PlcCommunication::readHdLowBit(int hdReg) { return readHdBit(hdReg, 0); }

int PlcCommunication::readHdHighBit(int hdReg) { return readHdBit(hdReg, 8); }

// ============================================================
//  Modbus 读写
// ============================================================

int PlcCommunication::readRegisterBit(int addr) {
    if (!modbusTcp_ || !connectStatus_) return -1;
    uint8_t reply[8] = {0};
    int ret = modbus_read_bits(modbusTcp_, addr, 8, reply);
    if (ret == -1) {
        modbus_close(modbusTcp_);
        tryToConnect();
        return -1;
    }
    return (int(reply[0]) == 0) ? 0 : 1;
}

int PlcCommunication::readRegisterInt32(int addr) {
    if (!modbusTcp_ || !connectStatus_) return -1;
    uint16_t reply[2] = {0};
    int ret = modbus_read_registers(modbusTcp_, addr, 2, reply);
    if (ret == -1) {
        modbus_close(modbusTcp_);
        tryToConnect();
        return -1;
    }
    return dataTransUInt16_Int32(reply);
}

float PlcCommunication::readRegisterFloat(int addr) {
    if (!modbusTcp_ || !connectStatus_) return 0.0f;
    uint16_t reply[2] = {0};
    int ret = modbus_read_registers(modbusTcp_, addr, 2, reply);
    if (ret == -1) {
        modbus_close(modbusTcp_);
        tryToConnect();
        return 0.0f;
    }
    return dataTransUInt16_Float(reply);
}

double PlcCommunication::readRegisterDouble(int addr) {
    if (!modbusTcp_ || !connectStatus_) return 0.0;
    uint16_t reply[4] = {0};
    int ret = modbus_read_registers(modbusTcp_, addr, 4, reply);
    if (ret == -1) {
        modbus_close(modbusTcp_);
        tryToConnect();
        return 0.0;
    }
    return dataTransUInt16_Double(reply);
}

int PlcCommunication::readRegistersInt32(int addr, int num, int *data) {
    if (!modbusTcp_ || !connectStatus_) return -1;
    uint16_t reply[125] = {0};
    num = (2 * num <= 125) ? num : 62;
    int ret = modbus_read_registers(modbusTcp_, addr, 2 * num, reply);
    if (ret == -1) {
        modbus_close(modbusTcp_);
        tryToConnect();
        return -1;
    }
    for (int i = 0; i < ret / 2; i++) {
        data[i] = static_cast<int>(dataTransUInt16_Int32(&reply[2 * i]));
    }
    return 0;
}

int PlcCommunication::writeRegisterBit(int addr, int status) {
    if (!modbusTcp_ || !connectStatus_) return -1;
    int ret = modbus_write_bit(modbusTcp_, addr, status);
    if (ret == -1) {
        modbus_close(modbusTcp_);
        tryToConnect();
        return -1;
    }
    return 0;
}

int PlcCommunication::writeRegisters(int addr, int num, uint16_t *data) {
    if (!modbusTcp_ || !connectStatus_) return -1;
    num = (num <= 123) ? num : 123;
    int ret = modbus_write_registers(modbusTcp_, addr, num, data);
    if (ret == -1) {
        modbus_close(modbusTcp_);
        tryToConnect();
        return -1;
    }
    return 0;
}

// ============================================================
//  数据转换（从 realplc 旧代码迁移）
// ============================================================

int16_t PlcCommunication::dataTransUInt16_Int16(uint16_t *data) { return static_cast<int16_t>(*data); }

uint16_t PlcCommunication::dataTransUInt16_UInt16(uint16_t *data) { return static_cast<uint16_t>(*data); }

int32_t PlcCommunication::dataTransUInt16_Int32(uint16_t *data) {
    uint32_t combined = (static_cast<uint32_t>(data[1]) << 16) | static_cast<uint32_t>(data[0]);
    return static_cast<int32_t>(combined);
}

uint32_t PlcCommunication::dataTransUInt16_UInt32(uint16_t *data) {
    uint32_t combined = (static_cast<uint32_t>(data[1]) << 16) | static_cast<uint32_t>(data[0]);
    return static_cast<uint32_t>(combined);
}

float PlcCommunication::dataTransUInt16_Float(uint16_t *data) {
    uint32_t combined = (static_cast<uint32_t>(data[1]) << 16) | static_cast<uint32_t>(data[0]);
    float result;
    std::memcpy(&result, &combined, sizeof(float));
    return result;
}

double PlcCommunication::dataTransUInt16_Double(uint16_t *data) {
    uint64_t combined = (static_cast<uint64_t>(data[3]) << 48) | (static_cast<uint64_t>(data[2]) << 32) | (static_cast<uint64_t>(data[1]) << 16) |
                        static_cast<uint64_t>(data[0]);
    double result;
    std::memcpy(&result, &combined, sizeof(result));
    return result;
}

void PlcCommunication::dataTransInt16_UInt16(int16_t dataIn, uint16_t *dataOut) { *dataOut = static_cast<uint16_t>(dataIn); }

void PlcCommunication::dataTransUInt16_UInt16(uint16_t dataIn, uint16_t *dataOut) { *dataOut = static_cast<uint16_t>(dataIn); }

void PlcCommunication::dataTransInt32_UInt16(int32_t dataIn, uint16_t *dataOut) {
    dataOut[0] = static_cast<uint16_t>(dataIn & 0xFFFF);
    dataOut[1] = static_cast<uint16_t>((dataIn >> 16) & 0xFFFF);
}

void PlcCommunication::dataTransUInt32_UInt16(uint32_t dataIn, uint16_t *dataOut) {
    dataOut[0] = static_cast<uint16_t>(dataIn & 0xFFFF);
    dataOut[1] = static_cast<uint16_t>((dataIn >> 16) & 0xFFFF);
}

void PlcCommunication::dataTransFloat_UInt16(float dataIn, uint16_t *dataOut) {
    uint32_t value;
    std::memcpy(&value, &dataIn, sizeof(float));
    dataOut[0] = static_cast<uint16_t>(value & 0xFFFF);
    dataOut[1] = static_cast<uint16_t>((value >> 16) & 0xFFFF);
}

void PlcCommunication::dataTransDouble_UInt16(double dataIn, uint16_t *dataOut) {
    uint64_t value;
    std::memcpy(&value, &dataIn, sizeof(double));
    dataOut[0] = static_cast<uint16_t>(value & 0xFFFF);
    dataOut[1] = static_cast<uint16_t>((value >> 16) & 0xFFFF);
    dataOut[2] = static_cast<uint16_t>((value >> 32) & 0xFFFF);
    dataOut[3] = static_cast<uint16_t>((value >> 48) & 0xFFFF);
}

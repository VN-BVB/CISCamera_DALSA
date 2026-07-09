#pragma once

#include <stdint.h>

#include "modbus-tcp.h"
#include "modbus.h"

struct plcCtrlParams;
struct plcFdbkParams;

// ============================================================
//  PlcCommunication — 纯 C++ 类（非 QObject）
//  负责 modbus TCP 读写，不包含线程/信号
// ============================================================
class PlcCommunication {
public:
    PlcCommunication(plcCtrlParams *ctrl = nullptr);
    ~PlcCommunication();

    // ---- 连接 ----
    bool plcConnect();
    bool plcDisconnect();
    bool plcReset();

    // ======================== 地轨 ========================
    bool railEnable();
    bool railDisable();
    bool railAbsMove(double pos, double vel, double acc, double jerk);
    bool railHome();
    bool railStop(double dec = 0, double jerk = 0);
    bool railReset();

    // ======================== 对位平台总控 ========================
    // pltIdx: 0~6
    bool pltEnable(int pltIdx);
    bool pltDisable(int pltIdx);
    bool pltHome(int pltIdx);
    bool pltLocate(int pltIdx, double x, double y, double r,
                   double vel, double acc, double jerk);
    bool pltStop(int pltIdx);
    bool pltReset(int pltIdx);

    // ======================== 单轴控制 ========================
    // axis: 0=X, 1=Y, 2=R
    bool axisMoveR(int pltIdx, int axis, double pos, double vel,
                   double acc, double jerk);
    bool axisStop(int pltIdx, int axis);
    bool axisReset(int pltIdx, int axis);

    // ======================== 批量控制 ========================
    bool pltEnableAll();
    bool pltHomeAll();
    bool pltResetAll();
    bool pltStopAll();

    // ======================== 状态反馈 ========================
    bool plcStateFdbk(plcFdbkParams &data);

    // ======================== 连接状态 ========================
    bool isConnected() const { return connectStatus_; }

    // ---- HD 寄存器位操作（读-改-写，位0=低位，位8=高位）----
    int writeHdBit(int hdReg, int bitIndex, bool val);
    int writeHdLowBit(int hdReg, bool val);
    int writeHdHighBit(int hdReg, bool val);
    int readHdBit(int hdReg, int bitIndex);
    int readHdLowBit(int hdReg);
    int readHdHighBit(int hdReg);

private:
    void tryToConnect();

    // ---- Modbus 读 ----
    int    readRegisterBit(int addr);
    int    readRegisterInt32(int addr);
    float  readRegisterFloat(int addr);
    double readRegisterDouble(int addr);
    int    readRegistersInt32(int addr, int num, int *data);

    // ---- Modbus 写 ----
    int writeRegisterBit(int addr, int status);
    int writeRegisters(int addr, int num, uint16_t *data);
    int writeHighLowBit(int addrHigh, int addrLow,
                        bool high, bool low);

    // ---- 数据转换 ----
    // 读方向
    int16_t  dataTransUInt16_Int16(uint16_t *data);
    uint16_t dataTransUInt16_UInt16(uint16_t *data);
    int32_t  dataTransUInt16_Int32(uint16_t *data);
    uint32_t dataTransUInt16_UInt32(uint16_t *data);
    float    dataTransUInt16_Float(uint16_t *data);
    double   dataTransUInt16_Double(uint16_t *data);

    // 写方向
    void dataTransInt16_UInt16(int16_t dataIn, uint16_t *dataOut);
    void dataTransUInt16_UInt16(uint16_t dataIn, uint16_t *dataOut);
    void dataTransInt32_UInt16(int32_t dataIn, uint16_t *dataOut);
    void dataTransUInt32_UInt16(uint32_t dataIn, uint16_t *dataOut);
    void dataTransFloat_UInt16(float dataIn, uint16_t *dataOut);
    void dataTransDouble_UInt16(double dataIn, uint16_t *dataOut);

    // ---- 成员 ----
    plcCtrlParams *params_  = nullptr;
    modbus_t      *modbusTcp_ = nullptr;
    bool           connectStatus_ = false;
};

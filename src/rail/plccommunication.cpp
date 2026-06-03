#include "plccommunication.h"

#include "plc_addr.h"
#include "plccommon.h"

using namespace plc;

PlcCommunication::PlcCommunication(plcCtrlParams *ctrl) : params_(ctrl) {
    if (!params_) return;

    // 创建modbus_tcp对象
    modbusTcp = modbus_new_tcp(params_->plc.ip.c_str(), params_->plc.port);
    if (!modbusTcp) return;

    // 设置响应超时时间，参数分别为modbus_tcp对象、秒、毫秒
    modbus_set_response_timeout(modbusTcp, 0, 200 * 1000);
}

PlcCommunication::~PlcCommunication() {
    // 断开连接
    if (modbusTcp != nullptr) {
        modbus_close(modbusTcp);
        modbus_free(modbusTcp);
        modbusTcp = nullptr;
        connectStatus = false;
    }
}

bool PlcCommunication::plcConnect() {
    if (!modbusTcp) return false;

    if (connectStatus) return true;

    // 建立连接
    if (modbus_connect(modbusTcp) == -1) {
        connectStatus = false;
        return false;
    } else {
        connectStatus = true;
        return true;
    }
}

bool PlcCommunication::plcDisconnect() {
    if (!modbusTcp) return true;

    if (!connectStatus) return true;

    // 断开连接
    if (modbusTcp != nullptr) {
        modbus_close(modbusTcp);
        connectStatus = false;
    }
    return true;
}

bool PlcCommunication::plcReset() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 清除工件位置数据和复位Y输出
        if (writeRegisterBit(MAddr + M_Init, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcConfig() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        uint16_t data[10] = {0};
        dataTransInt32_UInt16(params_->plc.pulseCntMax, &data[0]);  // HD0  高速AB相计数器HSC0的比较值
        dataTransInt32_UInt16(params_->plc.pulseDiv, &data[2]);     // HD2   高速AB相计数器HSC0的中断设定值
        dataTransFloat_UInt16(0, &data[4]);                         // HD4  预留
        dataTransInt32_UInt16(params_->mscNumMax, &data[6]);        // HD6  多工位控制——最大可处理的工件数
        dataTransInt32_UInt16(0, &data[8]);                         // HD4  预留
        // 参数写入
        if (writeRegisters(HDAddr + HD_CntabMax, 10, data) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcCaptureOpen() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        if (writeRegisterBit(MAddr + M_CaptureImg, 1) != 0) return false;
        if (writeRegisterBit(MAddr + M_CapturePos, 1) != 0) return false;
        return true;
    }
    return false;
}

bool PlcCommunication::plcCaptureClose() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        if (writeRegisterBit(MAddr + M_CaptureImg, 0) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcPositionCap() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        if (writeRegisterBit(MAddr + M_WorkPiecePos, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcConveyor1Enable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 输送链控制使能
        if (writeRegisterBit(MAddr + M_ConveyEnable, 1) != 0) {
            conveyEnable = false;
            return false;
        } else {
            conveyEnable = true;
            return true;
        }
    }
    return false;
}

bool PlcCommunication::plcConveyor1Disable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 输送链控制去使能
        if (writeRegisterBit(MAddr + M_ConveyEnable, 0) != 0)
            return false;
        else {
            conveyEnable = false;
            return true;
        }
    }
    return false;
}

bool PlcCommunication::plcConveyor1Fwd(double vel, double acc, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double vel_max = params_->plc.conveyor1.speedMax;
        double acc_max = params_->plc.conveyor1.accMax;
        double jerk_max = params_->plc.conveyor1.jerkMax;
        if (std::abs(vel) > vel_max || acc > acc_max || jerk > jerk_max) return false;

        // 运动参数
        uint16_t data[19] = {0};
        dataTransDouble_UInt16(vel, &data[0]);    // 目标速度
        dataTransDouble_UInt16(acc, &data[4]);    // 目标加速度
        dataTransDouble_UInt16(acc, &data[8]);    // 目标减速度
        dataTransDouble_UInt16(jerk, &data[12]);  // 目标加加速度
        dataTransUInt16_UInt16(0, &data[16]);     // 持续更新
        dataTransUInt16_UInt16(0, &data[17]);     // 方向
        dataTransUInt16_UInt16(0, &data[18]);     // 缓存模式(中断模式)

        // 参数写入
        if (writeRegisters(DAddr + D_Convey1FwdParams, 19, data) != 0) return false;

        // 开始运动
        if (writeRegisterBit(MAddr + M_Convey1Forward, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcConveyor1Rev(double vel, double acc, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double vel_max = params_->plc.conveyor1.speedMax;
        double acc_max = params_->plc.conveyor1.accMax;
        double jerk_max = params_->plc.conveyor1.jerkMax;
        if (std::abs(vel) > vel_max || acc > acc_max || jerk > jerk_max) return false;
        // 运动参数
        uint16_t data[19] = {0};
        dataTransDouble_UInt16(-1.0 * vel, &data[0]);  // 目标速度
        dataTransDouble_UInt16(acc, &data[4]);         // 目标加速度
        dataTransDouble_UInt16(acc, &data[8]);         // 目标减速度
        dataTransDouble_UInt16(jerk, &data[12]);       // 目标加加速度
        dataTransUInt16_UInt16(0, &data[16]);          // 持续更新
        dataTransUInt16_UInt16(0, &data[17]);          // 方向
        dataTransUInt16_UInt16(0, &data[18]);          // 缓存模式(中断模式)
        // 参数写入
        if (writeRegisters(DAddr + D_Convey1RevParams, 19, data) != 0) return false;

        // 开始运动
        if (writeRegisterBit(MAddr + M_Convey1Reverse, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcConveyor1Stop(double acc, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double acc_max = params_->plc.conveyor1.accMax;
        double jerk_max = params_->plc.conveyor1.jerkMax;
        if (acc > acc_max || jerk > jerk_max) return false;
        // 运动参数
        uint16_t data[9] = {0};
        dataTransDouble_UInt16(acc, &data[0]);   // 减速度
        dataTransDouble_UInt16(jerk, &data[4]);  // 加加速度
        dataTransUInt16_UInt16(0, &data[8]);     // 停止类型
        // 写入参数
        if (writeRegisters(DAddr + D_Convey1StopParams, 9, data) != 0) return false;

        // 停止运动
        if (writeRegisterBit(MAddr + M_Convey1Stop, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcConveyor2Enable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 输送链控制使能
        if (writeRegisterBit(MAddr + M_ConveyEnable, 1) != 0) {
            conveyEnable = false;
            return false;
        } else {
            conveyEnable = true;
            return true;
        }
    }
    return false;
}

bool PlcCommunication::plcConveyor2Disable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 输送链控制去使能
        if (writeRegisterBit(MAddr + M_ConveyEnable, 0) != 0)
            return false;
        else {
            conveyEnable = false;
            return true;
        }
    }
    return false;
}

bool PlcCommunication::plcConveyor2Fwd(double vel, double acc, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double vel_max = params_->plc.conveyor2.speedMax;
        double acc_max = params_->plc.conveyor2.accMax;
        double jerk_max = params_->plc.conveyor2.jerkMax;
        if (vel > vel_max || acc > acc_max || jerk > jerk_max) return false;
        // 运动参数
        uint16_t data[2] = {0};
        dataTransFloat_UInt16(vel, &data[0]);  // 目标速度
        // 参数写入
        if (writeRegisters(DAddr + D_Convey2VelParams, 2, data) != 0) return false;

        // 开始运动
        if (writeRegisterBit(MAddr + M_Convey2Forward, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcConveyor2Rev(double vel, double acc, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double vel_max = params_->plc.conveyor2.speedMax;
        double acc_max = params_->plc.conveyor2.accMax;
        double jerk_max = params_->plc.conveyor2.jerkMax;
        if (vel > vel_max || acc > acc_max || jerk > jerk_max) return false;
        // 运动参数
        uint16_t data[2] = {0};
        dataTransFloat_UInt16(vel, &data[0]);  // 目标速度
        // 参数写入
        if (writeRegisters(DAddr + D_Convey2VelParams, 2, data) != 0) return false;

        // 开始运动
        if (writeRegisterBit(MAddr + M_Convey2Reverse, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcConveyor2Stop(double acc, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double acc_max = params_->plc.conveyor2.accMax;
        double jerk_max = params_->plc.conveyor2.jerkMax;
        if (acc > acc_max || jerk > jerk_max) return false;
        // 开始运动
        if (writeRegisterBit(MAddr + M_Convey2Stop, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis1Enable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 外部轴1伺服使能
        if (writeRegisterBit(MAddr + M_ExAxis1Enable, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis1Disable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 外部轴1伺服去使能
        if (writeRegisterBit(MAddr + M_ExAxis1Enable, 0) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis1ZRN() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        if (writeRegisterBit(MAddr + M_ExAxis1ZRN, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis1Move(double pos, double vel, double acc, double jerk) {
    if (!modbusTcp) return false;

    std::cout << "axis CMD" << pos << " " << vel << " " << acc << std::endl;

    if (connectStatus) {
        // 运动范围限位
        double limit_min = params_->plc.exAxis1.minMotionRange;
        double limit_max = params_->plc.exAxis1.maxMotionRange;
        if (pos > limit_max + 1e-6 || pos < limit_min - 1e-6) return false;
        // 运动参数限位
        double vel_max = params_->plc.exAxis1.speedMax;
        double acc_max = params_->plc.exAxis1.accMax;
        double jerk_max = params_->plc.exAxis1.jerkMax;
        if (vel > vel_max + 1e-6 || acc > acc_max + 1e-6 || jerk > jerk_max + 1e-6) return false;
        // 运动参数
        uint16_t data[23] = {0};
        dataTransDouble_UInt16(pos, &data[0]);    // 目标位置
        dataTransDouble_UInt16(vel, &data[4]);    // 目标速度
        dataTransDouble_UInt16(acc, &data[8]);    // 目标加速度
        dataTransDouble_UInt16(acc, &data[12]);   // 目标减速度
        dataTransDouble_UInt16(jerk, &data[16]);  // 目标加加速度
        dataTransUInt16_UInt16(0, &data[20]);     // 持续更新
        dataTransUInt16_UInt16(0, &data[21]);     // 方向
        dataTransUInt16_UInt16(0, &data[22]);     // 中断模式

        // 写入运行参数
        if (writeRegisters(DAddr + D_ExAxis1MOVEAParams, 23, data) != 0) return false;

        // 开始运动
        if (writeRegisterBit(MAddr + M_ExAxis1MOVEA, 1) != 0)
            return false;
        else
            return true;
    }
    return true;
}

bool PlcCommunication::plcExAxis1Follow() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 同步运动
        if (writeRegisterBit(MAddr + M_ExAxis1FOLOOW, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis1Halt(double dec, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double acc_max = params_->plc.exAxis1.accMax;
        double jerk_max = params_->plc.exAxis1.jerkMax;
        //        if (dec > acc_max || jerk > jerk_max)
        //            return false;
        // 运动参数
        uint16_t data[9] = {0};
        dataTransDouble_UInt16(dec, &data[0]);   // 减速度
        dataTransDouble_UInt16(jerk, &data[4]);  // 加加速度
        dataTransUInt16_UInt16(0, &data[8]);     // 停止类型
        // 写入运行参数
        if (writeRegisters(DAddr + D_ExAxis1HaltParams, 9, data) != 0) return false;

        // 停止运动
        if (writeRegisterBit(MAddr + M_ExAxis1Halt, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis1Stop(double dec, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double acc_max = params_->plc.exAxis1.accMax;
        double jerk_max = params_->plc.exAxis1.jerkMax;
        //        if (dec > acc_max || jerk > jerk_max)wjq
        //        {
        //            std::cout << ">>> [PLC Error] 外部轴1停止失败！原因：减速度或加加速度超出了系统设定的最大限制！" << std::endl;
        //            return false;
        //        }
        //            return false;
        // 运动参数
        uint16_t data[9] = {0};
        dataTransDouble_UInt16(dec, &data[0]);   // 减速度
        dataTransDouble_UInt16(jerk, &data[4]);  // 加加速度
        dataTransUInt16_UInt16(0, &data[8]);     // 停止类型
        // 写入运行参数
        if (writeRegisters(DAddr + D_ExAxis1StopParams, 9, data) != 0) return false;

        // 停止运动
        if (writeRegisterBit(MAddr + M_ExAxis1Stop, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis1Reset() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        if (writeRegisterBit(MAddr + M_ExAxis1Rst, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis2Enable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 外部轴2伺服使能
        if (writeRegisterBit(MAddr + M_ExAxis2Enable, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis2Disable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 外部轴2伺服去使能
        if (writeRegisterBit(MAddr + M_ExAxis2Enable, 0) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis2ZRN() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        if (writeRegisterBit(MAddr + M_ExAxis2ZRN, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis2Move(double pos, double vel, double acc, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动范围限位
        double limit_min = params_->plc.exAxis2.minMotionRange;
        double limit_max = params_->plc.exAxis2.maxMotionRange;
        if (pos > limit_max || pos < limit_min) return false;
        // 运动参数限位
        double vel_max = params_->plc.exAxis2.speedMax;
        double acc_max = params_->plc.exAxis2.accMax;
        double jerk_max = params_->plc.exAxis2.jerkMax;
        if (vel > vel_max || acc > acc_max || jerk > jerk_max) return false;
        // 运动参数
        uint16_t data[23] = {0};
        dataTransDouble_UInt16(pos, &data[0]);    // 目标位置
        dataTransDouble_UInt16(vel, &data[4]);    // 目标速度
        dataTransDouble_UInt16(acc, &data[8]);    // 目标加速度
        dataTransDouble_UInt16(acc, &data[12]);   // 目标减速度
        dataTransDouble_UInt16(jerk, &data[16]);  // 目标加加速度
        dataTransUInt16_UInt16(0, &data[20]);     // 持续更新
        dataTransUInt16_UInt16(0, &data[21]);     // 方向
        dataTransUInt16_UInt16(0, &data[22]);     // 缓存模式
        // 写入运行参数
        if (writeRegisters(DAddr + D_ExAxis2MOVEAParams, 23, data) != 0) return false;

        // 开始运动
        if (writeRegisterBit(MAddr + M_ExAxis2MOVEA, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis2Follow() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 同步运动
        if (writeRegisterBit(MAddr + M_ExAxis2FOLOOW, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis2Halt(double dec, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double acc_max = params_->plc.exAxis2.accMax;
        double jerk_max = params_->plc.exAxis2.jerkMax;
        if (dec > acc_max || jerk > jerk_max) return false;
        // 运动参数
        uint16_t data[9] = {0};
        dataTransDouble_UInt16(dec, &data[0]);   // 减速度
        dataTransDouble_UInt16(jerk, &data[4]);  // 加加速度
        dataTransUInt16_UInt16(0, &data[8]);     // 停止类型
        // 写入参数
        if (writeRegisters(DAddr + D_ExAxis2HaltParams, 9, data) != 0) return false;

        // 停止运动
        if (writeRegisterBit(MAddr + M_ExAxis2Halt, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis2Stop(double dec, double jerk) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 运动参数限位
        double acc_max = params_->plc.exAxis2.accMax;
        double jerk_max = params_->plc.exAxis2.jerkMax;
        if (dec > acc_max || jerk > jerk_max) return false;
        // 运动参数
        uint16_t data[9] = {0};
        dataTransDouble_UInt16(dec, &data[0]);   // 减速度
        dataTransDouble_UInt16(jerk, &data[4]);  // 加加速度
        dataTransUInt16_UInt16(0, &data[8]);     // 停止类型
        // 写入参数
        if (writeRegisters(DAddr + D_ExAxis2StopParams, 9, data) != 0) return false;

        // 停止运动
        if (writeRegisterBit(MAddr + M_ExAxis2Stop, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcExAxis2Reset() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        if (writeRegisterBit(MAddr + M_ExAxis2Rst, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcSprayGun1Enable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 开启喷枪
        if (writeRegisterBit(MAddr + M_SprayGun1CYL, 1) != 0) return false;
        // 开启雾化
        if (writeRegisterBit(MAddr + M_SprayGun1CAP_FAN, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcSprayGun1Disable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 关闭雾化
        if (writeRegisterBit(MAddr + M_SprayGun1CAP_FAN, 0) != 0) return false;
        // 关闭喷枪
        if (writeRegisterBit(MAddr + M_SprayGun1CYL, 0) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcGun1ChangPaintEnable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 开启喷枪
        if (writeRegisterBit(MAddr + M_SprayGun1CYL, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcGun1ChangPaintDisable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 关闭喷枪
        if (writeRegisterBit(MAddr + M_SprayGun1CYL, 0) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcSprayGun2Enable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 开启喷枪
        if (writeRegisterBit(MAddr + M_SprayGun2CYL, 1) != 0) return false;
        // 开启雾化
        if (writeRegisterBit(MAddr + M_SprayGun2CAP_FAN, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcSprayGun2Disable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 关闭雾化
        if (writeRegisterBit(MAddr + M_SprayGun2CAP_FAN, 0) != 0) return false;
        // 关闭喷枪
        if (writeRegisterBit(MAddr + M_SprayGun2CYL, 0) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcGun2ChangPaintEnable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 开启喷枪
        if (writeRegisterBit(MAddr + M_SprayGun2CYL, 1) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcGun2ChangPaintDisable() {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // 关闭喷枪
        if (writeRegisterBit(MAddr + M_SprayGun2CYL, 0) != 0)
            return false;
        else
            return true;
    }
    return false;
}

bool PlcCommunication::plcStateFdbk(plcFdbkParams &data) {
    if (!modbusTcp) return false;

    if (connectStatus) {
        // PLC连接状态
        data.connectStatus = connectStatus ? 1 : 0;
        // PLC运行状态
        data.runStatus = readRegisterBit(SMAddr + SM_Run);
        // 输送链编码器计数值
        data.encoderCnt = readRegisterInt32(HSCDAddr + HSCD_CntAB);
        // 采图区域光幕信号
        data.captureTrigger = readRegisterBit(MAddr + M_LightSensor1);
        // 喷涂区域光幕信号
        data.sprayTrigger = readRegisterBit(MAddr + M_LightSensor2);
        // 采图状态
        data.captureStatus = readRegisterBit(MAddr + M_CaptureImg);
        // 触发光幕1后的移动距离
        data.captureDis = params_->pulseDis * readRegisterInt32(DAddr + D_CapPlusNum);
        // 触发光幕1后测量工件长度
        if (data.sprayTrigger == 0)
            data.captureLength = params_->pulseDis * readRegisterInt32(DAddr + D_CapLen);
        else
            data.captureLength = 0.0;
        // 已扫描的工件数
        data.workpieceNum = readRegisterInt32(HDAddr + HD_PlusNum);
        // 每个工件的输送链位置
        int orgdataPlus[64] = {};
        readRegistersInt32(HDAddr + HD_PlusNum1, params_->mscNumMax, orgdataPlus);
        data.workpiecePos.resize(params_->mscNumMax);
        for (int i = 0; i < params_->mscNumMax; i++) data.workpiecePos[i] = params_->pulseDis * double(orgdataPlus[i]);
        // 输送链使能状态
        data.conveyorEnale = conveyEnable ? 1 : 0;
        // 输送链速度
        data.conveyorVel = (1000.0 / 80.0) * params_->pulseDis * readRegisterInt32(DAddr + D_ConveyVel);
        // 外部轴1使能状态
        data.exAxis1Enable = readRegisterBit(MAddr + M_ExAxis1Enable);
        // 外部轴1绝对位置
        data.exAxis1Pos = readRegisterDouble(DAddr + D_ExAxis1Pos);
        // 外部轴1运动速度
        data.exAxis1Vel = readRegisterDouble(DAddr + D_ExAxis1Vel);
        // 外部轴2使能状态
        data.exAxis2Enable = readRegisterBit(MAddr + M_ExAxis2Enable);
        // 外部轴2绝对位置
        data.exAxis2Pos = readRegisterDouble(DAddr + D_ExAxis2Pos);
        // 外部轴2运动速度
        data.exAxis2Vel = readRegisterDouble(DAddr + D_ExAxis2Vel);
        // 输出结果
        return true;
    }
    return false;
}

void PlcCommunication::tryToConnect() {
    if (!modbusTcp) return;

    connectStatus = false;
    if (modbus_connect(modbusTcp) == -1)
        return;
    else
        connectStatus = true;
}

int PlcCommunication::readRegisterBit(int addr) {
    uint8_t reply[8] = {0};

    int ret = modbus_read_bits(modbusTcp, addr, 8, reply);
    if (ret == -1) {
        modbus_close(modbusTcp);
        tryToConnect();
        return -1;
    }

    if (int(reply[0]) == 0)
        return 0;
    else
        return 1;
}

int PlcCommunication::readRegisterInt32(int addr) {
    uint16_t reply[2] = {0};

    int ret = modbus_read_registers(modbusTcp, addr, 2, reply);
    if (ret == -1) {
        modbus_close(modbusTcp);
        tryToConnect();
        return -1;
    }

    return dataTransUInt16_Int32(reply);
}

float PlcCommunication::readRegisterFloat(int addr) {
    uint16_t reply[2] = {0};

    int ret = modbus_read_registers(modbusTcp, addr, 2, reply);
    if (ret == -1) {
        modbus_close(modbusTcp);
        tryToConnect();
        return 0.0f;
    }

    return dataTransUInt16_Float(reply);
}

double PlcCommunication::readRegisterDouble(int addr) {
    uint16_t reply[4] = {0};

    int ret = modbus_read_registers(modbusTcp, addr, 4, reply);
    if (ret == -1) {
        modbus_close(modbusTcp);
        tryToConnect();
        return 0.0f;
    }

    return dataTransUInt16_Double(reply);
}

int PlcCommunication::readRegistersInt32(int addr, int num, int *data) {
    uint16_t reply[125] = {0};  // Modbus报文最长读取125个寄存器

    num = (2 * num <= 125) ? num : 62;
    int ret = modbus_read_registers(modbusTcp, addr, 2 * num, reply);
    if (ret == -1) {
        modbus_close(modbusTcp);
        tryToConnect();
        return -1;
    }

    for (int i = 0; i < ret / 2; i++) {
        int32_t int32_value = dataTransUInt16_Int32(&reply[2 * i]);
        data[i] = static_cast<int>(int32_value);
    }

    return 0;
}

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
    std::memcpy(&result, &combined, sizeof(float));  // 复制二进制数据到 float
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
    dataOut[0] = static_cast<uint16_t>(dataIn & 0xFFFF);          // 低16位
    dataOut[1] = static_cast<uint16_t>((dataIn >> 16) & 0xFFFF);  // 高16位
}

void PlcCommunication::dataTransUInt32_UInt16(uint32_t dataIn, uint16_t *dataOut) {
    dataOut[0] = static_cast<uint16_t>(dataIn & 0xFFFF);          // 低16位
    dataOut[1] = static_cast<uint16_t>((dataIn >> 16) & 0xFFFF);  // 高16位
}

void PlcCommunication::dataTransFloat_UInt16(float dataIn, uint16_t *dataOut) {
    uint32_t value;
    std::memcpy(&value, &dataIn, sizeof(float));  // 复制 float 的二进制数据

    // 小端模式拆分为 2 个 uint16_t
    dataOut[0] = static_cast<uint16_t>(value & 0xFFFF);          // 低16位
    dataOut[1] = static_cast<uint16_t>((value >> 16) & 0xFFFF);  // 高16位
}

void PlcCommunication::dataTransDouble_UInt16(double dataIn, uint16_t *dataOut) {
    uint64_t value;
    std::memcpy(&value, &dataIn, sizeof(double));  // 将 double 按位复制到 uint64_t

    dataOut[0] = static_cast<uint16_t>(value & 0xFFFF);
    dataOut[1] = static_cast<uint16_t>((value >> 16) & 0xFFFF);
    dataOut[2] = static_cast<uint16_t>((value >> 32) & 0xFFFF);
    dataOut[3] = static_cast<uint16_t>((value >> 48) & 0xFFFF);
}

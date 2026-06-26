#pragma once

#include "modbus.h"
#include "modbus-tcp.h"

struct plcCtrlParams;
struct plcFdbkParams;

class PlcCommunication
{
public:
    PlcCommunication(plcCtrlParams *ctrl = nullptr);

    ~PlcCommunication();

public:
    // PLC连接
    bool plcConnect();
    // PLC断开连接
    bool plcDisconnect();
    // PLC系统复位
    bool plcReset();
    // PLC配置参数写入
    bool plcConfig();
    // 开启采图
    bool plcCaptureOpen();
    // 关闭采图
    bool plcCaptureClose();
    // 工件位置捕获
    bool plcPositionCap();
    // 输送链1使能
    bool plcConveyor1Enable();
    // 输送链1下使能
    bool plcConveyor1Disable();
    // 输送链1正转
    bool plcConveyor1Fwd(double vel = 0.0, double acc = 0.0, double jerk = 0.0);
    // 输送链1反转
    bool plcConveyor1Rev(double vel = 0.0, double acc = 0.0, double jerk = 0.0);
    // 输送链1停转
    bool plcConveyor1Stop(double acc = 0.0, double jerk = 0.0);
    // 输送链2使能
    bool plcConveyor2Enable();
    // 输送链2下使能
    bool plcConveyor2Disable();
    // 输送链2正转
    bool plcConveyor2Fwd(double vel = 0.0, double acc = 0.0, double jerk = 0.0);
    // 输送链2反转
    bool plcConveyor2Rev(double vel = 0.0, double acc = 0.0, double jerk = 0.0);
    // 输送链2停转
    bool plcConveyor2Stop(double acc = 0.0, double jerk = 0.0);
    // 外部轴1伺服使能
    bool plcExAxis1Enable();
    // 外部轴1伺服下使能
    bool plcExAxis1Disable();
    // 外部轴1机械归零运动
    bool plcExAxis1ZRN();
    // 外部轴1绝对位置运动
    bool plcExAxis1Move(double pos = 0.0, double vel = 0.0, double acc = 0.0, double jerk = 0.0);
    // 外部轴1输送链跟踪运动
    bool plcExAxis1Follow();
    // 外部轴1暂停
    bool plcExAxis1Halt(double dec = 0.0, double jerk = 0.0);
    // 外部轴1停转
    bool plcExAxis1Stop(double dec = 0.0, double jerk = 0.0);
    // 外部轴1复位
    bool plcExAxis1Reset();
    // 外部轴2伺服使能
    bool plcExAxis2Enable();
    // 外部轴2伺服下使能
    bool plcExAxis2Disable();
    // 外部轴2机械归零运动
    bool plcExAxis2ZRN();
    // 外部轴绝对位置运动
    bool plcExAxis2Move(double pos = 0.0, double vel = 0.0, double acc = 0.0, double jerk = 0.0);
    // 外部轴2输送链跟踪运动
    bool plcExAxis2Follow();
    // 外部轴2暂停
    bool plcExAxis2Halt(double dec = 0.0, double jerk = 0.0);
    // 外部轴2停转
    bool plcExAxis2Stop(double dec = 0.0, double jerk = 0.0);
    // 外部轴2复位
    bool plcExAxis2Reset();
    // 喷枪1控制开启
    bool plcSprayGun1Enable();
    // 喷枪1控制关闭
    bool plcSprayGun1Disable();
    // 喷枪1换料开启
    bool plcGun1ChangPaintEnable();
    // 喷枪1换料关闭
    bool plcGun1ChangPaintDisable();
    // 喷枪2控制开启
    bool plcSprayGun2Enable();
    // 喷枪2控制关闭
    bool plcSprayGun2Disable();
    // 喷枪2换料开启
    bool plcGun2ChangPaintEnable();
    // 喷枪2换料开启
    bool plcGun2ChangPaintDisable();
    // PLC设备状态反馈
    bool plcStateFdbk(plcFdbkParams &data);

private:
    void tryToConnect(); // 重连

    /************************************读数据***********************************************/
    int readRegisterBit(int addr);                        // 读位
    int readRegisterInt32(int addr);                      // 读32位整型
    float readRegisterFloat(int addr);                    // 读单精度浮点数
    double readRegisterDouble(int addr);                  // 读双精度浮点数
    int readRegistersInt32(int addr, int num, int *data); // 批量读32位整型

    int16_t dataTransUInt16_Int16(uint16_t *data);   // 1位16无符号整数转16位整型
    uint16_t dataTransUInt16_UInt16(uint16_t *data); // 1位16无符号整数转16位无符号整型
    int32_t dataTransUInt16_Int32(uint16_t *data);   // 2位16无符号整数转32位整型
    uint32_t dataTransUInt16_UInt32(uint16_t *data); // 2位16无符号整数转32位无符号整型
    float dataTransUInt16_Float(uint16_t *data);     // 2位16无符号整数转32位浮点数
    double dataTransUInt16_Double(uint16_t *data);   // 4位16无符号整数转64位浮点数

    /************************************写数据***********************************************/
    int writeRegisterBit(int addr, int status);            // 写位
    int writeRegisters(int addr, int num, uint16_t *data); // 批量写寄存器

    void dataTransInt16_UInt16(int16_t dataIn, uint16_t *dataOut);   // 16位整型转1位16无符号整数
    void dataTransUInt16_UInt16(uint16_t dataIn, uint16_t *dataOut); // 16位无符号整型转1位16无符号整数
    void dataTransInt32_UInt16(int32_t dataIn, uint16_t *dataOut);   // 32位整型转2位16无符号整数
    void dataTransUInt32_UInt16(uint32_t dataIn, uint16_t *dataOut); // 32位无符号整型转2位16无符号整数
    void dataTransFloat_UInt16(float dataIn, uint16_t *dataOut);     // 32位浮点数转2位16无符号整数
    void dataTransDouble_UInt16(double dataIn, uint16_t *dataOut);   // 64位浮点数转4位16无符号整数

private:
    plcCtrlParams *params_ = nullptr;
    bool connectStatus = false;
    bool conveyEnable = false;
    modbus_t *modbusTcp = nullptr; // modbus指针
};

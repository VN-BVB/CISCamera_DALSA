#ifndef MOTION_CONTROLLER_H
#define MOTION_CONTROLLER_H

#include <QObject>
#include <QTimer>

#include "plccommunication.h"

// ============================================================
//  MotionController — QObject 桥接层
//  运行在独立线程，封装 PlcCommunication 的 modbus 读写
//  与 UI 层通过 Qt 信号/槽通信
// ============================================================
class MotionController : public QObject {
    Q_OBJECT
public:
    explicit MotionController(QObject *parent = nullptr);
    ~MotionController();

public slots:
    // ---- 连接 ----
    void connectPlc(const QString &ip, int port);
    void disconnectPlc();

    // ---- 地轨 ----
    void railAbsMove(double pos, double vel, double acc, double jerk);
    void railHome();
    void railStop();
    void railForward(double vel);
    void railReverse(double vel);
    void railReset();
    void railEnable();
    void railDisable();

    // ---- 对位平台 ----
    void pltHomeAll();
    void pltEnableAll();
    void pltResetAll();
    void pltLocateAll();
    // 单平台运动 (pltIdx 0~6)，读取 table_axispos 三轴位置 + 全局速度
    void pltSingleMove(int pltIdx, double x, double y, double r, double vel);
    // 单轴相对运动
    void axisSingleMoveR(int pltIdx, int axis, double pos, double vel);

    // ---- 轮询 ----
    void onStateTimeout();
    void onRealTimeout();

signals:
    // 地轨位置/速度更新
    void railPosVelUpdated(double pos, double vel);
    // 地轨绝对定位完成
    void railAbsFinished();
    // 状态文字（axis / motion）
    void stateTextUpdated(const QString &msgAxis, const QString &msgMotion);
    // 日志
    void logMessage(const QString &msg);
    // 连接状态灯
    void connectionStateChanged(const QString &color);

private:
    void setRailVelPosDouble(double pos, double vel, double acc, double jerk, int addr);
    void setRailVelDouble(double vel, int addr);
    void dataTransDouble_UInt16(double dataIn, uint16_t *dataOut);
    double dataTransUInt16_Double(uint16_t *data);
    void readModbusValue(int address, int num, bool isCoil);
    void readModbusValuePosVel(int addrPos, int addrVel);
    void sendRailState(int coilIndex);
    int writeRegistersRaw(int addr, int num, uint16_t *data);

    PlcCommunication *plc_ = nullptr;
    plcCtrlParams    *params_ = nullptr;
    QTimer           *stateTimer_ = nullptr;
    QTimer           *realTimer_  = nullptr;

    QString plcIp_;
    int    plcPort_ = 502;
    bool   connected_ = false;

    // 地轨运动状态
    bool   absMoveStarted_ = false;
    bool   absMoveDone_    = true;
    double targetAbsPos_   = 0.0;
    double absFinishTol_   = 0.5;

    QVector<bool> prevCoilStatuses_;
};
#endif  // MOTION_CONTROLLER_H

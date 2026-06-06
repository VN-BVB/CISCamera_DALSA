#ifndef PLC_CONTROL_H
#define PLC_CONTROL_H

#include <errno.h>
#include <plog/Log.h>

#include <QCoreApplication>
#include <QDebug>
#include <QNetworkProxy>
#include <QObject>
#include <QProcess>
#include <QThread>
#include <QTimer>

#include "axis_register.h"
#include "plc_variableaddress.h"
// libmodbus
#include "modbus-tcp.h"
#include "modbus.h"

class Rail : public QObject {
    Q_OBJECT
public:
    explicit Rail(QObject *parent = nullptr);
    ~Rail();

    void onStateTimeout();
    void onRealTimeout();
    void readModbusValue(int address, int num, bool isCoil);
    void readModbusValueXinjie(int addresspos, int addresvel);
    void sendRailState(int coilIndex);

    void waitForRegisterWriteComplete();
    void setRailVel(float v, int address);                                                   // 设置地轨速度
    void setRailVelPosDouble(double vel, double pos, double acc, double jerk, int address);  // 设置地轨速度、位置、加减速(double类型)
    void setRailVelDouble(double vel, int address);                                          // 设置地轨速度、加减速(double类型)
    void dataTransDouble_UInt16(double dataIn, uint16_t *dataOut);                           // 将double转换成4寄存器16位无符号整数
    double dataTransUInt16_Double(uint16_t *data);                                           // 4寄存器16无符号整数转64位浮点数
    QVector<quint16> whenFloat2Quint16(float value);                                         // 将float类型转换为两个quint16寄存器值

    bool mobusDisconnect;
    modbus_t *modbusTcp = nullptr;  // modbus指针
    QTimer *readStateTimer;
    QTimer *readRealTimer;

signals:
    void senderSignalWriteFinished();
    void sendText(QString message);
    void sendTextState(QString messageAxis, QString messageMotion);
    void modbusReadRealFinished(QVariant result);
    void sendPositionAndSpeed(double position, double speed);
    void sendSignalFinishWriteCoils();
    void sendSignalFinishWriteRegisters();
    void sendSignalDisconnected();
    void sendAbsFinished();

    void sendRailStatus(QString color);  // 地轨设备状态信号

public slots:
    void writeCoils(int address, const QVector<bool> &values);
    void writeRegisters(int address, const QVector<quint16> &values);
    int writeRegistersRaw(int addr, int num, uint16_t *data);  // 批量写寄存器
    void connectPLC(const QString ip, int port);
    void disConnectPLC();
    void tryToConnect();

    // void whenMove2AbsPosition(float vel, float pos);                                   // 运动到绝对位置
    void whenMove2AbsPositionDouble(double pos, double vel, double acc, double jerk);  // 运动到绝对位置(Double类型)
    void whenForward(double vel);                                                      // 正向点动
    void whenReverse(double vel);                                                      // 反向点动

private:
    QString plcIpAddress;
    int plcPort;
    bool AbMoveDone = true;
    bool AbMoveStart = false;
    double targetAbsPos = 0.0;
    double absFinishTolerance = 0.5;
    float vel = 150;  // 地轨速度

    QVector<bool> previousCoilStatuses;  // 存储上一时刻的线圈状态

    friend class RailWeldingSystem;
    friend class RailWidget;
};
#endif  // PLC_CONTROL_H

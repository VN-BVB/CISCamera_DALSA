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
    void sendRailState(int coilIndex);

    void waitForRegisterWriteComplete();
    void setRailVel(float v, int address);            // 设置地轨速度
    QVector<quint16> whenFloat2Quint16(float value);  // 将float类型转换为两个quint16寄存器值

    bool mobusDisconnect;
    modbus_t *modbusTcp = nullptr;  // modbus指针
    QTimer *readStateTimer;
    QTimer *readRealTimer;

signals:
    void senderSignalWriteFinished();
    void sendText(QString message);
    void sendTextState(QString messageAxis, QString messageMotion);
    void modbusReadRealFinished(QVariant result);
    void sendPositionAndSpeed(float position, float speed);
    void sendSignalFinishWriteCoils();
    void sendSignalFinishWriteRegisters();
    void sendSignalDisconnected();
    void sendAbsFinished();

    void sendRailStatus(QString color);  // 地轨设备状态信号

public slots:
    void writeCoils(int address, const QVector<bool> &values);
    void writeRegisters(int address, const QVector<quint16> &values);
    void connectPLC(const QString ip, int port);
    void disConnectPLC();
    void tryToConnect();

    void whenMove2AbsPosition(float vel, float pos);  // 运动到绝对位置
    void whenForward(float vel);                      // 正向点动
    void whenReverse(float vel);                      // 反向点动

private:
    QString plcIpAddress;
    int plcPort;
    bool AbMoveDone = true;
    bool AbMoveStart = false;
    float vel = 150;  // 地轨速度

    QVector<bool> previousCoilStatuses;  // 存储上一时刻的线圈状态

    friend class RailWeldingSystem;
    friend class RailWidget;
};
#endif  // PLC_CONTROL_H

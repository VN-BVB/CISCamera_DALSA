#include "motion_controller.h"

#include <QThread>
#include <QElapsedTimer>

#include "plc_addr.h"
#include "plccommon.h"

using namespace plc;

MotionController::MotionController(QObject *parent) : QObject(parent) {
    prevCoilStatuses_ = QVector<bool>(32, false);
}

MotionController::~MotionController() {
    if (plc_)    { plc_->plcDisconnect(); delete plc_; plc_ = nullptr; }
    if (params_) { delete params_; params_ = nullptr; }
}

// ============================================================
//  连接 / 断开（参照旧 rail.cpp 的 connectPLC / disConnectPLC）
// ============================================================

void MotionController::connectPlc(const QString &ip, int port) {
    plcIp_   = ip;
    plcPort_ = port;

    emit logMessage(u8"Modbus 状态: 正在连接...");

    // 清理旧连接
    if (plc_)    { plc_->plcDisconnect(); delete plc_; plc_ = nullptr; }
    if (params_) { delete params_; params_ = nullptr; }

    // 创建新 Modbus TCP 上下文
    params_ = new plcCtrlParams();
    params_->plc.ip   = ip.toStdString();
    params_->plc.port = port;

    plc_ = new PlcCommunication(params_);
    if (!plc_) {
        emit logMessage(u8"Modbus 状态: 创建 Modbus TCP 对象失败！");
        return;
    }

    // 建立连接
    if (plc_->plcConnect()) {
        connected_ = true;
        emit logMessage(u8"Modbus 状态: 协议连接成功，等待使能完成。");
        emit connectionStateChanged("green");

        // 连接成功后写使能 + 清零绝对定位触发位
        plc_->writeHdLowBit(HD_ExAxis1Enable, true);
        plc_->writeHdLowBit(HD_ExAxis1MOVEA, false);

        // 启动轮询定时器
        if (!stateTimer_) {
            stateTimer_ = new QTimer(this);
            connect(stateTimer_, &QTimer::timeout, this, &MotionController::onStateTimeout);
        }
        if (!realTimer_) {
            realTimer_ = new QTimer(this);
            connect(realTimer_, &QTimer::timeout, this, &MotionController::onRealTimeout);
        }
        stateTimer_->start(111);
        realTimer_->start(100);

        // 复位
        plc_->writeHdLowBit(HD_ExAxis1Rst, true);
    } else {
        connected_ = false;
        emit logMessage(u8"Modbus 状态: 连接失败！");
        emit connectionStateChanged("red");
    }
}

void MotionController::disconnectPlc() {
    // 先关闭使能
    if (plc_ && connected_) {
        plc_->writeHdLowBit(HD_ExAxis1Enable, false);
    }

    if (stateTimer_) stateTimer_->stop();
    if (realTimer_)  realTimer_->stop();

    if (plc_) {
        plc_->plcDisconnect();
        delete plc_;
        plc_ = nullptr;
    }
    connected_ = false;
    emit logMessage(u8"Modbus 状态: 断开连接");
    emit connectionStateChanged("red");
}

// ============================================================
//  地轨控制
// ============================================================

void MotionController::railEnable() {
    if (plc_) plc_->railEnable();
}

void MotionController::railDisable() {
    if (plc_) plc_->railDisable();
}

void MotionController::railAbsMove(double pos, double vel, double acc, double jerk) {
    if (!plc_ || !connected_) {
        emit logMessage(u8"[错误] 地轨未连接，无法运动");
        return;
    }

    targetAbsPos_    = pos;
    absMoveStarted_  = true;
    absMoveDone_     = false;

    emit logMessage(QString(u8"地轨绝对定位: pos=%1 vel=%2 acc=%3 jerk=%4")
                        .arg(pos).arg(vel).arg(acc).arg(jerk));

    if (!plc_->railAbsMove(pos, vel, acc, jerk)) {
        emit logMessage(u8"[错误] 地轨绝对定位指令发送失败");
    }
}

void MotionController::railHome() {
    if (!plc_ || !connected_) return;
    plc_->railHome();
    emit logMessage(u8"地轨回零指令已发送");
}

void MotionController::railStop() {
    if (!plc_ || !connected_) return;
    plc_->railStop();
    absMoveStarted_ = false;
    emit logMessage(u8"地轨停止指令已发送");
}

void MotionController::railForward(double vel) {
    if (!plc_ || !connected_) return;
    // 点动用绝对定位，目标位置设一个很大的值
    // 或者如果有独立的速度运动命令用那个
    emit logMessage(QString(u8"地轨正向点动: vel=%1").arg(vel));
}

void MotionController::railReverse(double vel) {
    if (!plc_ || !connected_) return;
    emit logMessage(QString(u8"地轨反向点动: vel=%1").arg(-vel));
}

// ============================================================
//  对位平台
// ============================================================

void MotionController::pltHomeAll() {
    if (!plc_ || !connected_) {
        emit logMessage(u8"[错误] PLC 未连接，无法执行一键回参");
        return;
    }
    emit logMessage(u8"一键回参: 开始");

    for (int i = 0; i < 7; ++i) {
        if (plc_->pltHome(i))
            emit logMessage(QString(u8"  平台 %1 回参指令已发送").arg(i));
        else
            emit logMessage(QString(u8"  [错误] 平台 %1 回参发送失败").arg(i));
    }
}

void MotionController::pltSingleMove(int pltIdx, double x, double y, double r, double vel) {
    if (!plc_ || !connected_) {
        emit logMessage(u8"[错误] PLC 未连接，无法运动");
        return;
    }
    if (pltIdx < 0 || pltIdx > 6) return;
    emit logMessage(QString(u8"平台 %1 单平台运动: X=%2 Y=%3 R=%4")
                        .arg(pltIdx).arg(x).arg(y).arg(r));
    plc_->pltLocate(pltIdx, x, y, r, vel, 100, 100);
}

void MotionController::axisSingleMoveR(int pltIdx, int axis, double pos, double vel) {
    if (!plc_ || !connected_) {
        emit logMessage(u8"[错误] PLC 未连接，无法运动");
        return;
    }
    if (pltIdx < 0 || pltIdx > 6 || axis < 0 || axis > 2) return;
    const char* axisName[] = {"X", "Y", "R"};
    emit logMessage(QString(u8"平台 %1 %2 轴相对运动: pos=%3")
                        .arg(pltIdx).arg(axisName[axis]).arg(pos));
    plc_->axisMoveR(pltIdx, axis, pos, vel, 100, 100);
}

void MotionController::pltEnableAll() {
    if (!plc_ || !connected_) return;
    for (int i = 0; i < 7; ++i) plc_->pltEnable(i);
    emit logMessage(u8"一键使能: 7 个平台指令已发送");
}

void MotionController::pltResetAll() {
    if (!plc_ || !connected_) return;
    for (int i = 0; i < 7; ++i) plc_->pltReset(i);
    emit logMessage(u8"一键复位: 7 个平台指令已发送");
}

void MotionController::pltLocateAll() {
    if (!plc_ || !connected_) {
        emit logMessage(u8"[错误] PLC 未连接，无法一键定位");
        return;
    }
    // TODO: 从 table_axispos + globalVel 读取 7 个平台的目标位置，然后 pltLocate
    emit logMessage(u8"一键定位: 暂未实现，请使用单平台定位");
}

void MotionController::railReset() {
    if (!plc_ || !connected_) return;
    plc_->railReset();
    emit logMessage(u8"地轨复位指令已发送");
}

// ============================================================
//  轮询
// ============================================================

void MotionController::onStateTimeout() {
    if (!plc_ || !connected_) return;

    plcFdbkParams data;
    if (plc_->plcStateFdbk(data)) {
        emit railPosVelUpdated(data.rail.pos, data.rail.vel);

        // 判断绝对定位完成：读取 HD_ExAxis1MOVEADone(2224) 低位
        if (absMoveStarted_ && !absMoveDone_) {
            int done = plc_->readHdLowBit(HD_ExAxis1MOVEADone);
            if (done == 1) {
                absMoveDone_   = true;
                absMoveStarted_ = false;
                emit railAbsFinished();
                emit logMessage(QString(u8"地轨绝对定位完成: pos=%1").arg(data.rail.pos));
            }
        }
    }
}

void MotionController::onRealTimeout() {
    // 和 state timeout 共用，实际可合并
    onStateTimeout();
}

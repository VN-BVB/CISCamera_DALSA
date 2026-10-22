#include "motion_controller.h"

#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <QThread>

#include "plc_addr.h"
#include "plccommon.h"

using namespace plc;

MotionController::MotionController(QObject *parent) : QObject(parent) {
    prevCoilStatuses_ = QVector<bool>(32, false);
    qRegisterMetaType<QVector<QVector<bool>>>("QVector<QVector<bool>>");

    playbackTimer_ = new QTimer(this);
    playbackTimer_->setInterval(5);
    connect(playbackTimer_, &QTimer::timeout, this, &MotionController::onPlaybackTick);

    homeTimer_ = new QTimer(this);
    homeTimer_->setInterval(100);
    connect(homeTimer_, &QTimer::timeout, this, &MotionController::onHomeTick);
}

MotionController::~MotionController() {
    cancelHome(u8"控制器退出");
    if (plc_) {
        plc_->plcDisconnect();
        delete plc_;
        plc_ = nullptr;
    }
    if (params_) {
        delete params_;
        params_ = nullptr;
    }
}

// ============================================================
//  连接 / 断开（参照旧 rail.cpp 的 connectPLC / disConnectPLC）
// ============================================================

void MotionController::connectPlc(const QString &ip, int port) {
    cancelHome(u8"重新连接 PLC");
    connected_ = false;
    plcIp_ = ip;
    plcPort_ = port;

    emit logMessage(u8"Modbus 状态: 正在连接...");

    // 清理旧连接
    if (plc_) {
        plc_->plcDisconnect();
        delete plc_;
        plc_ = nullptr;
    }
    if (params_) {
        delete params_;
        params_ = nullptr;
    }

    // 创建新 Modbus TCP 上下文
    params_ = new plcCtrlParams();
    params_->plc.ip = ip.toStdString();
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
        stateTimer_->start(500);

        // 复位
        plc_->writeHdLowBit(HD_ExAxis1Rst, true);
    } else {
        connected_ = false;
        emit logMessage(u8"Modbus 状态: 连接失败！");
        emit connectionStateChanged("red");
    }
}

void MotionController::disconnectPlc() {
    cancelHome(u8"断开 PLC");
    // 先关闭使能
    if (plc_ && connected_) {
        plc_->writeHdLowBit(HD_ExAxis1Enable, false);
    }

    if (stateTimer_) stateTimer_->stop();
    if (realTimer_) realTimer_->stop();

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

    targetAbsPos_ = pos;
    absMoveStarted_ = true;
    absMoveDone_ = false;

    emit logMessage(QString(u8"地轨绝对定位: pos=%1 vel=%2 acc=%3 jerk=%4").arg(pos).arg(vel).arg(acc).arg(jerk));

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
    if (homeRunning_ || playbackRunning_) {
        emit logMessage(u8"[错误] 回参或轨迹播放正在执行，无法重复启动回参");
        return;
    }
    emit logMessage(u8"一键回参: 按顺序执行平台 0~6，等待各平台 X/Y 相对运动完成");
    homePlatformIdx_ = 0;
    homeRunning_ = true;
    startHomePlatform();
}

void MotionController::startHomePlatform() {
    for (int axis = 0; axis < 2; ++axis) {
        homeSeenNotDone_[axis] = false;
        homeAxisDone_[axis] = false;
    }
    // 清除旧的回参命令，确保本次触发；不改写 PLC 的完成反馈位。
    if (!plc_->pltClearHome(homePlatformIdx_) || !plc_->pltHome(homePlatformIdx_)) {
        cancelHome(QString(u8"[错误] 平台 %1 回参发送失败").arg(homePlatformIdx_));
        return;
    }
    homeElapsed_.restart();
    homeTimer_->start();
    emit logMessage(QString(u8"  平台 %1 回参指令已发送，等待 X/Y 相对运动完成").arg(homePlatformIdx_));
}

void MotionController::onHomeTick() {
    if (!homeRunning_) {
        homeTimer_->stop();
        return;
    }
    if (!plc_ || !connected_) {
        cancelHome(u8"[错误] PLC 未连接");
        return;
    }
    if (homeElapsed_.elapsed() >= homeTimeoutMs_) {
        cancelHome(QString(u8"[错误] 平台 %1 等待本次 X/Y 相对运动完成超时（60 秒），请检查 PLC 完成信号").arg(homePlatformIdx_));
        return;
    }

    for (int axis = 0; axis < 2; ++axis) {
        const int done = plc_->readAxisMoverDone(homePlatformIdx_, axis);
        if (done < 0) {
            cancelHome(QString(u8"[错误] 平台 %1 %2 轴完成信号读取失败").arg(homePlatformIdx_).arg(axis == 0 ? "X" : "Y"));
            return;
        }
        // 本次指令发出后必须观察到 0→1，避免上次残留的 Done=1 导致提前启动下一平台。
        if (done == 0) homeSeenNotDone_[axis] = true;
        if (done == 1 && homeSeenNotDone_[axis]) homeAxisDone_[axis] = true;
    }
    if (!homeAxisDone_[0] || !homeAxisDone_[1]) return;

    if (!plc_->pltClearHome(homePlatformIdx_)) {
        cancelHome(QString(u8"[错误] 平台 %1 回参触发位清除失败").arg(homePlatformIdx_));
        return;
    }
    emit logMessage(QString(u8"  平台 %1 X/Y 相对运动完成").arg(homePlatformIdx_));
    ++homePlatformIdx_;
    if (homePlatformIdx_ >= homePlatformCount_) {
        homeRunning_ = false;
        homeTimer_->stop();
        emit logMessage(u8"一键回参: 平台 0~6 全部完成");
        return;
    }
    startHomePlatform();
}

void MotionController::cancelHome(const QString &reason) {
    if (!homeRunning_) return;
    homeRunning_ = false;
    homeTimer_->stop();
    emit logMessage(QString(u8"一键回参中止: %1").arg(reason));
    if (plc_ && connected_) {
        if (!plc_->pltStop(homePlatformIdx_)) emit logMessage(QString(u8"[错误] 平台 %1 停止指令发送失败").arg(homePlatformIdx_));
        if (!plc_->pltClearHome(homePlatformIdx_)) emit logMessage(QString(u8"[错误] 平台 %1 回参触发位清除失败").arg(homePlatformIdx_));
    }
}

void MotionController::pltSingleMove(int pltIdx, double x, double y, double r, double vel) {
    if (homeRunning_) {
        emit logMessage(u8"[错误] 一键回参正在执行，无法启动单平台运动");
        return;
    }
    if (!plc_ || !connected_) {
        emit logMessage(u8"[错误] PLC 未连接，无法运动");
        return;
    }
    if (pltIdx < 0 || pltIdx > 6) return;
    emit logMessage(QString(u8"平台 %1 单平台运动: X=%2 Y=%3 R=%4").arg(pltIdx).arg(x).arg(y).arg(r));
    plc_->pltLocate(pltIdx, x, y, r, vel, 100, 100);
}

void MotionController::axisSingleMoveR(int pltIdx, int axis, double pos, double vel) {
    if (homeRunning_) {
        emit logMessage(u8"[错误] 一键回参正在执行，无法启动单轴运动");
        return;
    }
    if (!plc_ || !connected_) {
        emit logMessage(u8"[错误] PLC 未连接，无法运动");
        return;
    }
    if (pltIdx < 0 || pltIdx > 6 || axis < 0 || axis > 2) return;
    const char *axisName[] = {"X", "Y", "R"};
    emit logMessage(QString(u8"平台 %1 %2 轴相对运动: pos=%3").arg(pltIdx).arg(axisName[axis]).arg(pos));
    plc_->axisMoveR(pltIdx, axis, pos, vel, 100, 100);
}

void MotionController::pltEnableAll() {
    if (!plc_ || !connected_) return;
    for (int i = 0; i < 7; ++i) plc_->pltEnable(i);
    emit logMessage(u8"一键使能: 7 个平台指令已发送");
}

void MotionController::pltDisableAll() {
    cancelHome(u8"一键去使能");
    if (!plc_ || !connected_) return;
    plc_->pltDisableAll();
    emit logMessage(u8"一键去使能: 7 个平台指令已发送");
}

void MotionController::pltStopAll() {
    cancelHome(u8"一键停止");
    if (!plc_ || !connected_) return;
    plc_->pltStopAll();
    emit logMessage(u8"一键停止: 7 个平台指令已发送");
}

void MotionController::pltResetAll() {
    cancelHome(u8"一键复位");
    if (!plc_ || !connected_) return;
    for (int i = 0; i < 7; ++i) plc_->pltReset(i);
    emit logMessage(u8"一键复位: 7 个平台指令已发送");
}

void MotionController::pltLocateAll() {
    if (homeRunning_) {
        emit logMessage(u8"[错误] 一键回参正在执行，无法启动一键定位");
        return;
    }
    if (!plc_ || !connected_) {
        emit logMessage(u8"[错误] PLC 未连接，无法一键定位");
        return;
    }

    // 读取 CSV
    QString csvPath = "D:/Code/CISCamera_DALSA/data/motion_commands.csv";
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit logMessage(QString(u8"[错误] 无法打开轨迹文件: %1").arg(csvPath));
        return;
    }

    QTextStream in(&file);
    struct CsvRow {
        double tx[7], ty[7], rz[7];
    };
    QVector<CsvRow> rows;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList fields = line.split(',');
        if (fields.size() < 23) continue;
        if (fields[0] == "Time(s)") continue;

        CsvRow row;
        for (int plt = 0; plt < 7; ++plt) {
            int base = 2 + plt * 3;
            row.tx[plt] = fields[base].toDouble();
            row.ty[plt] = fields[base + 1].toDouble();
            row.rz[plt] = fields[base + 2].toDouble();
        }
        rows.append(row);
    }
    file.close();

    if (rows.size() < 2) {
        emit logMessage(u8"[错误] 轨迹数据不足");
        return;
    }

    // CSV 的值是绝对位置，PLC 需要增量（当前步减去上一步）
    // 跳过第 0 行（StepIndex=0，全零起始点），从第 1 行开始
    playbackSteps_.resize(rows.size() - 1);
    for (int i = 1; i < rows.size(); ++i) {
        for (int plt = 0; plt < 7; ++plt) {
            playbackSteps_[i - 1].tx[plt] = rows[i].tx[plt] - rows[i - 1].tx[plt];
            playbackSteps_[i - 1].ty[plt] = rows[i].ty[plt] - rows[i - 1].ty[plt];
            playbackSteps_[i - 1].rz[plt] = rows[i].rz[plt] - rows[i - 1].rz[plt];
        }
    }

    // 找到最后一个有运动的步
    playbackLastStep_ = playbackSteps_.size() - 1;
    for (int i = playbackSteps_.size() - 1; i >= 0; --i) {
        bool same = true;
        for (int plt = 0; plt < 7; ++plt) {
            if (playbackSteps_[i].tx[plt] != 0 || playbackSteps_[i].ty[plt] != 0 || playbackSteps_[i].rz[plt] != 0) {
                same = false;
                break;
            }
        }
        if (!same) {
            playbackLastStep_ = i;
            break;
        }
    }

    emit logMessage(QString(u8"分步播放: 共 %1 步增量，有效至第 %2 步").arg(playbackSteps_.size()).arg(playbackLastStep_));

    // 发送第0步并启动定时器
    playbackIdx_ = 0;
    playbackWaitingDone_ = false;
    playbackRunning_ = true;
    playbackTimer_->start();
    emit logMessage(u8"分步播放: 已启动");
}

void MotionController::onPlaybackTick() {
    if (!plc_ || !connected_ || !playbackRunning_) {
        playbackTimer_->stop();
        return;
    }

    if (!playbackWaitingDone_) {
        // 发送当前步：对有增量的平台调用 pltLocatePos
        const PlaybackStep &step = playbackSteps_[playbackIdx_];
        emit logMessage(QString(u8"[Tick] 发送第 %1 步 (plt0: dx=%2 dy=%3 dr=%4)")
                            .arg(playbackIdx_)
                            .arg(step.tx[0], 0, 'f', 6)
                            .arg(step.ty[0], 0, 'f', 6)
                            .arg(step.rz[0], 0, 'f', 6));
        for (int plt = 0; plt < 7; ++plt) {
            if (step.tx[plt] != 0 || step.ty[plt] != 0 || step.rz[plt] != 0) {
                plc_->pltLocatePos(plt, step.tx[plt], step.ty[plt], step.rz[plt], 5.0);
            }
        }
        playbackWaitingDone_ = true;
        return;
    }

    // 轮询：只检查有增量的平台的 LocationDone
    const PlaybackStep &step = playbackSteps_[playbackIdx_];
    bool allDone = true;
    for (int plt = 0; plt < 7; ++plt) {
        if (step.tx[plt] == 0 && step.ty[plt] == 0 && step.rz[plt] == 0) continue;
        bool done = plc_->pltIsLocationDone(plt);
        if (!done) {
            allDone = false;
            break;
        }
    }

    if (!allDone) return;

    // 当前步完成
    playbackWaitingDone_ = false;

    if (playbackIdx_ >= playbackLastStep_) {
        playbackRunning_ = false;
        playbackTimer_->stop();
        emit logMessage(u8"分步播放: 完成");
        return;
    }

    ++playbackIdx_;
    emit logMessage(QString(u8"[Tick] 第 %1 步完成 → %2").arg(playbackIdx_ - 1).arg(playbackIdx_));
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
                absMoveDone_ = true;
                absMoveStarted_ = false;
                emit railAbsFinished();
                emit logMessage(QString(u8"地轨绝对定位完成: pos=%1").arg(data.rail.pos));
            }
        }

        // 读取 7 个平台 × 3 轴的 enableDone 状态（高位）
        QVector<QVector<bool>> axisStatus = plc_->readAllAxisEnableDone();
        emit pltAxisEnableStatus(axisStatus);
    }
}

void MotionController::onRealTimeout() {}

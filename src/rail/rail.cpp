#include "rail.h"

#include "./src/ui/utils/stateLight/StateLight.h"

#pragma execution_character_set("utf-8")

Rail::Rail(QObject *parent) : QObject(parent) {
    mobusDisconnect = 1;
    // qDebug() << "Rail constructor executed in thread:" << QThread::currentThread();
}

Rail::~Rail() {
    if (modbusTcp) {
        modbus_close(modbusTcp);  // 关闭连接
        modbus_free(modbusTcp);   // 释放资源
        modbusTcp = nullptr;
    }
}

void Rail::connectPLC(const QString ip, int port) {
    emit sendText(QString(u8"Modbus 状态: 正在连接..."));
    // QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    previousCoilStatuses = QVector<bool>(32, false);
    if (mobusDisconnect) {
        readStateTimer = new QTimer(this);
        readRealTimer = new QTimer(this);
        connect(readStateTimer, &QTimer::timeout, this, &Rail::onStateTimeout);
        connect(readRealTimer, &QTimer::timeout, this, &Rail::onRealTimeout);
    }
    if (modbusTcp != nullptr) {
        modbus_close(modbusTcp);
        modbus_free(modbusTcp);
        modbusTcp = nullptr;
    }
    // 创建新的 Modbus TCP 上下文
    modbusTcp = modbus_new_tcp(ip.toStdString().c_str(), port);
    if (modbusTcp == nullptr) {
        emit sendText(QString(u8"Modbus 状态: 创建 Modbus TCP 对象失败！"));
        return;
    }

    // 设置响应超时时间（秒、微秒）
    if (modbus_set_response_timeout(modbusTcp, 0, 200 * 1000) == -1) {
        emit sendText(QString(u8"Modbus 状态: 设置超时时间失败！"));
        modbus_free(modbusTcp);
        modbusTcp = nullptr;
        return;
    }

    // 建立连接
    if (modbus_connect(modbusTcp) == -1) {
        QString errorStr = QString::fromLocal8Bit(modbus_strerror(errno));
        emit sendText(QString(u8"Modbus 状态: 连接失败：%1").arg(errorStr));
        emit sendRailStatus(MY_COLOR::RED);
        modbus_free(modbusTcp);
        modbusTcp = nullptr;
        return;
    } else {
        mobusDisconnect = 0;
        emit sendText(QString(u8"Modbus 状态:协议连接成功，等待使能完成。"));
        writeCoils(X_ServoEnable, {true});
        readStateTimer->start(111);
        readRealTimer->start(100);
    }
    writeCoils(X_Reset, {true});
}

/**
 * @brief 重新连接
 */
void Rail::tryToConnect() {
    if (modbusTcp == nullptr) return;

    if (modbus_connect(modbusTcp) == -1) {
        emit sendText("Modbus 状态: 连接断开！");
        QThread::msleep(1000);  // 等待 1 秒后再由调用者决定是否继续
    } else {
        emit sendText("Modbus 状态: 重连成功！");
    }
}
void Rail::disConnectPLC() {
    writeCoils(X_ServoEnable, {false});
    // 断开 libmodbus 连接
    if (modbusTcp != nullptr) {
        modbus_close(modbusTcp);
        modbus_free(modbusTcp);
        modbusTcp = nullptr;
    }
    // 设置断开标志
    mobusDisconnect = 1;
    emit sendText(QString(u8"Modbus 状态: 断开连接"));
    // 停止定时器
    // if (readStateTimer->isActive()) {
    //     readStateTimer->stop();
    // }
    // if (readRealTimer->isActive()) {
    //     readRealTimer->stop();
    // }
    // emit sendSignalDisconnected();
    emit sendRailStatus(MY_COLOR::RED);
}

// 将float类型转换为两个quint16寄存器值
QVector<quint16> Rail::whenFloat2Quint16(float value) {
    union {
        float f;
        quint16 u[2];
    } data;
    data.f = value;
    QVector<quint16> values = {data.u[0], data.u[1]};  // 提取低 16 位和高 16 位
    return values;
}

// 4寄存器16无符号整数转64位浮点数
double Rail::dataTransUInt16_Double(uint16_t *data) {
    uint64_t combined = (static_cast<uint64_t>(data[3]) << 48) | (static_cast<uint64_t>(data[2]) << 32) | (static_cast<uint64_t>(data[1]) << 16) |
                        static_cast<uint64_t>(data[0]);

    double result;
    std::memcpy(&result, &combined, sizeof(result));

    return result;
}

// 将double转换成4寄存器16位无符号整数
void Rail::dataTransDouble_UInt16(double dataIn, uint16_t *dataOut) {
    uint64_t value;
    std::memcpy(&value, &dataIn, sizeof(double));  // 将 double 按位复制到 uint64_t

    dataOut[0] = static_cast<uint16_t>(value & 0xFFFF);
    dataOut[1] = static_cast<uint16_t>((value >> 16) & 0xFFFF);
    dataOut[2] = static_cast<uint16_t>((value >> 32) & 0xFFFF);
    dataOut[3] = static_cast<uint16_t>((value >> 48) & 0xFFFF);
}

// 等待寄存器写入完成
void Rail::waitForRegisterWriteComplete() {
    QEventLoop loop;
    connect(this, &Rail::sendSignalFinishWriteRegisters, &loop, &QEventLoop::quit);
    loop.exec();
}

// 设置地轨速度
void Rail::setRailVel(float v, int address) {
    QVector<quint16> values;
    values.append(whenFloat2Quint16(v));  // 绝对速度
    writeRegisters(address, values);
}

// 设置绝对位置运动参数（位置，速度，加速度，加加速度，地址）(double类型)
void Rail::setRailVelPosDouble(double pos, double vel, double acc, double jerk, int address) {
    // 运动参数
    uint16_t data[19] = {0};
    dataTransDouble_UInt16(pos, &data[0]);    // 绝对位置
    dataTransDouble_UInt16(vel, &data[4]);    // 绝对速度
    dataTransDouble_UInt16(acc, &data[8]);    // 加速度
    dataTransDouble_UInt16(acc, &data[12]);   // 减速度
    dataTransDouble_UInt16(jerk, &data[16]);  // 加加速度
    writeRegistersRaw(address, 20, data);
}

// 设置点动运动参数（速度，地址）（double类型）
void Rail::setRailVelDouble(double vel, int address) {
    // 运动参数
    uint16_t data[3] = {0};
    dataTransDouble_UInt16(vel, &data[0]);  // 绝对速度
    writeRegistersRaw(address, 4, data);
}

// // 运动到绝对位置
// void Rail::whenMove2AbsPosition(float vel, float pos) {
//     // PLOGD << L"绝对位置运动: 速度" << vel << L" 位置" << pos;
//     previousCoilStatuses[16] = 0;  // 清零地轨标志位
//     AbMoveStart = true;
//     setRailVel(vel, X_AbsSpeed);  // 设置地轨速度

//     QVector<quint16> values;
//     values.append(whenFloat2Quint16(pos));  // 绝对位置
//     writeRegisters(X_AbsPosition, values);
//     if (AbMoveDone) {
//         QVector<bool> Commands = {true};
//         writeCoils(X_AbsPositionCommand, Commands);
//     } else {
//         QVector<bool> Commands = {false};
//         writeCoils(X_AbsPositionCommand, Commands);
//         // QThread::msleep(100);
//         Commands = {true};
//         writeCoils(X_AbsPositionCommand, Commands);
//     }
//     AbMoveDone = 0;
// }

// 运动到绝对位置(Double类型)
void Rail::whenMove2AbsPositionDouble(double pos, double vel, double acc = 100, double jerk = 100) {
    // PLOGD << L"绝对位置运动: 速度" << vel << L" 位置" << pos;
    targetAbsPos = pos;
    AbMoveStart = true;
    AbMoveDone = false;
    previousCoilStatuses[16] = 0;  // 清零地轨标志位
    AbMoveStart = true;
    setRailVelPosDouble(pos, vel, acc, jerk, X_AbsSpeed);  // 设置地轨速度

    if (AbMoveDone) {
        QVector<bool> Commands = {true};
        writeCoils(X_AbsPositionCommand, Commands);
        QThread::msleep(100);
        writeCoils(X_AbsPositionCommand, {false});
    } else {
        QVector<bool> Commands = {false};
        writeCoils(X_AbsPositionCommand, Commands);
        // QThread::msleep(100);
        Commands = {true};
        writeCoils(X_AbsPositionCommand, Commands);
        QThread::msleep(100);
        writeCoils(X_AbsPositionCommand, {false});
    }
    AbMoveDone = 0;
}

// 正向点动
void Rail::whenForward(double vel) {
    setRailVelDouble(vel, X_JogSpeed);
    writeCoils(X_JogForward, {true});
}

// 反向点动
void Rail::whenReverse(double vel) {
    setRailVelDouble(-vel, X_JogSpeed);
    writeCoils(X_JogReverse, {true});
}

/**
 * @brief 写入多个线圈（Coil）
 * @param deviceId 设备 ID（通常为 1）
 * @param address 起始地址
 * @param values 要写入的布尔值数组
 */
void Rail::writeCoils(int address, const QVector<bool> &values) {
    if (modbusTcp == nullptr || modbus_get_socket(modbusTcp) < 0) {
        emit sendText("Warning : Modbus 未连接，无法写入线圈！");
        return;
    }
    // readStateTimer->stop();
    // readRealTimer->stop();

    int size = values.size();
    std::vector<uint8_t> bits(size);
    for (int i = 0; i < size; ++i) {
        bits[i] = values[i] ? 1 : 0;
    }

    int ret = modbus_write_bits(modbusTcp, address, size, bits.data());
    if (ret == -1) {
        modbus_close(modbusTcp);
        emit sendText("Modbus 状态: 写入线圈失败，尝试重连...");
        tryToConnect();
        // if (readStateTimer) readStateTimer->start();
        // if (readRealTimer) readRealTimer->start();
        return;
    }
    emit sendText(QString("Modbus 状态: 写入线圈成功，地址 0x%1，大小 %2").arg(address, 0, 16).arg(size));
    if (!mobusDisconnect) {
        emit sendText("Modbus连接PLC");
        mobusDisconnect = 0;
    }
    emit sendSignalFinishWriteCoils();

    // if(readStateTimer) readStateTimer->start();
    // if(readRealTimer) readRealTimer->start();
}

/**
 * @brief 写入多个保持寄存器（Holding Register）
 * @param deviceId 设备 ID（通常为 1）
 * @param address 起始地址
 * @param values 要写入的数值数组
 */
void Rail::writeRegisters(int address, const QVector<quint16> &values) {
    if (modbusTcp == nullptr || modbus_get_socket(modbusTcp) < 0) {
        emit sendText("Warning : Modbus 未连接，无法写入寄存器！");
        return;
    }

    // // 暂停定时器（如果需要）
    // if (readStateTimer) readStateTimer->stop();
    // if (readRealTimer) readRealTimer->stop();

    int size = values.size();
    int maxWriteNum = 123;  // modbus协议单次最大写入寄存器数

    // Modbus协议规定 单次写入寄存器的数量最多为123个。
    int writeNum = (size <= maxWriteNum) ? size : maxWriteNum;

    // libmodbus要求uint16_t*，QVector<quint16>底层类型兼容，直接取data()即可
    int ret = modbus_write_registers(modbusTcp, address, writeNum, reinterpret_cast<const uint16_t *>(values.data()));

    if (ret == -1) {
        modbus_close(modbusTcp);
        emit sendText("Modbus 状态: 写入寄存器失败，尝试重连...");
        tryToConnect();
        // if (readStateTimer) readStateTimer->start();
        // if (readRealTimer) readRealTimer->start();
        return;
    }

    // 如果你想发信号告诉写入结束
    emit sendSignalFinishWriteRegisters();

    // // 恢复定时器
    // if (readStateTimer) readStateTimer->start();
    // if (readRealTimer) readRealTimer->start();
}

int Rail::writeRegistersRaw(int addr, int num, uint16_t *data) {
    num = (num <= 123) ? num : 123;  // Modbus报文最长写入123个寄存器

    int ret = modbus_write_registers(modbusTcp, addr, num, data);
    if (ret == -1) {
        modbus_close(modbusTcp);
        tryToConnect();
        return -1;
    }
    return 0;
}

/**
 * @brief 读取 Modbus 线圈或寄存器的值
 *
 * 此函数用于从 Modbus 设备读取线圈或寄存器数据，并返回 `QVariant` 形式的结果。
 * - 如果 `isCoil` 为 `true`，则读取线圈状态，并返回 `bool`（0 -> false, 1 -> true）。
 * - 如果 `num == 1`，则读取单个寄存器，并返回 `quint16` 值。
 * - 如果 `num > 1`，则读取多个寄存器，并返回 `QVariantList`（每个元素为 `quint16`）。
 *
 * @param deviceId  设备 ID（通常为 1）
 * @param address   读取的起始地址
 * @param num       读取的数量（1 表示单个寄存器，2 以上则返回 `QVariantList`）
 * @param isCoil    是否读取线圈（`true` 读取线圈，`false` 读取保持寄存器）
 * @return QVariant 读取的值，可转换为 `bool`、`int` 或 `QVariantList`
 */
void Rail::onStateTimeout() {
    readModbusValue(X_PosLimitSignal, 32, true);  // 传入isCoil为true表示读取线圈
    // qDebug() << "onStateTimeoutonTimeout constructor executed in thread:" << QThread::currentThread();
}

void Rail::onRealTimeout() {
    readModbusValueXinjie(X_CurrentPosition, X_CurrentSpeed);
    // qDebug() << "onRealTimeoutonTimeout constructor executed in thread:" << QThread::currentThread();
}

// Xinjie读速度位置
void Rail::readModbusValueXinjie(int addresspos, int addresvel) {
    uint16_t pos[4] = {0};

    int ret1 = modbus_read_registers(modbusTcp, addresspos, 4, pos);
    if (ret1 == -1) {
        modbus_close(modbusTcp);
        tryToConnect();
        return;
    }

    double currentpos = dataTransUInt16_Double(pos);

    uint16_t vel[4] = {0};

    int ret2 = modbus_read_registers(modbusTcp, addresvel, 4, vel);
    if (ret2 == -1) {
        modbus_close(modbusTcp);
        tryToConnect();
        return;
    }

    double currentvel = dataTransUInt16_Double(vel);
    if (AbMoveStart && std::abs(currentpos - targetAbsPos) < absFinishTolerance && std::abs(currentvel) < 0.01) {
        emit sendAbsFinished();
        AbMoveDone = true;
        AbMoveStart = false;
    }

    emit sendPositionAndSpeed(currentpos, currentvel);
}

// iscoil 为1读取线圈，为0读取寄存器，num为读取线圈或寄存器的个数
void Rail::readModbusValue(int address, int num, bool isCoil) {
    if (modbusTcp == nullptr || modbus_get_socket(modbusTcp) < 0) {
        if (mobusDisconnect) {
            emit sendText("Modbus没有连接PLC，实时状态无法读取");
            mobusDisconnect = 0;
        }
        return;
    }

    if (isCoil) {
        std::vector<uint8_t> coilBuffer(num);
        int ret = modbus_read_bits(modbusTcp, address, num, coilBuffer.data());
        if (ret == -1) {
            emit sendText("读取线圈失败");
            return;
        }

        QVariantList coilStatuses;
        for (int i = 0; i < num; ++i) {
            bool status = coilBuffer[i];
            coilStatuses.append(status);

            if (i < previousCoilStatuses.size()) {
                if (status && !previousCoilStatuses[i]) {
                    sendRailState(i);
                    previousCoilStatuses[i] = status;
                } else if (!status) {
                    previousCoilStatuses[i] = status;
                }
            }
        }

        emit modbusReadRealFinished(coilStatuses);
    } else {
        std::vector<uint16_t> regBuffer(num);
        int ret = modbus_read_registers(modbusTcp, address, num, regBuffer.data());
        if (ret == -1) {
            emit sendText("读取寄存器失败");
            return;
        }

        QVariant result;
        QVariantList registerValues;

        if (num == 1) {
            result = static_cast<int>(regBuffer[0]);
        } else if (num == 2) {
            uint32_t combined = (static_cast<uint32_t>(regBuffer[1]) << 16) | regBuffer[0];
            float floatVal;
            std::memcpy(&floatVal, &combined, sizeof(float));
            result = floatVal;
        } else if (num == 4) {
            uint32_t combinedPos = (static_cast<uint32_t>(regBuffer[1]) << 16) | regBuffer[0];
            uint32_t combinedSpeed = (static_cast<uint32_t>(regBuffer[3]) << 16) | regBuffer[2];

            float position, speed;
            std::memcpy(&position, &combinedPos, sizeof(float));
            std::memcpy(&speed, &combinedSpeed, sizeof(float));

            emit sendPositionAndSpeed(position, speed);
            result = QVariant();
        } else if (num > 2 && num % 2 == 0) {
            for (int i = 0; i < num; i += 2) {
                uint32_t combined = (static_cast<uint32_t>(regBuffer[i + 1]) << 16) | regBuffer[i];
                float floatVal;
                std::memcpy(&floatVal, &combined, sizeof(float));
                registerValues.append(floatVal);
            }
            result = registerValues;
        } else {
            for (int i = 0; i < num; ++i) {
                registerValues.append(regBuffer[i]);
            }
            result = registerValues;
        }

        emit modbusReadRealFinished(result);
    }
}

void Rail::sendRailState(int coilIndex) {
    QString message;
    QString messageAxis;
    QString messageMotion;
    // 根据线圈状态生成相应的消息
    switch (coilIndex) {
        // 轴状态 (Axis Status)
        case 0:
            messageAxis = u8"到达正极限位";
            break;
        case 2:
            messageAxis = u8"到达负极限位";
            break;
        case 12:
            messageAxis = u8"相对定位执行中";
            break;
        case 13:
            messageAxis = u8"相对定位完成保持";
            break;
        case 15:
            messageAxis = u8"绝对定位执行中";
            break;
        case 16:
            messageAxis = u8"绝对定位完成保持";
            // PLOGD << L"messageAxis = u8绝对定位完成保持";
            if (AbMoveStart) {
                emit sendAbsFinished();
                AbMoveDone = true;
                AbMoveStart = false;
            }

            break;
        case 18:
            messageAxis = u8"回原执行中";
            break;
        case 19:
            messageAxis = u8"回原完成保持";
            break;
        case 21:
            messageAxis = u8"点动运行中";
            break;
        case 27:
            messageAxis = u8"去使能状态";
            break;
        case 28:
            messageAxis = u8"使能非运行";
            break;

        // 运动状态 (Motion Status)
        case 29:
            messageMotion = u8"恒速运动（或为零）";
            break;
        case 30:
            messageMotion = u8"加速运动";
            break;
        case 31:
            messageMotion = u8"减速运动";
            break;

        // 状态推送 (States Sender)
        case 4:
            message = u8"到达原点";
            break;
        case 5:
            message = u8"使能完成";
            emit sendRailStatus(MY_COLOR::GREEN);
            break;
        case 6:
            message = u8"停止完成";
            break;
        case 7:
            message = u8"复位完成";
            writeCoils(X_Reset, {false});
            break;
        case 8:
            message = u8"在当前绝对定位位置";
            break;
        case 14:
            message = u8"相对定位完成";
            break;
        case 17:
            message = u8"绝对定位完成";
            break;
        case 20:
            message = u8"回原完成";
            break;
        case 22:
            message = u8"急停完成";
            break;
            // 轴报警 (Axis Alarms)
        case 1:
            message = u8"正限位报警";
            break;
        case 3:
            message = u8"负限位报警";
            break;
        case 9:
            message = u8"驱动器报警";
            break;
        case 10:
            message = u8"轴故障中";
            break;
        case 23:
            message = u8"急停错误";
            break;
        case 24:
            message = u8"运动超调有效";
            break;
        case 25:
            message = u8"运动超调忙";
            break;
        case 26:
            message = u8"运动超调故障";
            break;
    }

    // 发送信号将信息传递到 MainWindow
    emit sendTextState(messageAxis, messageMotion);

    if (!message.isEmpty()) {
        emit sendText(message);
    }
}

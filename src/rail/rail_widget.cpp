#include "rail_widget.h"

#include <QTextCodec>

#include "ui_rail_widget.h"
// #pragma execution_character_set("utf-8")

RailWidget::RailWidget(QWidget *parent) : QWidget(parent), ui(new Ui::RailWidget) {
    ui->setupUi(this);

    qRegisterMetaType<QString>("QString");
    qRegisterMetaType<QVector<bool>>("QVector<bool>");
    qRegisterMetaType<QVector<quint16>>("QVector<quint16>");
    // 可学习QMetaObject代替
    ui->label_X_CurrentPosition->setStyleSheet("background-color: white; border: 1px solid black;");
    ui->label_X_CurrentSpeed->setStyleSheet("background-color: white; border: 1px solid black;");

    rail->moveToThread(railThread);
    connect(rail, &Rail::sendText, this, &RailWidget::whenAppendCalibrationLog);
    connect(rail, &Rail::sendTextState, this, &RailWidget::whenUpdateAMState);
    connect(rail, &Rail::sendPositionAndSpeed, this, &RailWidget::whenUpdatePositionAndSpeed);

    connect(this, &RailWidget::sendConnectToPLC, rail, &Rail::connectPLC);
    connect(this, &RailWidget::sendDisconnectToPLC, rail, &Rail::disonnectPLC);
    connect(this, &RailWidget::sendWriteCoils, rail, &Rail::writeCoils);
    connect(this, &RailWidget::sendWriteRegisters, rail, &Rail::writeRegisters);
    connect(this, &RailWidget::sendMove2AbsPosition, rail, &Rail::whenMove2AbsPosition);
    connect(this, &RailWidget::sendForward, rail, &Rail::whenForward);
    connect(this, &RailWidget::sendReverse, rail, &Rail::whenReverse);

    railThread->start();
}

RailWidget::~RailWidget() { this->disconnectRail(); }

// 设置绝对运动位置框
void RailWidget::setEditAbsPosition(QString position) { ui->edit_X_AbsPosition->setText(position); }

// 设置速度框
void RailWidget::setEditSpeed(QString speed) { ui->edit_X_AbsSpeed->setText(speed); }
// 获取当前位置
double RailWidget::getCurrentXPosition() const { return ui->label_X_CurrentPosition->text().toDouble(); }

// 在信息框推送信息
void RailWidget::whenAppendCalibrationLog(const QString message) { ui->textEdit->append(message); }

// 更新轴和运动状态信息
void RailWidget::whenUpdateAMState(const QString messageAxis, const QString messageMotion) {
    // 检查 messageAxis 是否为空，若非空则更新 edit_AxisState
    if (!messageAxis.isEmpty()) {
        ui->edit_AxisState->clear();
        ui->edit_AxisState->append(messageAxis);
    }

    // 检查 messageMotion 是否为空，若非空则更新 edit_MotionState
    if (!messageMotion.isEmpty()) {
        ui->edit_MotionState->clear();
        ui->edit_MotionState->append(messageMotion);
    }
}

// 更新地轨当前位置和速度
void RailWidget::whenUpdatePositionAndSpeed(float position, float speed) {
    if (!std::isnan(position) && !std::isnan(speed)) {
        // 处理数据并更新 UI
        ui->label_X_CurrentPosition->setText(QString::number(position, 'f', 3));
        ui->label_X_CurrentSpeed->setText(QString::number(speed, 'f', 3));
    } else {
        ui->label_X_CurrentPosition->setText("Read failed!");
        ui->label_X_CurrentSpeed->setText("Read failed!");
    }
}

// --------------------------------------- 按钮调用 --------------------------------------------
// 地轨回归原点按钮点击事件
void RailWidget::on_btn_regressOrigin_clicked() { emit sendWriteCoils(X_HomeCommand, {true}); }

// 连接PLC按钮点击事件
void RailWidget::connectRail() { emit sendConnectToPLC(ip, port); }

// 断开PLC连接按钮点击事件
void RailWidget::disconnectRail() {
    emit sendDisconnectToPLC();
    whenUpdateAMState(u8"断开连接", u8"断开连接");
}

// 绝对位置运动按钮
void RailWidget::on_btn_X_AbsPositionCommand_clicked() {
    if (rail->mobusDisconnect) {
        whenAppendCalibrationLog(QString(u8"轨道未连接"));
        return;
    }
    emit sendMove2AbsPosition(ui->edit_X_AbsSpeed->text().toFloat(), ui->edit_X_AbsPosition->text().toFloat());
}

// 运动绝对位置滑块
void RailWidget::on_horizontalSlider_X_AbsPosition_sliderMoved(int val) { ui->edit_X_AbsPosition->setText(QString::number(val)); }

// 运动速度滑块
void RailWidget::on_horizontalSlider_X_AbsSpeed_sliderMoved(int val) { ui->edit_X_AbsSpeed->setText(QString::number(val)); }

// 地轨停止按钮状态切换事件
void RailWidget::on_chk_Stop_toggled(bool checked) { emit sendWriteCoils(X_Stop, {checked}); }

// 地轨重置按钮状态切换事件
void RailWidget::on_btn_chk_Rest_clicked() { emit sendWriteCoils(X_Reset, {true}); }

// 地轨紧急停止按钮状态切换事件
void RailWidget::on_chk_ImmediateStop_toggled(bool checked) { emit sendWriteCoils(X_ImmediateStop, {checked}); }

// 地轨正向点动按钮按下事件
void RailWidget::on_btn_X_JogForward_pressed() { emit sendForward(ui->edit_X_AbsSpeed->text().toFloat()); }

// 地轨正向点动按钮释放事件
void RailWidget::on_btn_X_JogForward_released() { emit sendWriteCoils(X_JogForward, {false}); }

// 地轨反向点动按钮按下事件
void RailWidget::on_btn_X_JogReverse_pressed() { emit sendReverse(ui->edit_X_AbsSpeed->text().toFloat()); }

// 地轨反向点动按钮释放事件
void RailWidget::on_btn_X_JogReverse_released() { emit sendWriteCoils(X_JogReverse, {false}); }

// 地轨速度改变
void RailWidget::on_edit_X_AbsSpeed_textChanged(const QString &arg1) { rail->vel = arg1.toFloat(); }

void RailWidget::on_btn_contectRail_clicked() { connectRail(); }

void RailWidget::on_btn_discontectRail_clicked() { disconnectRail(); }

#include "motion_widget.h"

#include <QThread>
#include <cstdio>

#include "motion_controller.h"
#include "ui_motion_widget.h"

MotionWidget::MotionWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MotionWidget) {
    ui->setupUi(this);

    // 初始化 21 个位置单元格
    for (int r = 0; r < 7; ++r)
        for (int c = 0; c < 3; ++c) ui->table_axispos->setItem(r, c, new QTableWidgetItem("0"));

    controller_ = new MotionController();
    ctrlThread_ = new QThread(this);
    controller_->moveToThread(ctrlThread_);

    // MotionController → MotionWidget
    connect(controller_, &MotionController::railPosVelUpdated, this, &MotionWidget::onRailPosVelUpdated);
    connect(controller_, &MotionController::connectionStateChanged, this, &MotionWidget::onConnectionStateChanged);
    connect(controller_, &MotionController::logMessage, this, &MotionWidget::onLogMessage);
    connect(controller_, &MotionController::railAbsFinished, this, &MotionWidget::sendAbsFinished);

    // UI 按钮 → MotionController
    connect(ui->rail_btn_connect, &QPushButton::clicked, this, &MotionWidget::connectRail);
    connect(ui->rail_btn_disconnect, &QPushButton::clicked, this, &MotionWidget::disconnectRail);
    connect(ui->rail_btn_absLocate, &QPushButton::clicked, this, &MotionWidget::on_btn_X_AbsPositionCommand_clicked);
    connect(ui->rail_btn_home, &QPushButton::clicked, this, &MotionWidget::on_rail_btn_home_clicked);

    // 平台联控按钮
    connect(ui->plt_btn_homeAll, &QPushButton::clicked, this, [this]() { QMetaObject::invokeMethod(controller_, "pltHomeAll"); });
    connect(ui->plt_btn_enableAll, &QPushButton::clicked, this, [this]() { QMetaObject::invokeMethod(controller_, "pltEnableAll"); });
    connect(ui->plt_btn_resetAll, &QPushButton::clicked, this, [this]() { QMetaObject::invokeMethod(controller_, "pltResetAll"); });
    connect(ui->plt_btn_locateAll, &QPushButton::clicked, this, [this]() { QMetaObject::invokeMethod(controller_, "pltLocateAll"); });

    // 单平台运动 / 单轴运动
    connect(ui->plt_btn_singlepltmover, &QPushButton::clicked, this, &MotionWidget::on_plt_single_move_clicked);
    connect(ui->plt_btn_singleaxismover, &QPushButton::clicked, this, &MotionWidget::on_plt_single_axis_move_clicked);

    ctrlThread_->start();
}

MotionWidget::~MotionWidget() {
    if (ctrlThread_) {
        ctrlThread_->quit();
        ctrlThread_->wait();
    }
    delete ui;
}

// ============================================================
//  CISWidget 接口
// ============================================================

double MotionWidget::getCurrentXPosition() const { return ui->rail_edit_curPos->text().toDouble(); }

void MotionWidget::setEditAbsPosition(const QString &position) { ui->rail_edit_targetPos->setText(position); }

void MotionWidget::setEditSpeed(const QString &speed) { ui->rail_edit_targetVel->setText(speed); }

void MotionWidget::on_btn_X_AbsPositionCommand_clicked() {
    double pos = ui->rail_edit_targetPos->text().toDouble();
    double vel = ui->rail_edit_targetVel->text().toDouble();
    double acc = 10.0;  // TODO: 后续加 acc 输入框
    double jerk = 10.0;

    QMetaObject::invokeMethod(controller_, "railAbsMove", Q_ARG(double, pos), Q_ARG(double, vel), Q_ARG(double, acc), Q_ARG(double, jerk));
}

// CISWidget 用这个来停止地轨
void MotionWidget::on_chk_Stop_toggled(bool checked) {
    Q_UNUSED(checked);
    QMetaObject::invokeMethod(controller_, "railStop");
}

// ============================================================
//  平台控制槽 — 读 UI 控件的值
// ============================================================

void MotionWidget::on_plt_single_move_clicked() {
    int pltIdx = ui->combo_singlepltchoose->currentIndex();
    double x = ui->table_axispos->item(pltIdx, 0) ? ui->table_axispos->item(pltIdx, 0)->text().toDouble() : 0;
    double y = ui->table_axispos->item(pltIdx, 1) ? ui->table_axispos->item(pltIdx, 1)->text().toDouble() : 0;
    double r = ui->table_axispos->item(pltIdx, 2) ? ui->table_axispos->item(pltIdx, 2)->text().toDouble() : 0;

    QMetaObject::invokeMethod(controller_, "pltSingleMove", Q_ARG(int, pltIdx), Q_ARG(double, x), Q_ARG(double, y), Q_ARG(double, r),
                              Q_ARG(double, 5.0));
}

void MotionWidget::on_plt_single_axis_move_clicked() {
    int pltIdx = ui->combo_singleaxispltchoose->currentIndex();
    int axis = ui->combo_singleaxischoose->currentIndex();
    double pos = ui->plt_edit_singleaxismoverpos->text().toDouble();

    QMetaObject::invokeMethod(controller_, "axisSingleMoveR", Q_ARG(int, pltIdx), Q_ARG(int, axis), Q_ARG(double, pos), Q_ARG(double, 5.0));
}

// ============================================================
//  按钮槽
// ============================================================

void MotionWidget::connectRail() { QMetaObject::invokeMethod(controller_, "connectPlc", Q_ARG(QString, ip_), Q_ARG(int, port_)); }

void MotionWidget::disconnectRail() { QMetaObject::invokeMethod(controller_, "disconnectPlc"); }

void MotionWidget::on_rail_btn_home_clicked() { QMetaObject::invokeMethod(controller_, "railHome"); }

void MotionWidget::onRailPosVelUpdated(double pos, double vel) {
    if (!std::isnan(pos) && !std::isnan(vel)) {
        ui->rail_edit_curPos->setText(QString::number(pos, 'f', 3));
        ui->rail_edit_curVel->setText(QString::number(vel, 'f', 3));
    } else {
        ui->rail_edit_curPos->setText("Read failed!");
        ui->rail_edit_curVel->setText("Read failed!");
    }
}

void MotionWidget::onConnectionStateChanged(const QString &color) {
    if (color == "green") {
        ui->rail_label_status->setStyleSheet("background-color: #22c55e; border-radius: 7px; border: 1px solid #16a34a;");
        ui->rail_label_state->setText(u8"已连接");
    } else if (color == "red") {
        ui->rail_label_status->setStyleSheet("background-color: #ef4444; border-radius: 7px; border: 1px solid #dc2626;");
        ui->rail_label_state->setText(u8"已断开");
    }
}

void MotionWidget::onLogMessage(const QString &msg) { fprintf(stdout, "[Motion] %s\n", msg.toLocal8Bit().constData()); }

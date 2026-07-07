#include "motion_widget.h"

#include "ui_motion_widget.h"

MotionWidget::MotionWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MotionWidget) {
    ui->setupUi(this);
}

MotionWidget::~MotionWidget() { delete ui; }

double MotionWidget::getCurrentXPosition() const { return 0.0; }

void MotionWidget::setEditAbsPosition(const QString &position) { Q_UNUSED(position); }

void MotionWidget::setEditSpeed(const QString &speed) { Q_UNUSED(speed); }

void MotionWidget::on_btn_X_AbsPositionCommand_clicked() {}

void MotionWidget::on_chk_Stop_toggled(bool checked) { Q_UNUSED(checked); }

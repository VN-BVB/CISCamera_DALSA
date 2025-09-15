#include "CISCameraImage.h"

#include "ui_CISCameraImage.h"

CISWidget::CISWidget(QWidget *parent) : QWidget(parent), ui(new Ui::CISWidget) {
    ui->setupUi(this);
    initCamera();
    initUIConnections();
}

CISWidget::~CISWidget() {
    if (CISCamera) {
        CISCamera->stopGrab();
        CISCamera->moveToThread(QApplication::instance()->thread());  // 回到主线程
    }
    delete ui;
}

void CISWidget::on_btnStart_clicked() {}

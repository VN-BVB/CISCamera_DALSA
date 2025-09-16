#include "CISCameraImage.h"

#include "ui_CISCameraImage.h"

CISWidget::CISWidget(QWidget* parent) : QWidget(parent), ui(new Ui::CISWidget) {
    ui->setupUi(this);
    initCamera();
    initUIConnections();
}

CISWidget::~CISWidget() {
    if (CISCamera) {
        CISCamera->stopGrab();
    }
    delete ui;
}
void CISWidget::initCamera() {
    // 使用工厂模式创建相机对象
    CISCamera = AbstractCameraFactory::createCamera(CameraType::DALSA);

    // 将相机对象移动到独立线程
    CISCamera->moveToThread(cameraThread);
    cameraThread->start();
    QMetaObject::invokeMethod(CISCamera.get(), [=]() { CISCamera->initCamera("xxx.ccf"); }, Qt::QueuedConnection);
}
void CISWidget::initUIConnections() {
    qRegisterMetaType<cv::Mat>("cv::Mat");
    connect(CISCamera.get(), &AbstractCamera::newImageReady, this, &CISWidget::onNewImage, Qt::QueuedConnection);
}

void CISWidget::onNewImage(const cv::Mat& img) {
    ui->imgLive->setOpenCVImage(img);
    // 如果启用拼接模式
    if (ui->ckbSplice->isChecked()) {
        if (resultMat.empty()) {
            resultMat = cv::Mat::zeros(img.cols, img.rows * 10, img.type());
            offset_x = 0;
        }
        //  90 度
        cv::Mat rotated;
        cv::rotate(img, rotated, cv::ROTATE_90_CLOCKWISE);

        if (offset_x + rotated.cols <= resultMat.cols) {
            rotated.copyTo(resultMat(cv::Rect(offset_x, 0, rotated.cols, rotated.rows)));
            offset_x += rotated.cols;
        } else {
            // resultMat = cv::Mat::zeros(img.cols, img.rows * 10, img.type());
            offset_x = 0;
            rotated.copyTo(resultMat(cv::Rect(offset_x, 0, rotated.cols, rotated.rows)));
            offset_x += rotated.cols;
        }
        ui->imgSplice->setOpenCVImage(resultMat);
    }
}

void CISWidget::on_btnStart_clicked() {
    if (CISCamera) {
        QMetaObject::invokeMethod(CISCamera.get(), "startGrab");
    }
}

void CISWidget::on_btnStop_clicked() {
    if (CISCamera) {
        QMetaObject::invokeMethod(CISCamera.get(), "stopGrab");
    }
}

void CISWidget::on_btnFreeze_clicked() {
    if (CISCamera) {
        QMetaObject::invokeMethod(CISCamera.get(), "freezeGrab", Q_ARG(bool, true));
    }
}

void CISWidget::on_btnContinue_clicked() {
    if (CISCamera) {
        QMetaObject::invokeMethod(CISCamera.get(), "freezeGrab", Q_ARG(bool, false));
    }
}
void CISWidget::on_ckbSplice_toggled(bool checked) {
    if (!checked) {
        resultMat = cv::Mat::zeros(1, 1, CV_8UC3);

        ui->imgSplice->setOpenCVImage(resultMat);
        offset_x = 0;
    }
}

void CISWidget::on_ckbSave_toggled(bool checked) {
    if (CISCamera) {
        QMetaObject::invokeMethod(CISCamera.get(), "saveFrames", Q_ARG(bool, checked));
    }
}

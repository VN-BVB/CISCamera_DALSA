#include "cis_camera_image.h"

#include "ui_cis_camera_image.h"
#define ENABLE_SLAVE_CAMERA
CISWidget::CISWidget(QWidget* parent) : QWidget(parent), ui(new Ui::CISWidget) {
    ui->setupUi(this);
    initCamera();
    initUIConnections();
}

CISWidget::~CISWidget() {
    if (masterCISCamera) {
        masterCISCamera->stopGrab();
    }
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) {
        slaveCISCamera->stopGrab();
    }
#endif
    delete ui;
}

void CISWidget::initCamera() {
    // Master Camera
    masterCISCamera = AbstractCameraFactory::createCamera(CameraType::DALSA);
    masterCISCamera->moveToThread(cameraThreadMaster);
    cameraThreadMaster->start();
    QMetaObject::invokeMethod(
        masterCISCamera.get(), [=]() { masterCISCamera->initCamera("./data/CISConfig/MasterInternal.ccf"); }, Qt::QueuedConnection);

#ifdef ENABLE_SLAVE_CAMERA
    // Slave Camera
    slaveCISCamera = AbstractCameraFactory::createCamera(CameraType::DALSA);
    slaveCISCamera->moveToThread(cameraThreadSlave);
    cameraThreadSlave->start();
    QMetaObject::invokeMethod(
        slaveCISCamera.get(), [=]() { slaveCISCamera->initCamera("./data/CISConfig/SlaveInternal.ccf"); }, Qt::QueuedConnection);
#endif

    // 外部配置程序
    configCISCamera = std::make_shared<ExternalExeRunner>();
    configCISCamera->moveToThread(cameraThreadConfig);
    cameraThreadConfig->start();
}

void CISWidget::initUIConnections() {
    qRegisterMetaType<cv::Mat>("cv::Mat");

    connect(masterCISCamera.get(), &AbstractCamera::sendNewImageReady, this, &CISWidget::whenGetNewImage, Qt::QueuedConnection);

#ifdef ENABLE_SLAVE_CAMERA
    connect(
        masterCISCamera.get(), &AbstractCamera::sendNewImageReady, this,
        [=](const cv::Mat& img) {
            masterImg = img.clone();
            masterReady = true;
            tryStitchImages();
        },
        Qt::QueuedConnection);

    connect(
        slaveCISCamera.get(), &AbstractCamera::sendNewImageReady, this,
        [=](const cv::Mat& img) {
            slaveImg = img.clone();
            slaveReady = true;
            tryStitchImages();
        },
        Qt::QueuedConnection);
#endif
}

void CISWidget::whenGetNewImage(const cv::Mat& img) { ui->imgLive->setOpenCVImage(img); }

#ifdef ENABLE_SLAVE_CAMERA
void CISWidget::tryStitchImages() {
    if (ui->ckbSplice->isChecked() && masterReady && slaveReady) {
        cv::hconcat(masterImg, slaveImg, resultMat);
        ui->imgSplice->setOpenCVImage(resultMat);
        masterReady = false;
        slaveReady = false;
    }
}
#endif

void CISWidget::on_btnStart_clicked() {
    if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "startGrab");
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "startGrab");
#endif
}

void CISWidget::on_btnStop_clicked() {
    if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "stopGrab");
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "stopGrab");
#endif
}

void CISWidget::on_btnFreeze_clicked() {
    if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "freezeGrab", Q_ARG(bool, true));
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "freezeGrab", Q_ARG(bool, true));
#endif
}

void CISWidget::on_btnContinue_clicked() {
    if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "freezeGrab", Q_ARG(bool, false));
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "freezeGrab", Q_ARG(bool, false));
#endif
}

void CISWidget::on_ckbSave_toggled(bool checked) {
    if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "saveFrames", Q_ARG(bool, checked));
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "saveFrames", Q_ARG(bool, checked));
#endif
}

void CISWidget::on_btnSoftWareTrigger_clicked() {
    if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "softwareTrigger");
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "softwareTrigger");
#endif
}

void CISWidget::on_ckbSplice_toggled(bool checked) {
    if (!checked) {
        resultMat = cv::Mat::zeros(1, 1, CV_8UC3);
    }
}

void CISWidget::on_btnCISConfig_clicked() {
    if (configCISCamera) {
        QMetaObject::invokeMethod(
            configCISCamera.get(),
            [=]() {
                configCISCamera->addDllDirToPath("./data/CISConfig/externExE");
                configCISCamera->start("./data/CISConfig/externExE/ConfigCIS.exe", {"--help"});
                configCISCamera->writeInput("some command");
            },
            Qt::QueuedConnection);
    }
}

#include "cis_camera_image.h"

#include "ui_cis_camera_image.h"
#define ENABLE_SLAVE_CAMERA
CISWidget::CISWidget(QWidget* parent) : QWidget(parent), ui(new Ui::CISWidget) {
    ui->setupUi(this);
    initCamera();
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
    // // 单独开一个线程来串行初始化，避免阻塞主线程
    QThread* initThread = QThread::create([this]() {
        // Master
        masterCISCamera = AbstractCameraFactory::createCamera(CameraType::DALSA);
        if (!masterCISCamera->initCamera("./data/CISConfig/MasterInternal.ccf", 0)) {
            PLOGE << "Master 初始化失败";
            return;
        } else {
            PLOGD << "Master 初始化成功";
        }

        // Slave
        slaveCISCamera = AbstractCameraFactory::createCamera(CameraType::DALSA);
        if (!slaveCISCamera->initCamera("./data/CISConfig/SlaveInternal.ccf", 1)) {
            PLOGE << "Slave 初始化失败";
            return;
        } else {
            PLOGD << "Slave 初始化成功";
        }

        // --- 初始化完成后，再切换到采集线程 ---
        masterCISCamera->moveToThread(cameraThreadMaster);
        cameraThreadMaster->start();

        slaveCISCamera->moveToThread(cameraThreadSlave);
        cameraThreadSlave->start();

        // 外部配置程序
        configCISCamera = std::make_shared<ExternalExeRunner>();
        configCISCamera->moveToThread(cameraThreadConfig);
        cameraThreadConfig->start();

        PLOGD << "相机配置初始化完成";
        initCamera2UIConnections();
    });

    initThread->start();
}

void CISWidget::initCamera2UIConnections() {
    qRegisterMetaType<cv::Mat>("cv::Mat");

    connect(slaveCISCamera.get(), &AbstractCamera::sendNewImageReady, this, &CISWidget::whenGetNewImage, Qt::QueuedConnection);

#ifdef ENABLE_SLAVE_CAMERA
    connect(
        masterCISCamera.get(), &AbstractCamera::sendNewImageReady, this,
        [=](const cv::Mat& img) {
            masterImg = img;
            masterReady = true;
            tryStitchImages();
        },
        Qt::QueuedConnection);

    connect(
        slaveCISCamera.get(), &AbstractCamera::sendNewImageReady, this,
        [=](const cv::Mat& img) {
            slaveImg = img;
            slaveReady = true;
            tryStitchImages();
        },
        Qt::QueuedConnection);
#endif
}

void CISWidget::whenGetNewImage(const cv::Mat& img) { ui->imgLive->setOpenCVImage(img); }

#ifdef ENABLE_SLAVE_CAMERA
void CISWidget::tryStitchImages() {
    PLOGD << "进入拼接函数";

    PLOGD << "ckbSplice checked = " << ui->ckbSplice->isChecked();
    PLOGD << "masterReady = " << masterReady << ", slaveReady = " << slaveReady;
    PLOGD << "masterImg size = " << masterImg.cols << "x" << masterImg.rows << ", empty = " << masterImg.empty();
    PLOGD << "slaveImg size = " << slaveImg.cols << "x" << slaveImg.rows << ", empty = " << slaveImg.empty();

    if (ui->ckbSplice->isChecked() && masterReady && slaveReady) {
        try {
            cv::hconcat(masterImg, slaveImg, resultMat);
            PLOGD << "拼接成功, resultMat size = " << resultMat.cols << "x" << resultMat.rows;
            ui->imgSplice->setOpenCVImage(resultMat);

            masterReady = false;
            slaveReady = false;
        } catch (const cv::Exception& e) {
            PLOGE << "拼接失败: " << e.what();
        }
    } else {
        PLOGD << "条件未满足，未执行拼接";
    }
}

#endif

void CISWidget::on_btnStart_clicked() {
    if (masterCISCamera) {
        PLOGD << "启动 master camera";
        QMetaObject::invokeMethod(masterCISCamera.get(), "startGrab");
    }
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) {
        PLOGD << "启动 slave camera";
        QMetaObject::invokeMethod(slaveCISCamera.get(), "startGrab");
    } else {
        PLOGE << "slaveCISCamera is null!";
    }
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

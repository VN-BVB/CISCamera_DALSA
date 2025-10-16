#include "cis_camera_image.h"

#include "ui_cis_camera_image.h"
#define ENABLE_SLAVE_CAMERA
CISWidget::CISWidget(QWidget* parent) : QWidget(parent), ui(new Ui::CISWidget) {
    ui->setupUi(this);
    initregisterMetaType();
    initCamera();
    initUIControls();
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
        whenAppendMessageLog(QString(u8"DALSA采集卡初始化中"));
        // Master
        masterCISCamera = AbstractCameraFactory::createCamera(CameraType::DALSA);
        if (!masterCISCamera->initCamera("./data/CISConfig/MasterInternal.ccf", 0)) {
            PLOGE << "Master 初始化失败";
            whenAppendMessageLog(QString(u8"Master 初始化失败"));
            return;
        } else {
            PLOGD << "Master 初始化成功";
            whenAppendMessageLog(QString(u8"Master 初始化成功"));
        }

        // Slave
        slaveCISCamera = AbstractCameraFactory::createCamera(CameraType::DALSA);
        if (!slaveCISCamera->initCamera("./data/CISConfig/SlaveInternal.ccf", 1)) {
            PLOGE << "Slave 初始化失败";
            whenAppendMessageLog(QString(u8"Slave 初始化失败"));
            return;
        } else {
            PLOGD << "Slave 初始化成功";
            whenAppendMessageLog(QString(u8"Slave 初始化成功"));
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
        whenAppendMessageLog(QString(u8"相机配置初始化完成"));
        initCamera2UIConnections();
    });

    initThread->start();
}
void CISWidget::initCamera2UIConnections() {
    connect(slaveCISCamera.get(), &AbstractCamera::sendNewImageReady, this, &CISWidget::whenGetNewImage, Qt::QueuedConnection);
#ifdef ENABLE_SLAVE_CAMERA
    connect(
        masterCISCamera.get(), &AbstractCamera::sendNewImageReady, this,
        [=](std::shared_ptr<cv::Mat> imgPtr) {
            masterImg = *imgPtr;
            masterReady = true;
            tryStitchImages();
        },
        Qt::QueuedConnection);

    connect(
        slaveCISCamera.get(), &AbstractCamera::sendNewImageReady, this,
        [=](std::shared_ptr<cv::Mat> imgPtr) {
            slaveImg = *imgPtr;
            slaveReady = true;
            tryStitchImages();
        },
        Qt::QueuedConnection);
#endif
    connect(masterCISCamera.get(), &AbstractCamera::sendText, this, &CISWidget::whenAppendMessageLog, Qt::QueuedConnection);
    connect(slaveCISCamera.get(), &AbstractCamera::sendText, this, &CISWidget::whenAppendMessageLog, Qt::QueuedConnection);
}
void CISWidget::initUIControls() {
    ui->btnSoftWareTrigger->setEnabled(true);
    ui->ckbSplice->setChecked(true);
}

void CISWidget::initregisterMetaType() {
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<std::shared_ptr<cv::Mat>>("std::shared_ptr<cv::Mat>");
}
void CISWidget::whenGetNewImage(std::shared_ptr<cv::Mat> matPt) { ui->imgLive->setOpenCVImage(*matPt); }
// 在信息框推送信息
void CISWidget::whenAppendMessageLog(const QString& message) { ui->textEdit->append(message); }

#ifdef ENABLE_SLAVE_CAMERA
void CISWidget::tryStitchImages() {
    PLOGD << "进入拼接函数";
    whenAppendMessageLog(QString(u8"进入拼接函数"));
    PLOGD << "ckbSplice checked = " << ui->ckbSplice->isChecked();
    PLOGD << "masterReady = " << masterReady << ", slaveReady = " << slaveReady;
    PLOGD << "masterImg size = " << masterImg.cols << "x" << masterImg.rows << ", empty = " << masterImg.empty();
    PLOGD << "slaveImg size = " << slaveImg.cols << "x" << slaveImg.rows << ", empty = " << slaveImg.empty();

    if (ui->ckbSplice->isChecked() && masterReady && slaveReady) {
        try {
            cv::hconcat(masterImg, slaveImg, resultMat);
            PLOGD << "拼接成功, resultMat size = " << resultMat.cols << "x" << resultMat.rows;
            whenAppendMessageLog(QString(u8"拼接成功, resultMat size = %1 x %2").arg(resultMat.cols).arg(resultMat.rows));
            ui->imgSplice->setOpenCVImage(resultMat);

            masterReady = false;
            slaveReady = false;
        } catch (const cv::Exception& e) {
            PLOGE << "拼接失败: " << e.what();
            whenAppendMessageLog(QString(u8"拼接失败: %1").arg(e.what()));
        }
    } else {
        PLOGD << "条件未满足，未执行拼接";
        whenAppendMessageLog(QString(u8"条件未满足，未执行拼接"));
    }
}
#endif
void CISWidget::on_btnSave_clicked() {
    QDir dir("./data/CISCamera_Image");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    if (ui->ckbSplice->isChecked()) {
        if (!resultMat.empty()) {
            QString fileName = dir.filePath(QString("Splice_%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz")));

            try {
                cv::imwrite(fileName.toStdString(), resultMat);
                whenAppendMessageLog(QString(u8"已保存拼接图像: %1").arg(fileName));
            } catch (const cv::Exception& e) {
                whenAppendMessageLog(QString(u8"保存拼接图像失败: %1").arg(e.what()));
            }
        } else {
            whenAppendMessageLog(QString(u8"未检测到拼接图像，无法保存"));
        }

    } else {
        // 未拼接状态，调用原有相机保存逻辑
        bool checked = true;
        if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "saveFrames", Q_ARG(bool, checked));
#ifdef ENABLE_SLAVE_CAMERA
        if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "saveFrames", Q_ARG(bool, checked));
#endif
        whenAppendMessageLog(QString(u8"分别保存主/从相机的图像帧"));
    }
}

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
    ui->btnSoftWareTrigger->setEnabled(true);
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

// 软件触发
void CISWidget::on_btnSoftWareTrigger_clicked() {
    on_btnStart_clicked();
    double currentPos = ui->railWidget->getCurrentXPosition();
    // 如果当前未在380附近，则先运动到380
    if (std::abs(currentPos - startPos) > 1.0) {
        connect(ui->railWidget->rail, &Rail::sendAbsFinished, this, &CISWidget::whenMoveToStartFinished);
        ui->railWidget->setEditAbsPosition(QString::number(startPos));
        ui->railWidget->setEditSpeed(QString::number(speed));
        ui->railWidget->on_btn_X_AbsPositionCommand_clicked();
    } else {
        // 已在起点，直接开始扫描
        whenMoveToStartFinished();
    }
}
void CISWidget::whenMoveToStartFinished() {
    disconnect(ui->railWidget->rail, &Rail::sendAbsFinished, this, &CISWidget::whenMoveToStartFinished);

    // 启动相机采集（软件触发）
    if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "softwareTrigger");
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "softwareTrigger");
#endif
    ui->railWidget->setEditAbsPosition(QString::number(endPos));
    ui->railWidget->setEditSpeed(QString::number(speed));
    ui->railWidget->on_btn_X_AbsPositionCommand_clicked();
    connect(ui->railWidget->rail, &Rail::sendAbsFinished, this, [this]() {
        // 手动断开这个信号，确保只触发一次
        disconnect(ui->railWidget->rail, &Rail::sendAbsFinished, nullptr, nullptr);
        on_btnStop_clicked();
    });
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

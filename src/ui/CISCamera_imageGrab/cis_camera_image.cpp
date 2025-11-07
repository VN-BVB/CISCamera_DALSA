#include "cis_camera_image.h"

#include "ui_cis_camera_image.h"
#define ENABLE_SLAVE_CAMERA
CISWidget::CISWidget(QWidget* parent) : QWidget(parent), ui(new Ui::CISWidget) {
    ui->setupUi(this);

    initregisterMetaType();
    initUIControls();
    initCISCameraConfig();
    initCameraImageProcessor();
    initCamera();
    initCameraCalibrator();
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
    if (processorThread) {
        processorThread->quit();
        processorThread->wait();
    }
    delete ui;
}

void CISWidget::initUIControls() {
    ui->btnSoftWareTrigger->setEnabled(true);
    ui->ckbSplice->setChecked(true);
}

void CISWidget::initregisterMetaType() {
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<std::shared_ptr<cv::Mat>>("std::shared_ptr<cv::Mat>");
}
// 外部配置程序
void CISWidget::initCISCameraConfig() {
    configCISCamera = std::make_shared<ExternalExeRunner>();
    configCISCamera->moveToThread(cameraThreadConfig);
    cameraThreadConfig->start();
    connect(configCISCamera.get(), &ExternalExeRunner::sendMessage2UI, this, &CISWidget::whenAppendMessageLog,
            Qt::QueuedConnection);
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
            masterImg = imgPtr;
            masterReady = true;
            tryStitchImages();
        },
        Qt::QueuedConnection);

    connect(
        slaveCISCamera.get(), &AbstractCamera::sendNewImageReady, this,
        [=](std::shared_ptr<cv::Mat> imgPtr) {
            slaveImg = imgPtr;
            slaveReady = true;
            tryStitchImages();
        },
        Qt::QueuedConnection);
#endif
    connect(masterCISCamera.get(), &AbstractCamera::sendText, this, &CISWidget::whenAppendMessageLog, Qt::QueuedConnection);
    connect(slaveCISCamera.get(), &AbstractCamera::sendText, this, &CISWidget::whenAppendMessageLog, Qt::QueuedConnection);
}

void CISWidget::initCameraImageProcessor() {
    imageProcessor = std::make_shared<CameraImageProcessor>();
    processorThread = new QThread(this);
    imageProcessor->moveToThread(processorThread);
    processorThread->start();

    // 信号连接
    connect(
        imageProcessor.get(), &CameraImageProcessor::imageReady, this,
        [this](std::shared_ptr<cv::Mat> result) {
            if (result && !result->empty()) {
                ui->imgSplice->displayImage(result, true);
            }
        },
        Qt::QueuedConnection);
    connect(imageProcessor.get(), &CameraImageProcessor::text, this, &CISWidget::whenAppendMessageLog, Qt::QueuedConnection);
    connect(imageProcessor.get(), &CameraImageProcessor::error, this, &CISWidget::whenAppendMessageLog, Qt::QueuedConnection);
}
void CISWidget::initCameraCalibrator() {
    libcbDetector = std::make_shared<LibCBDetector>();
    telecentricLineCalibrator = std::make_shared<TelecentricLineCalibrator>();
}
void CISWidget::whenGetNewImage(std::shared_ptr<cv::Mat> matPt) { ui->imgLive->setOpenCVImage(*matPt); }
// 在信息框推送信息
void CISWidget::whenAppendMessageLog(const QString& message) { ui->textEdit->append(message); }

void CISWidget::tryStitchImages() {
    if (ui->ckbSplice->isChecked() && masterReady && slaveReady) {
        masterReady = slaveReady = false;

        QMetaObject::invokeMethod(imageProcessor.get(), "processPair", Qt::QueuedConnection,
                                  Q_ARG(std::shared_ptr<cv::Mat>, masterImg), Q_ARG(std::shared_ptr<cv::Mat>, slaveImg),
                                  Q_ARG(bool, true)  // 或 ui->ckbSplice->isChecked()
        );
    }
}
void CISWidget::on_btnSave_clicked() {
    if (ui->ckbSplice->isChecked()) {
        QMetaObject::invokeMethod(imageProcessor.get(), "saveResult", Qt::QueuedConnection,
                                  Q_ARG(QString, "./data/CISCamera_Image"), Q_ARG(QString, "Splice"),
                                  Q_ARG(QString, ".bmp"),  // 需要更高精度可改 ".tif" / ".exr"
                                  Q_ARG(bool, false)       // 是否同时保存主/从
        );
    } else {
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
    startPos = ui->end_lineEdit->text().toDouble();
    endPos = ui->end_lineEdit->text().toDouble();
    speed = ui->speed_lineEdit->text().toDouble();
    if (triggerRunning) {
        whenAppendMessageLog(QString(u8"帧触发进行中"));
        return;
    } else {
        triggerRunning = true;
    }
    on_btnStart_clicked();
    double currentPos = ui->railWidget->getCurrentXPosition();
    disconnect(ui->railWidget->rail, &Rail::sendAbsFinished, this, &CISWidget::whenMoveToStartFinished);
    // 如果当前未在380附近，则先运动到380
    if (std::abs(currentPos - startPos) > 1.0) {
        ui->railWidget->on_chk_Stop_toggled(false);
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

    // connect(ui->railWidget->rail, &Rail::sendAbsFinished, this, [this]() {
    //     disconnect(ui->railWidget->rail, &Rail::sendAbsFinished, nullptr, nullptr);
    //     on_btnStop_clicked();
    // });
    // 使用QMetaObject::Connection来管理信号连接，以便精确断开
    static QMetaObject::Connection endMoveConnection;
    endMoveConnection = connect(ui->railWidget->rail, &Rail::sendAbsFinished, this, [this]() {
        // 只断开当前建立的连接
        disconnect(endMoveConnection);
        on_btnStop_clicked();
    });
    triggerRunning = false;
}

void CISWidget::on_btnStopTrigger_clicked() {
    triggerRunning = false;
    ui->railWidget->on_chk_Stop_toggled(true);
}
void CISWidget::on_ckbSplice_toggled(bool checked) {
    if (!checked) {
    }
}

void CISWidget::on_btnCISConfig_clicked() {
    if (!configCISCamera) return;

    QMetaObject::invokeMethod(
        configCISCamera.get(),
        [this]() {
            configCISCamera->addDllDirToPath("./data/CISConfig/externExE");

            configCISCamera->startEmbedded("./data/CISConfig/externExE/ConfigCIS.exe", {"--help"}, ui->cisConfigHost->winId());
            configCISCamera->writeInput("some command");
        },
        Qt::QueuedConnection);
}

void CISWidget::on_btn_ChessboardDetector_clicked() {}

void CISWidget::on_btnCameraCalibrate_clicked() {}

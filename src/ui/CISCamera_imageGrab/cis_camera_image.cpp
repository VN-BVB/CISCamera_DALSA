#include "cis_camera_image.h"

#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <fstream>
#include <sstream>

#include "src/cameraFactory/abstract_camera.h"
#include "src/cameraFactory/abstract_camera_factory.h"
#include "src/cameraFactory/dalsaCameralink/external_exe_runner.h"
#include "src/config/calibration_data_io.h"
#include "src/motion/motion_widget.h"
#include "src/telecentricLineCalibrator/libcbdetect/lib_cb_detecor.h"
#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"
#include "src/ui/utils/display/graphicItems/axes_item.h"
#include "src/ui/utils/display/graphicItems/circle_item.h"
#include "src/ui/utils/display/graphicItems/graphic_item_component.h"
#include "src/ui/utils/display/graphicItems/graphic_item_composite.h"
#include "src/ui/utils/display/graphicItems/point_item.h"
#include "src/utils/image_utils.cpp"
#include "ui_cis_camera_image.h"
#define ENABLE_SLAVE_CAMERA
CISWidget::CISWidget(QWidget* parent) : QWidget(parent), ui(new Ui::CISWidget) {
    ui->setupUi(this);

    initregisterMetaType();
    initUIControls();
    initCISCameraConfig();
    initCameraImageProcessor();
    initCamera();
    PLOG_INFO << "当前主线程";
    std::string filePath = R"(D:\Code\CISCamera_DALSA\data\CISCamera_Image\test\Splice_20251030_214803209.bmp)";
    // 读取图像
    cv::Mat img = readLargeBMP(filePath);
    if (img.empty()) {
        img = cv::imread(filePath, cv::IMREAD_GRAYSCALE);  // 回退（小图或非BMP）
    }
    ui->imgSplice->displayImage(std::make_shared<cv::Mat>(img), true);
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
    ui->ckbShowPLlatImg->setChecked(true);
}

void CISWidget::initregisterMetaType() {
    qRegisterMetaType<Pose>("Pose");
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<Eigen::Vector2d>("Eigen::Vector2d");
    qRegisterMetaType<Eigen::Matrix3d>("Eigen::Matrix3d");
    qRegisterMetaType<std::vector<Pose>>("std::vector<Pose>");
    qRegisterMetaType<std::shared_ptr<cv::Mat>>("std::shared_ptr<cv::Mat>");
    qRegisterMetaType<std::vector<Eigen::Vector2d>>("std::vector<Eigen::Vector2d>");
    qRegisterMetaType<std::vector<std::vector<Eigen::Vector2d>>>("std::vector<std::vector<Eigen::Vector2d>>");
}
// 外部配置程序
void CISWidget::initCISCameraConfig() {
    configCISCamera = std::make_shared<ExternalExeRunner>();
    configCISCamera->moveToThread(cameraThreadConfig);
    cameraThreadConfig->start();
    connect(configCISCamera.get(), &ExternalExeRunner::sendMessage2UI, this, &CISWidget::whenAppendMessageLog, Qt::QueuedConnection);
}

void CISWidget::initCamera() {
    // // 单独开一个线程来串行初始化，避免阻塞主线程
    QThread* initThread = QThread::create([this]() {
        whenAppendMessageLog(QString(u8"DALSA采集卡初始化中"));
        // Master
        masterCISCamera = AbstractCameraFactory::createCamera(CameraType::DALSA);
        if (!masterCISCamera->initCamera(masterCameraCCF_, 0)) {
            PLOG_ERROR << "Master 初始化失败";
            whenAppendMessageLog(QString(u8"Master 初始化失败"));
            return;
        } else {
            PLOG_INFO << "Master 初始化成功";
            whenAppendMessageLog(QString(u8"Master 初始化成功"));
        }

        // Slave
        slaveCISCamera = AbstractCameraFactory::createCamera(CameraType::DALSA);
        if (!slaveCISCamera->initCamera(slaveCameraCCF_, 1)) {
            PLOG_ERROR << "Slave 初始化失败";
            whenAppendMessageLog(QString(u8"Slave 初始化失败"));
            return;
        } else {
            PLOG_INFO << "Slave 初始化成功";
            whenAppendMessageLog(QString(u8"Slave 初始化成功"));
        }

        // --- 初始化完成后，再切换到采集线程 ---
        masterCISCamera->moveToThread(cameraThreadMaster);
        cameraThreadMaster->start();

        slaveCISCamera->moveToThread(cameraThreadSlave);
        cameraThreadSlave->start();

        PLOG_INFO << "相机配置初始化完成";
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
    connect(
        imageProcessor.get(), &CameraImageProcessor::imageReady, this,
        [this](std::shared_ptr<cv::Mat> result) {
            if (result && !result->empty()) {
                ui->imgSplice->clearAllDisplayImages();  // ← 新增：先清除旧图
                ui->imgSplice->displayImage(result, true);
            }
        },
        Qt::QueuedConnection);
    connect(imageProcessor.get(), &CameraImageProcessor::text, this, &CISWidget::whenAppendMessageLog, Qt::QueuedConnection);
    connect(imageProcessor.get(), &CameraImageProcessor::error, this, &CISWidget::whenAppendMessageLog, Qt::QueuedConnection);
    connect(imageProcessor.get(), &CameraImageProcessor::platformCalibDone, this, &CISWidget::whenDrawPlatformAxes, Qt::QueuedConnection);
    connect(imageProcessor.get(), &CameraImageProcessor::platformCalibDone, this, &CISWidget::whenDrawDetectCircles, Qt::QueuedConnection);
    QMetaObject::invokeMethod(imageProcessor.get(), [=]() { imageProcessor->initCameraCalibrator(); }, Qt::QueuedConnection);
}
void CISWidget::whenGetNewImage(std::shared_ptr<cv::Mat> matPt) { ui->imgLive->setOpenCVImage(*matPt); }
// 在信息框推送信息
void CISWidget::whenAppendMessageLog(const QString& message) { ui->textEdit->append(message); }

void CISWidget::tryStitchImages() {
    if (ui->ckbSplice->isChecked() && masterReady && slaveReady) {
        masterReady = slaveReady = false;

        QMetaObject::invokeMethod(imageProcessor.get(), "processPair", Qt::QueuedConnection, Q_ARG(std::shared_ptr<cv::Mat>, masterImg),
                                  Q_ARG(std::shared_ptr<cv::Mat>, slaveImg), Q_ARG(bool, true), Q_ARG(bool, false)  // 或 ui->ckbSplice->isChecked()
        );
    }
}
void CISWidget::on_btnSave_clicked() {
    if (ui->ckbSplice->isChecked()) {
        QMetaObject::invokeMethod(imageProcessor.get(), "saveResult", Qt::QueuedConnection, Q_ARG(QString, "./data/CISCamera_Image"),
                                  Q_ARG(QString, "Splice"), Q_ARG(QString, ".bmp"),  // 需要更高精度可改 ".tif" / ".exr"
                                  Q_ARG(bool, false)                                 // 是否同时保存主/从
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

// 添加返回值检查和显式连接类型
void CISWidget::on_btnStart_clicked() {
    if (masterCISCamera) {
        PLOG_INFO << "启动 master camera";
        bool ok = QMetaObject::invokeMethod(masterCISCamera.get(), "startGrab", Qt::QueuedConnection);
        if (!ok) {
            PLOG_ERROR << "Master startGrab invokeMethod 失败";
            whenAppendMessageLog(u8"Master 采集启动失败");
        }
    }
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) {
        PLOG_INFO << "启动 slave camera";
        bool ok = QMetaObject::invokeMethod(slaveCISCamera.get(), "startGrab", Qt::QueuedConnection);
        if (!ok) {
            PLOG_ERROR << "Slave startGrab invokeMethod 失败";
            whenAppendMessageLog(u8"Slave 采集启动失败");
        }
    } else {
        PLOGE << "slaveCISCamera is null!";
    }
#endif
    ui->btnSoftWareTrigger->setEnabled(true);
}

// 添加返回值检查和显式连接类型
void CISWidget::on_btnStop_clicked() {
    if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "stopGrab", Qt::QueuedConnection);
#ifdef ENABLE_SLAVE_CAMERA
    if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "stopGrab", Qt::QueuedConnection);
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
    startPos = ui->start_lineEdit->text().toDouble();
    endPos = ui->end_lineEdit->text().toDouble();
    speed = ui->speed_lineEdit->text().toDouble();
    bool leadOk = false;
    const double leadMs = ui->lead_lineEdit->text().toDouble(&leadOk);
    leadInTimer = (leadOk && leadMs >= 0.0) ? leadMs : 1000.0;
    if (triggerRunning) {
        whenAppendMessageLog(QString(u8"帧触发进行中"));
        return;
    } else {
        triggerRunning = true;
    }
    masterReady = false;
    slaveReady = false;
    masterImg.reset();
    slaveImg.reset();
    on_btnStart_clicked();
    ui->btnSoftWareTrigger->setEnabled(false);
    double currentPos = ui->motionWidget->getCurrentXPosition();
    disconnect(ui->motionWidget, &MotionWidget::sendAbsFinished, this, &CISWidget::whenMoveToStartFinished);
    if (std::abs(currentPos - startPos) > 0.05) {
        ui->motionWidget->on_chk_Stop_toggled(false);
        connect(ui->motionWidget, &MotionWidget::sendAbsFinished, this, &CISWidget::whenMoveToStartFinished);
        ui->motionWidget->setEditAbsPosition(QString::number(startPos));
        ui->motionWidget->setEditSpeed(QString::number(speed));
        ui->motionWidget->on_btn_X_AbsPositionCommand_clicked();
    } else {
        // 已在起点，直接开始扫描
        whenMoveToStartFinished();
    }
}
// void CISWidget::whenMoveToStartFinished() {
//     disconnect(ui->motionWidget, &MotionWidget::sendAbsFinished, this, &CISWidget::whenMoveToStartFinished);

//     // 启动相机采集（软件触发）
//     if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "softwareTrigger");
// #ifdef ENABLE_SLAVE_CAMERA
//     if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "softwareTrigger");
// #endif
//     ui->motionWidget->setEditAbsPosition(QString::number(endPos));
//     ui->motionWidget->setEditSpeed(QString::number(speed));
//     ui->motionWidget->on_btn_X_AbsPositionCommand_clicked();

//     // connect(ui->motionWidget->rail, &Rail::sendAbsFinished, this, [this]() {
//     //     disconnect(ui->motionWidget->rail, &Rail::sendAbsFinished, nullptr, nullptr);
//     //     on_btnStop_clicked();
//     // });
//     // 使用QMetaObject::Connection来管理信号连接，以便精确断开
//     static QMetaObject::Connection endMoveConnection;
//     endMoveConnection = connect(ui->motionWidget->rail, &Rail::sendAbsFinished, this, [this]() {
//         // 只断开当前建立的连接
//         double currentPos = ui->motionWidget->getCurrentXPosition();
//         if (std::abs(currentPos - endPos) < 0.5) {
//             disconnect(endMoveConnection);
//             on_btnStop_clicked();
//         }
//     });
//     triggerRunning = false;
// }

void CISWidget::whenMoveToStartFinished() {
    disconnect(ui->motionWidget, &MotionWidget::sendAbsFinished, this, &CISWidget::whenMoveToStartFinished);
    disconnect(endMoveConnection_);
    // 下发扫描运动
    ui->motionWidget->setEditAbsPosition(QString::number(endPos));
    ui->motionWidget->setEditSpeed(QString::number(speed));
    ui->motionWidget->on_btn_X_AbsPositionCommand_clicked();

    whenAppendMessageLog(u8"扫描运动已下发，进入 lead-in 阶段");
    const int leadMs = static_cast<int>(leadInTimer);
    QTimer::singleShot(leadMs, this, [this]() {
        if (!triggerRunning) return;

        if (masterCISCamera) QMetaObject::invokeMethod(masterCISCamera.get(), "softwareTrigger", Qt::QueuedConnection);
#ifdef ENABLE_SLAVE_CAMERA
        if (slaveCISCamera) QMetaObject::invokeMethod(slaveCISCamera.get(), "softwareTrigger", Qt::QueuedConnection);
#endif
        scanStartPosReal = ui->motionWidget->getCurrentXPosition();

        whenAppendMessageLog(QString(u8"Lead-in %1 ms 到达，开始相机触发\n"
                                     u8"扫描起始位置：%2")
                                 .arg(leadInTimer)
                                 .arg(scanStartPosReal, 0, 'f', 3));

        endMoveConnection_ = connect(ui->motionWidget, &MotionWidget::sendAbsFinished, this, [this]() {
            disconnect(endMoveConnection_);
            scanEndPosReal = ui->motionWidget->getCurrentXPosition();

            whenAppendMessageLog(QString(u8"扫描结束\n"
                                         u8"  起始位置：%1\n"
                                         u8"  结束位置：%2\n"
                                         u8"  实际位移：%3")
                                     .arg(scanStartPosReal, 0, 'f', 3)
                                     .arg(scanEndPosReal, 0, 'f', 3)
                                     .arg(scanEndPosReal - scanStartPosReal, 0, 'f', 3));
            whenAppendMessageLog(QString(u8"扫描结束\n"
                                         u8"  起始位置：%1\n"
                                         u8"  结束位置2：%2\n"
                                         u8"  实际位移2：%3")
                                     .arg(scanStartPosReal, 0, 'f', 3)
                                     .arg(endPos, 0, 'f', 3)
                                     .arg(endPos - scanStartPosReal, 0, 'f', 3));

            triggerRunning = false;
            ui->btnSoftWareTrigger->setEnabled(true);
            // 延迟 1 秒等所有帧回传完再 stopGrab
            QTimer::singleShot(1000, this, [this]() { on_btnStop_clicked(); });
        });
    });
}

void CISWidget::on_btnStopTrigger_clicked() {
    disconnect(ui->motionWidget, &MotionWidget::sendAbsFinished, this, &CISWidget::whenMoveToStartFinished);
    disconnect(endMoveConnection_);
    triggerRunning = false;
    ui->btnSoftWareTrigger->setEnabled(true);
    ui->motionWidget->on_chk_Stop_toggled(true);
    on_btnStop_clicked();
}
void CISWidget::on_ckbSplice_toggled(bool checked) {
    if (!checked) {
    }
}

// void CISWidget::on_btnCISConfig_clicked() {
//     if (!configCISCamera) return;
//     QMetaObject::invokeMethod(
//         configCISCamera.get(),
//         [this]() {
//             configCISCamera->addDllDirToPath("./data/CISConfig/externExE");

//             configCISCamera->startEmbedded("./data/CISConfig/externExE/ConfigCIS.exe", {"--help"}, ui->cisConfigHost->winId());
//             configCISCamera->writeInput("some command");
//         },
//         Qt::QueuedConnection);
// }

void CISWidget::on_btn_ChessboardDetector_clicked() {
    QMetaObject::invokeMethod(imageProcessor.get(), &CameraImageProcessor::whenDetectChessboard, Qt::QueuedConnection);
}

void CISWidget::on_btnCameraCalibrate_clicked() {
    QMetaObject::invokeMethod(imageProcessor.get(), [=]() { imageProcessor->whenCameraCalibrate(); }, Qt::QueuedConnection);
}

void CISWidget::on_btnSaveAligenmentPlatImg_clicked() {
    int idx = ui->cbxPlatform->currentIndex();

    QMetaObject::invokeMethod(imageProcessor.get(), [=]() { imageProcessor->savePlatfromCailbImg("Splice", ".bmp", idx); }, Qt::QueuedConnection);
}

void CISWidget::on_btnCalibratePlat_clicked() {
    QMetaObject::invokeMethod(imageProcessor.get(), [=]() { imageProcessor->whenCalibrateCP(); }, Qt::QueuedConnection);
}

void CISWidget::on_btnReadLocalImg_clicked() {
    QMetaObject::invokeMethod(imageProcessor.get(), [=]() { imageProcessor->loadPlatformCalibImages(); }, Qt::QueuedConnection);
}

void CISWidget::on_btnClearCPImg_clicked() {
    if (imageProcessor) {
        QMetaObject::invokeMethod(imageProcessor.get(), "whenClearPlatFromFile", Qt::QueuedConnection, Q_ARG(QString, "img"));
    }
}

void CISWidget::on_btnClearCPDetectResult_clicked() {
    QMetaObject::invokeMethod(imageProcessor.get(), [=]() { imageProcessor->whenClearPlatFromFile("txt"); }, Qt::QueuedConnection);
}

void CISWidget::whenDrawPlatformAxes() {
    if (!imageProcessor) return;

    // 1. 加载 platform_pose.json（旧格式，含 worldPose + platforms[]）
    PlatformPoseData cam2PlatParam;
    if (!cam2PlatParam.load("./data/calibration_config/platform_pose.json")) return;

    auto savedR = imageProcessor->getWorldRvec();
    auto savedT = imageProcessor->getWorldTvec();
    imageProcessor->setWorldPose(cam2PlatParam.allRotVecs.back(), cam2PlatParam.allTransVecs.back());

    // 2. 收集像素坐标（从旋转向量恢复 X/Y 方向向量）
    struct PlatAxes {
        int id;
        double cx, cy, xx, xy, yx, yy;
    };
    std::vector<PlatAxes> axes;
    const double axisLen = 3.2;  // ~150 像素

    for (int i = 0; i + 1 < cam2PlatParam.allTransVecs.size(); ++i) {
        Eigen::Vector3d T_w = cam2PlatParam.allTransVecs[i];
        // 旋转向量 → 旋转矩阵 → X/Y 方向向量
        Eigen::Vector3d rv = cam2PlatParam.allRotVecs[i];
        cv::Mat rv_cv(3, 1, CV_64F);
        rv_cv.at<double>(0) = rv(0);
        rv_cv.at<double>(1) = rv(1);
        rv_cv.at<double>(2) = rv(2);
        cv::Mat R_cv;
        cv::Rodrigues(rv_cv, R_cv);
        Eigen::Vector3d X_w(R_cv.at<double>(0,0), R_cv.at<double>(1,0), R_cv.at<double>(2,0));
        Eigen::Vector3d Y_w(R_cv.at<double>(0,1), R_cv.at<double>(1,1), R_cv.at<double>(2,1));

        std::vector<Eigen::Vector2d> pts;
        pts.push_back(T_w.head<2>());
        pts.push_back((T_w + axisLen * X_w).head<2>());
        pts.push_back((T_w + axisLen * Y_w).head<2>());

        auto px = imageProcessor->convertToPix(pts);
        axes.push_back({i, px[0].x(), px[0].y(), px[1].x(), px[1].y(), px[2].x(), px[2].y()});
        PLOG_INFO << std::fixed << std::setprecision(5) << "平台 " << i << " 像素(" << px[0].x() << "," << px[0].y() << ")" << std::endl;
    }
    imageProcessor->setWorldPose(savedR, savedT);

    // 3. 加载原图，画轴
    if (axes.empty()) return;

    cv::Mat img = readLargeBMP("./data/PaltfromCalibrate/vis/vis.bmp");
    if (img.empty()) img = cv::imread("./data/PaltfromCalibrate/vis/vis.bmp", cv::IMREAD_UNCHANGED);
    if (img.empty()) return;

    cv::Mat gray = img.clone();
    if (gray.channels() == 3) cv::cvtColor(gray, gray, cv::COLOR_BGR2GRAY);

    ui->graphicsView_5->clearAllGraphicComponents();
    for (auto& a : axes) {
        std::shared_ptr<AxesItem> CpAxis = std::make_shared<AxesItem>(a.cx, a.cy, a.xx, a.xy, a.yx, a.yy, a.id);
        ui->graphicsView_5->addGraphicComponent(CpAxis);
    }
    ui->graphicsView_5->clearAllDisplayImages();
    ui->graphicsView_5->displayImage(std::make_shared<cv::Mat>(gray), true);
}

void CISWidget::whenDrawDetectCircles() {
    if (!imageProcessor) return;

    // 1. 加载 platform_pose.json，提取各平台旋转中心 T 的世界坐标
    PlatformPoseData cam2PlatParam;
    if (!cam2PlatParam.load("./data/calibration_config/platform_pose.json")) return;

    auto savedR = imageProcessor->getWorldRvec();
    auto savedT = imageProcessor->getWorldTvec();
    imageProcessor->setWorldPose(cam2PlatParam.allRotVecs.back(), cam2PlatParam.allTransVecs.back());

    // 解析平台 id → T 世界坐标
    struct PlatCenter {
        int id;
        Eigen::Vector2d worldT;
    };
    std::vector<PlatCenter> platCenters;
    for (int i = 0; i + 1 < cam2PlatParam.allTransVecs.size(); ++i) {
        platCenters.push_back({i, cam2PlatParam.allTransVecs[i].head<2>()});
    }

    // 把平台旋转中心转成像素坐标
    std::vector<Eigen::Vector2d> platPixels;
    for (auto& pc : platCenters) {
        auto px = imageProcessor->convertToPix({pc.worldT});
        platPixels.push_back(px[0]);
    }

    // 2. blob 检测
    cv::SimpleBlobDetector::Params params;
    params.minArea = 7e4;
    params.maxArea = 8e4;
    params.minCircularity = 0.7f;
    params.filterByCircularity = true;
    params.filterByColor = false;
    params.filterByConvexity = false;
    params.filterByInertia = false;

    cv::Ptr<cv::FeatureDetector> blobDetector = cv::SimpleBlobDetector::create(params);

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat img = readLargeBMP("./data/PaltfromCalibrate/vis/vis.bmp");
    if (img.empty()) img = cv::imread("./data/PaltfromCalibrate/vis/vis.bmp", cv::IMREAD_UNCHANGED);
    if (img.empty()) return;

    blobDetector->detect(img, keypoints);
    if (keypoints.empty()) return;

    cv::Mat gray;
    if (img.channels() == 3)
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else
        gray = img.clone();

    cv::Mat binary;
    cv::threshold(gray, binary, 240, 255, cv::THRESH_BINARY);

    std::vector<cv::Point2d> centers;
    std::vector<double> diameters;
    std::vector<QString> labels;
    const int pad = 20;

    for (auto& kp : keypoints) {
        int cx = cvRound(kp.pt.x), cy = cvRound(kp.pt.y);
        int roiSize = cvRound(kp.size) + pad * 2;
        int x = std::max(0, cx - roiSize / 2);
        int y = std::max(0, cy - roiSize / 2);
        int w = std::min(roiSize, binary.cols - x);
        int h = std::min(roiSize, binary.rows - y);

        cv::Mat roiBin = binary(cv::Rect(x, y, w, h));
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(roiBin, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        if (contours.empty()) continue;

        auto& cnt = *std::max_element(contours.begin(), contours.end(), [](auto& a, auto& b) { return cv::contourArea(a) < cv::contourArea(b); });
        if (cnt.size() < 6) continue;

        cv::Point2f ctr;
        float r;
        cv::minEnclosingCircle(cnt, ctr, r);
        double dx = ctr.x + x;
        double dy = ctr.y + y;
        centers.emplace_back(dx, dy);
        diameters.push_back(r * 2.0);

        // 3. 计算 offset：圆心到最近平台旋转中心的 (dx², dy²)
        double minDist2 = 1e18;
        int bestId = -1;
        Eigen::Vector2d bestPlatPix;
        for (size_t k = 0; k < platCenters.size(); ++k) {
            double d2 = std::pow(dx - platPixels[k].x(), 2) + std::pow(dy - platPixels[k].y(), 2);
            if (d2 < minDist2) {
                minDist2 = d2;
                bestId = platCenters[k].id;
                bestPlatPix = platPixels[k];
            }
        }
        double offsetX = dx - bestPlatPix.x();
        double offsetY = dy - bestPlatPix.y();
        double dist = std::sqrt(offsetX * offsetX + offsetY * offsetY);
        labels.push_back(QString(u8"plt%1 offset: %2").arg(bestId).arg(dist, 0, 'f', 1));
    }

    imageProcessor->setWorldPose(savedR, savedT);

    // 4. 显示
    std::shared_ptr<CircleItem> circleLight = std::make_shared<CircleItem>(centers, diameters, Qt::green, 1.0, 15.0, labels);
    // ui->graphicsView_5->clearAllGraphicComponents();
    ui->graphicsView_5->addGraphicComponent(circleLight);
}

std::vector<Eigen::Vector2d> CISWidget::convertToWorldDemo(const std::vector<Eigen::Vector2d>& pix_pts) {
    std::vector<Eigen::Vector2d> world;
    QMetaObject::invokeMethod(imageProcessor.get(), [&]() { world = imageProcessor->convertToWorld(pix_pts); }, Qt::BlockingQueuedConnection);
    return world;
}

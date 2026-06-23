#ifndef CIS_CAMERA_IMAGE_H
#define CIS_CAMERA_IMAGE_H
// #define Internal1
#include <QDateTime>
#include <QImage>
#include <QPainter>
#include <QThread>
#include <QTimer>
#include <QWidget>
#include <memory>
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include "cameraImage_processor.h"
class RailWidget;
class AbstractCamera;
class ExternalExeRunner;
class CameraImageProcessor;

QT_BEGIN_NAMESPACE
namespace Ui {
class CISWidget;
}
QT_END_NAMESPACE

class CISWidget : public QWidget {
    Q_OBJECT

public:
    CISWidget(QWidget* parent = nullptr);
    ~CISWidget();

    std::vector<Eigen::Vector2d> convertToWorldDemo(const std::vector<Eigen::Vector2d>& pix_pts);

private:
    void initCamera();
    void initCamera2UIConnections();
    void initUIControls();
    void initregisterMetaType();
    void initCameraImageProcessor();
    void initCISCameraConfig();
    void tryStitchImages();
    Ui::CISWidget* ui;
    QThread* cameraThreadMaster = new QThread;
    QThread* cameraThreadSlave = new QThread;
    QThread* cameraThreadConfig = new QThread;
    QThread* processorThread = nullptr;
    std::shared_ptr<CameraImageProcessor> imageProcessor{nullptr};
    std::shared_ptr<AbstractCamera> masterCISCamera{nullptr};
    std::shared_ptr<AbstractCamera> slaveCISCamera{nullptr};
    std::shared_ptr<ExternalExeRunner> configCISCamera{nullptr};
    std::shared_ptr<cv::Mat> masterImg, slaveImg;
#ifdef Internal1
    QString masterCameraCCF_ = "./data/CISConfig/MasterInternalFrameInternal.ccf ";
    QString slaveCameraCCF_ = "./data/CISConfig/SlaveInternalFrameInternal.ccf";
#else
    QString masterCameraCCF_ = "./data/CISConfig/MasterEncoderDriver.ccf";
    QString slaveCameraCCF_ = "./data/CISConfig/SlaveEncoderDriver.ccf";
#endif
    bool masterReady = false;
    bool slaveReady = false;
    bool triggerRunning = false;
    double leadInTimer = 1000.0;
    double startPos = 0.0;
    double endPos = 0.0;
    double speed = 0.0;
    double scanStartPosReal = 0.0, scanEndPosReal = 0.0;
    QMetaObject::Connection endMoveConnection_;
public slots:
    void whenAppendMessageLog(const QString& message);
    void whenMoveToStartFinished();
private slots:
    void whenGetNewImage(std::shared_ptr<cv::Mat> matPt);
    void on_btnStart_clicked();
    void on_btnStop_clicked();
    void on_btnFreeze_clicked();
    void on_btnContinue_clicked();
    void on_ckbSplice_toggled(bool checked);
    void on_btnSoftWareTrigger_clicked();
    void on_btnCISConfig_clicked();
    void on_btnSave_clicked();
    void on_btnStopTrigger_clicked();
    void on_btn_ChessboardDetector_clicked();
    void on_btnCameraCalibrate_clicked();
    void on_btnSaveAligenmentPlatImg_clicked();
    void on_btnCalibratePlat_clicked();
    void on_btnReadLocalImg_clicked();
    void on_btnClearCPImg_clicked();
    void on_btnClearCPDetectResult_clicked();
};
#endif  // CIS_CAMERA_IMAGE_H


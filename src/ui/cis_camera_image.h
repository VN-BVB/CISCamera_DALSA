#ifndef CIS_CAMERA_IMAGE_H
#define CIS_CAMERA_IMAGE_H

#include <QImage>
#include <QPainter>
#include <QThread>
#include <QWidget>
#include <memory>
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include "src/cameraFactory/abstract_camera.h"
#include "src/cameraFactory/abstract_camera_factory.h"
#include "src/cameraFactory/dalsaCameralink/external_exe_runner.h"
#include "src/rail/rail_widget.h"
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

private:
    Ui::CISWidget* ui;

    QThread* cameraThreadMaster = new QThread;
    QThread* cameraThreadSlave = new QThread;
    QThread* cameraThreadConfig = new QThread;

    std::shared_ptr<AbstractCamera> masterCISCamera{nullptr};
    std::shared_ptr<AbstractCamera> slaveCISCamera{nullptr};
    std::shared_ptr<ExternalExeRunner> configCISCamera{nullptr};

    cv::Mat masterImg, slaveImg, resultMat;
    bool masterReady = false;
    bool slaveReady = false;
    const double startPos = 380.0;
    const double endPos = 720.0;
    const double speed = 30.0;

    void tryStitchImages();
    void initCamera2UIConnections();
    void initCamera();
    void initUIControls();
public slots:
    void whenAppendMessageLog(const QString& message);
    void whenMoveToStartFinished();
private slots:
    void whenGetNewImage(const cv::Mat& img);
    void on_btnStart_clicked();
    void on_btnStop_clicked();
    void on_btnFreeze_clicked();
    void on_btnContinue_clicked();
    void on_ckbSplice_toggled(bool checked);
    void on_ckbSave_toggled(bool checked);
    void on_btnSoftWareTrigger_clicked();
    void on_btnCISConfig_clicked();
};
#endif  // CIS_CAMERA_IMAGE_H

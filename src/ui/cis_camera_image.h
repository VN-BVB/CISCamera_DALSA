#ifndef CIS_CAMERA_IMAGE_H
#define CIS_CAMERA_IMAGE_H

#include <QImage>
#include <QPainter>
#include <QThread>
#include <QWidget>

#include "src/cameraFactory/abstract_camera.h"
#include "src/cameraFactory/abstract_camera_factory.h"
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

    QThread* cameraThread = new QThread;

    std::shared_ptr<AbstractCamera> CISCamera{nullptr};

    cv::Mat resultMat;
    int offset_x = 0;

    void initUIConnections();
    void initCamera();
private slots:
    void whenGetNewImage(const cv::Mat& img);
    void on_btnStart_clicked();
    void on_btnStop_clicked();
    void on_btnFreeze_clicked();
    void on_btnContinue_clicked();
    void on_ckbSplice_toggled(bool checked);
    void on_ckbSave_toggled(bool checked);
    void on_comboBox_currentTextChanged(const QString& arg1);
    void on_btnSoftWareTrigger_clicked();
};
#endif  // CIS_CAMERA_IMAGE_H

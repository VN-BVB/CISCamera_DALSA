#ifndef CISCAMERAIMAGE_H
#define CISCAMERAIMAGE_H

#include <QImage>
#include <QPainter>
#include <QThread>
#include <QWidget>

#include "src/cameraFactory/AbstractCamera.h"
#include "src/cameraFactory/AbstractCameraFactory.h"
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
#endif  // CISCAMERAIMAGE_H

#ifndef CISCAMERAIMAGE_H
#define CISCAMERAIMAGE_H

#include <QImage>
#include <QPainter>
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

    QThread* cameraThread;

    std::shared_ptr<AbstractCamera> CISCamera;

    QImage resultImage;
    int offset_x = 0;
    bool isSplice = false;

    void initUIConnections();
    void initCamera();
private slots:
    // void onNewImage(const QImage& img);
    void on_btnStart_clicked();
};
#endif  // CISCAMERAIMAGE_H

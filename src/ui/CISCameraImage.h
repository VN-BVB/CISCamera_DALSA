#ifndef CISCAMERAIMAGE_H
#define CISCAMERAIMAGE_H

#include <QImage>
#include <QPainter>
#include <QWidget>

#include "src/cameraFactory/AbstractCamera.h"
#include "src/cameraFactory/AbstractCameraFactory.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget* parent = nullptr);
    ~Widget();

private:
    Ui::Widget* ui;
    QImage* resultImage;
    QPainter* painter;
    AbstractCamera* camera = nullptr;
    int offset_x = 0;
    bool isSplice = false;

    void initUIConnections();
    void initCamera();
private slots:
    void onNewImage(const QImage& img);
};
#endif  // CISCAMERAIMAGE_H

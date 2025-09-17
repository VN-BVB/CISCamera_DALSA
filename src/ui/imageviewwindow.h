#ifndef IMAGEVIEWWINDOW_H
#define IMAGEVIEWWINDOW_H

#include <QWidget>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Ui {
class ImageViewWindow;
}

class ImageViewWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewWindow(QWidget *parent = nullptr);
    ~ImageViewWindow();

private slots:
    void on_pb_open_clicked();

private:
    Ui::ImageViewWindow *ui;
};

#endif // IMAGEVIEWWINDOW_H

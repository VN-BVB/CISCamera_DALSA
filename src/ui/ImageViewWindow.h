#ifndef IMAGEVIEWWINDOW_H
#define IMAGEVIEWWINDOW_H

#include <QWidget>
#include <QGraphicsView>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QThread>

#include "src/jointDetection/ImagereadWorker.h"
#include "src/jointDetection/ImageProcessWorker.h"

namespace Ui {
class ImageViewWindow;
}

class ImageViewWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewWindow(QWidget *parent = nullptr);
    ~ImageViewWindow();

signals:
    void startImageRead(const QString &path);
    void startImageProcess(cv::Mat image);

private slots:
    void on_pb_open_clicked();
    void handleImageRead(cv::Mat image);
    void handleImageProcessed(cv::Mat processedImage, std::vector<cv::Point2f> subpixelContour);
    void handleError(const QString &error);


private:
    void drawSubpixelContour(QGraphicsScene *scene, const std::vector<cv::Point2f> &subpixelContour);

    Ui::ImageViewWindow *ui;
    QThread readThread;
    QThread processThread;
    ImageReadWorker *readWorker;
    ImageProcessWorker *processWorker;
};

#endif // IMAGEVIEWWINDOW_H

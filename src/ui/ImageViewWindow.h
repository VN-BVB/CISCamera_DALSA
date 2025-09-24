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
#include "src/jointDetection/edgeDetection/CannyDevernay.h"

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
    void handleImageProcessed(cv::Mat processedImage, std::vector<cv::Point2f> subpixelContour,
                              std::vector<std::vector<cv::Point>> pixelContour);
    void handleImageProcessedCannyDevenay(cv::Mat processedImage, std::vector<Point2fCurve> edgeCurves);
    void handleError(const QString &error);


private:
    // @TODO：绘制轮廓接口优化，可以创建一个轮廓绘制类，类中继承GraphicsItem，设置轮廓属性，线条属性等信息
    void drawSubpixelContour(QGraphicsScene *scene, const std::vector<cv::Point2f> &subpixelContour);
    void drawPixelContour(QGraphicsScene *scene, const std::vector<cv::Point> &pixelContour);
    void drawContour(QGraphicsScene *scene, const std::vector<cv::Point2f> &contour, bool isSubpixel = true);
    void drawContour(QGraphicsScene *scene, const std::vector<cv::Point> &contour, bool isSubpixel = false);

    Ui::ImageViewWindow *ui;
    QThread readThread;
    QThread processThread;
    ImageReadWorker *readWorker;
    ImageProcessWorker *processWorker;
};

#endif // IMAGEVIEWWINDOW_H

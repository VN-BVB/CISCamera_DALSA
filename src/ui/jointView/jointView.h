#ifndef JOINTVIEW_H
#define JOINTVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QThread>

#include "src/jointDetection/ImagereadWorker.h"
#include "src/jointDetection/ImageProcessWorker.h"
#include "src/jointDetection/edgeDetection/CannyDevernay.h"
#include "src/ui/utils/imageWidget/frmVisionDisplay.h"

namespace Ui {
class JointView;
}

class JointView : public QWidget
{
    Q_OBJECT

public:
    explicit JointView(QWidget *parent = nullptr);
    ~JointView();

signals:
    void startImageRead(const QString &path);
    void startImageProcess(std::shared_ptr<cv::Mat> image);

private slots:
    void on_pb_open_clicked();
    void handleImageRead(std::shared_ptr<cv::Mat> image);
    void handleImageProcessed(std::shared_ptr<cv::Mat> processedImage,
                              std::vector<std::vector<cv::Point2f>> subpixelContours,
                              std::vector<std::vector<cv::Point>> pixelContour,
                              std::vector<cv::Vec4f> lines);
    void handleImageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves);
    void handleError(const QString &error);

private:
    Ui::JointView *ui;
    QThread readThread;
    QThread processThread;
    ImageReadWorker *readWorker;
    ImageProcessWorker *processWorker;

    // @TODO:使用日志记录每个步骤处理时间
    std::chrono::high_resolution_clock::time_point startTime;   // 图像处理开始时间
};

#endif // JOINTVIEW_H

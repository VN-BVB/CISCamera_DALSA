#ifndef IMAGE_PROCESS_WORKER_H
#define IMAGE_PROCESS_WORKER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "edgeDetection/canny_devernay.h"
#include "contourProcess/curve_seg.h"
#include "contourProcess/joint_seam.h"


class ImageProcessWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessWorker(QObject *parent = nullptr);

public slots:
    void processImage(std::shared_ptr<cv::Mat> image);

signals:
    void imageProcessed(std::shared_ptr<cv::Mat> processedImage,
                        std::vector<std::vector<cv::Point2f>> subpixelContours,
                        std::vector<std::vector<cv::Point>> pixelContour,
                        std::vector<cv::Vec4f> lines,
                        std::vector<CurveSeg> curves);
    void imageProcessed(std::shared_ptr<cv::Mat> processedImage,
                        std::shared_ptr<JointSeam> jointSeam);
    void imageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves);
    void errorOccurred(const QString &error);

private:
    // 去除轮廓两端的一部分
    std::vector<cv::Point2f> trimContourEnds(const std::vector<cv::Point2f>& contour, float trimRatio);

};

#endif // IMAGE_PROCESS_WORKER_H

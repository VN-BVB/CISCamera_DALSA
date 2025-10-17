#ifndef IMAGEPROCESSWORKER_H
#define IMAGEPROCESSWORKER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "edgeDetection/CannyDevernay.h"


class ImageProcessWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessWorker(QObject *parent = nullptr);

public slots:
    void processImage(std::shared_ptr<cv::Mat> image);

signals:
    void imageProcessed(std::shared_ptr<cv::Mat> processedImage, std::vector<std::vector<cv::Point2f>> subpixelContours,
                        std::vector<std::vector<cv::Point>> pixelContour); // openCV的亚像素坐标
    void imageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves);
    void errorOccurred(const QString &error);

private:
    double adaptiveCannyThresholdByOtsu(const cv::Mat &srcImage);
    cv::Point2f zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius);
    std::vector<cv::Point2f> getSubpixelContourZernike(const cv::Mat &src,
                                                       const std::vector<cv::Point> &contour);
    // 根据多边形拟合点分割轮廓
    std::vector<std::vector<cv::Point2f>> segmentContourByApproxPoints(
        const std::vector<cv::Point2f>& contour,
        const std::vector<cv::Point2f>& approxPoints);
    // 直线拟合
    cv::Vec4f fitLineToPoints(const std::vector<cv::Point2f>& points);
    // 计算两条直线的交点
    cv::Point2f calculateLineIntersection(const cv::Vec4f& line1, const cv::Vec4f& line2);


    std::vector<std::vector<cv::Point2f>> m_subpixelContours;
    std::vector<std::vector<std::vector<cv::Point2f>>> m_segmentedContours; // 存储分割后的轮廓段
    cv::Point2f m_intersectionPoint; // 存储交点坐标
};

#endif // IMAGEPROCESSWORKER_H

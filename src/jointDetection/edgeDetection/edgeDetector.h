#ifndef EDGEDETECTOR_H
#define EDGEDETECTOR_H

#include <opencv2/opencv.hpp>

class EdgeDetector
{
public:
    EdgeDetector();
    // Zernike矩法辅助函数
    cv::Point2f zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius);
    // Zernike矩法获取亚像素点
    std::vector<cv::Point2f> getSubpixelContourZernike(const cv::Mat &src, const std::vector<cv::Point> &contour);
    // 获取Canny自适应阈值
    double adaptiveCannyThresholdByOtsu(const cv::Mat &srcImage);
};

#endif // EDGEDETECTOR_H

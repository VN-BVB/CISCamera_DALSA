#ifndef EDGE_DETECTOR_H
#define EDGE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include "src/utils/geometry_utils.h"

class EdgeDetector
{
public:
    EdgeDetector(cv::Mat image);
    // Zernike矩法辅助函数
    cv::Point2f zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius);
    // Zernike矩法获取亚像素点
    std::vector<cv::Point2f> getSubpixelContourZernike(const cv::Mat &src, const std::vector<cv::Point> &contour);
    // 获取Canny自适应阈值
    double adaptiveCannyThresholdByOtsu(const cv::Mat &srcImage);
    // 计算中间缝隙中心线（中轴变换 + RANSAC）
    cv::Vec4f calculateCenterLineBySkeletonAndRANSAC(const cv::Mat& image);
    // 根据中心线将轮廓分类到两侧
    std::pair<std::vector<std::vector<cv::Point>>, std::vector<std::vector<cv::Point>>>
    classifyContoursByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine);
    std::vector<std::vector<cv::Point>>
    classifyContourPointsByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine);
    // 执行拼缝两边轮廓检测
    std::vector<std::vector<cv::Point>> run();
private:
    cv::Mat m_image;
};

#endif // EDGE_DETECTOR_H

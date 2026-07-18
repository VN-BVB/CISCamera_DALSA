#ifndef ABSTRACT_CONTOUR_DETECTOR_H
#define ABSTRACT_CONTOUR_DETECTOR_H

#include <opencv2/opencv.hpp>

// 轮廓检测结果：轮廓点集 + 本次检测是否为碰撞情况
struct ContourDetectionResult {
    std::vector<std::vector<cv::Point2f>> contours;
    bool isCollision = false;
    cv::Vec4f centerLine{};   // 缝隙中心线 (vx, vy, x0, y0)
};

/**
 * @brief 策略模式---抽象轮廓检测器基类，定义轮廓检测的统一接口
 * @details 所有轮廓检测算法提供统一的抽象接口，包括轮廓检测和算法描述功能
 */
class AbstractContourDetector
{
public:
    AbstractContourDetector();
    virtual ~AbstractContourDetector() = default;

    // 边缘检测接口
    virtual ContourDetectionResult detectContours(const cv::Mat& inputImage) = 0;

    // 获取算法描述
    virtual std::string getDescription() const = 0;
};

#endif // ABSTRACT_CONTOUR_DETECTOR_H

#ifndef ABSTRACT_CONTOUR_DETECTOR_H
#define ABSTRACT_CONTOUR_DETECTOR_H

#include <opencv2/opencv.hpp>

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
    virtual std::vector<std::vector<cv::Point2f>> detectContours(const cv::Mat& inputImage) = 0;

    // 获取算法描述
    virtual std::string getDescription() const = 0;
};

#endif // ABSTRACT_CONTOUR_DETECTOR_H

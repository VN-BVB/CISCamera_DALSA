#ifndef ABSTRACT_CONTOUR_DETECTOR_H
#define ABSTRACT_CONTOUR_DETECTOR_H

#include <opencv2/opencv.hpp>

// @TODO:利用策略模式实现轮廓检测，便于切换算法
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

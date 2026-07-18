#ifndef CONTOUR_DETECTOR_CONTEXT_H
#define CONTOUR_DETECTOR_CONTEXT_H

#include "abstract_contour_detector.h"
#include "canny_zernike_detector.h"

/**
 * @brief 轮廓检测器上下文类，实现策略模式管理不同的轮廓检测算法
 * @details 该类作为策略模式的上下文，负责管理和切换不同的轮廓检测策略
 */
class ContourDetectorContext
{
public:
    ContourDetectorContext();
    ~ContourDetectorContext();

    // 设置当前使用的策略
    void setDetector(std::unique_ptr<AbstractContourDetector> detector);

    // 执行边缘轮廓检测
    ContourDetectionResult detectContours(const cv::Mat &inputImage);

    // 获取当前策略描述
    std::string getCurrentDescription() const;
private:
    std::unique_ptr<AbstractContourDetector> m_currentDetector;
};

#endif // CONTOUR_DETECTOR_CONTEXT_H

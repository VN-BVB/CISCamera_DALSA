#include "contour_detector_context.h"

ContourDetectorContext::ContourDetectorContext() = default;

ContourDetectorContext::~ContourDetectorContext() {}

// 设置当前使用的策略
void ContourDetectorContext::setDetector(std::unique_ptr<AbstractContourDetector> detector)
{
    m_currentDetector = std::move(detector);
}

// 执行边缘轮廓检测
std::vector<std::vector<cv::Point2f>> ContourDetectorContext::detectContours(const cv::Mat &inputImage)
{
    if (!m_currentDetector) {
        throw std::runtime_error("No detector strategy set");
    }
    return m_currentDetector->detectContours(inputImage);
}

// 获取当前策略描述
std::string ContourDetectorContext::getCurrentDescription() const
{
    return m_currentDetector ? m_currentDetector->getDescription() : "No detector selected";
}

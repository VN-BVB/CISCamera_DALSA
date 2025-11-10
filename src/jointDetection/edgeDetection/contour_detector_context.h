#ifndef CONTOUR_DETECTOR_CONTEXT_H
#define CONTOUR_DETECTOR_CONTEXT_H

#include "abstract_contour_detector.h"
#include "canny_zernike_detector.h"

class ContourDetectorContext
{
public:
    ContourDetectorContext();
    ~ContourDetectorContext();

    // 设置当前使用的策略
    void setDetector(std::unique_ptr<AbstractContourDetector> detector);

    // 执行边缘轮廓检测
    std::vector<std::vector<cv::Point2f>> detectContours(const cv::Mat &inputImage);

    // 获取当前策略描述
    std::string getCurrentDescription() const;
private:
    std::unique_ptr<AbstractContourDetector> m_currentDetector;
};

#endif // CONTOUR_DETECTOR_CONTEXT_H

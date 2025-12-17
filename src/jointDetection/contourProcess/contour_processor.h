#ifndef CONTOUR_PROCESSOR_H
#define CONTOUR_PROCESSOR_H

#include "contour_processing_pipeline.h"
#include "methods/contour_data.h"
#include <memory>

/**
 * @brief 轮廓处理器 - 使用策略模式
 */
class ContourProcessor {
public:
    ContourProcessor();

    // 处理轮廓
    bool processContour(const std::vector<cv::Point2f>& contour);

    // 获取处理结果
    const ContourData& getResult() const { return m_contourData; }

    // 获取处理摘要
    std::string getSummary() const;

    // 结果获取方法
    std::vector<cv::Point2f> getSortedContour() const { return m_contourData.getSortedContour(); }
    std::vector<cv::Point2f> getCornerPoints() const { return m_contourData.getCornerPoints(); }
    std::map<int, LineSeg> getLineSegments() const { return m_contourData.getLineSegments(); }
    std::map<int, CurveSeg> getCurveSegments() const { return m_contourData.getCurveSegments(); }
    std::vector<cv::Point2f> getIntersections() const { return m_contourData.getIntersections(); }
    std::vector<cv::Vec4f> getTangentLines() const { return m_contourData.getTangentLines(); }

private:
    ContourData m_contourData;
    std::unique_ptr<ContourProcessingPipeline> m_pipeline;

    void initializePipeline();
};

#endif // CONTOUR_PROCESSOR_H

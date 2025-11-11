#ifndef CONTOUR_PROCESSOR_H
#define CONTOUR_PROCESSOR_H

#include "contour_processing_pipeline.h"
#include "methods/contour_data.h"
#include <memory>

/**
 * @brief 新版轮廓处理器 - 使用策略模式
 */
class ContourProcessorV2 {
public:
    ContourProcessorV2();

    /**
     * @brief 处理轮廓
     */
    bool processContour(const std::vector<cv::Point2f>& contour);

    /**
     * @brief 获取处理结果
     */
    const ContourData& getResult() const { return m_data; }

    /**
     * @brief 获取处理摘要
     */
    std::string getSummary() const;

    // 结果获取方法
    std::vector<cv::Point2f> getSortedContour() const { return m_data.getSortedContour(); }
    std::vector<cv::Point2f> getCornerPoints() const { return m_data.getCornerPoints(); }
    std::map<int, LineSeg> getLineSegments() const { return m_data.getLineSegments(); }
    std::map<int, CurveSeg> getCurveSegments() const { return m_data.getCurveSegments(); }
    std::vector<cv::Point2f> getEndPoints() const { return m_data.getEndPoints(); }
    std::vector<cv::Vec4f> getTangentLines() const { return m_data.getTangentLines(); }

private:
    ContourData m_data;
    std::unique_ptr<ContourProcessingPipeline> m_pipeline;

    void initializePipeline();
};

#endif // CONTOUR_PROCESSOR_H

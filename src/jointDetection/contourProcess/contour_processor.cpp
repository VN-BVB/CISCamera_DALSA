#include "contour_processor.h"
#include "deduplication_strategy.h"
#include "direction_calculation_strategy.h"
#include "sorting_strategy.h"
#include "corner_detection_strategy.h"
#include "segmentation_strategy.h"
#include "curve_fitting_strategy.h"

ContourProcessorV2::ContourProcessorV2() {
    m_pipeline = std::make_unique<ContourProcessingPipeline>();
    initializePipeline();
}

void ContourProcessorV2::initializePipeline() {
    // 配置处理管道
    m_pipeline->addStrategy(std::make_shared<DeduplicationStrategy>());                 // 轮廓点去重
    m_pipeline->addStrategy(std::make_shared<DirectionCalculationStrategy>());          // 开口方向计算
    m_pipeline->addStrategy(std::make_shared<SortingStrategy>());                       // 逆时针排序点
    m_pipeline->addStrategy(std::make_shared<CornerDetectionStrategy>());               // 角点计算
    m_pipeline->addStrategy(std::make_shared<SegmentationStrategy>());                  // 分割轮廓
    m_pipeline->addStrategy(std::make_shared<CurveFittingStrategy>());                  // 拟合分割后轮廓
}

bool ContourProcessorV2::processContour(const std::vector<cv::Point2f>& contour) {
    m_data.clear();
    m_data.setSubpixelContour(contour);
    return m_pipeline->process(m_data);
}

std::string ContourProcessorV2::getSummary() const {
    return m_pipeline->getSummary(m_data);
}

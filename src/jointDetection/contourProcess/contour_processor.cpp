#include "contour_processor.h"
#include "strategies/deduplication_strategy.h"
#include "strategies/direction_calculation_strategy.h"
#include "strategies/sorting_strategy.h"
#include "strategies/corner_detection_strategy.h"
#include "strategies/segmentation_strategy.h"
#include "strategies/curve_fitting_strategy.h"
#include "src/utils/scoped_timer.h"

ContourProcessor::ContourProcessor() {
    m_pipeline = std::make_unique<ContourProcessingPipeline>();
    initializePipeline();
}

void ContourProcessor::initializePipeline() {
    // 配置处理管道
    m_pipeline->addStrategy(std::make_shared<DeduplicationStrategy>());                 // 轮廓点去重
    m_pipeline->addStrategy(std::make_shared<DirectionCalculationStrategy>());          // 开口方向计算
    m_pipeline->addStrategy(std::make_shared<SortingStrategy>());                       // 逆时针排序点
    m_pipeline->addStrategy(std::make_shared<CornerDetectionStrategy>());               // 角点计算
    m_pipeline->addStrategy(std::make_shared<SegmentationStrategy>());                  // 分割轮廓
    m_pipeline->addStrategy(std::make_shared<CurveFittingStrategy>());                  // 拟合分割后轮廓
}

bool ContourProcessor::processContour(const std::vector<cv::Point2f>& contour, int contourId, bool isCollision) {
    SCOPED_TIMER("处理单条轮廓");
    m_contourData.clear();
    m_contourData.setSubpixelContour(contour);
    m_contourData.setId(contourId);
    m_contourData.setIsCollision(isCollision);
    return m_pipeline->process(m_contourData);
}

std::string ContourProcessor::getSummary() const {
    return m_pipeline->getSummary(m_contourData);
}

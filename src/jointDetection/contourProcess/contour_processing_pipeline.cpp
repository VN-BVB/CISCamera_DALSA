#include "contour_processing_pipeline.h"
#include <sstream>

ContourProcessingPipeline::ContourProcessingPipeline() {}

void ContourProcessingPipeline::addStrategy(ContourProcessingStrategyPtr strategy) {
    m_strategies.push_back(strategy);
}

void ContourProcessingPipeline::setStrategies(const std::vector<ContourProcessingStrategyPtr>& strategies) {
    m_strategies = strategies;
}

bool ContourProcessingPipeline::process(ContourData& context) {
    for (auto& strategy : m_strategies) {
        if (!strategy->process(context)) {
            return false;
        }
    }
    return true;
}

std::string ContourProcessingPipeline::getSummary(const ContourData& context) const {
    std::stringstream ss;
    ss << "轮廓处理结果摘要:\n";
    ss << "轮廓点数: " << context.getSubpixelContour().size() << "\n";
    cv::Point2f dir = context.getOpeningDirection();
    float angleDeg = std::atan2(dir.y, dir.x) * 180.0f / static_cast<float>(CV_PI);
    ss << "开口方向: (" << dir.x << ", " << dir.y << ")  角度=" << angleDeg << "°\n";
    ss << "角点数: " << context.getCornerPoints().size() << "\n";
    ss << "分割段数: " << context.getSegmentedContours().size() << "\n";
    ss << "曲线段数: " << context.getCurveSegments().size() << "\n";
    ss << "直线段数: " << context.getLineSegments().size() << "\n";
    return ss.str();
}

void ContourProcessingPipeline::clearStrategies() {
    m_strategies.clear();
}

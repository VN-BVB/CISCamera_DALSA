#include "contour_processor.h"
#include "contour_feature_calculator.h"
#include "contour_segmenter.h"
#include "contour_fitter.h"

ContourProcessor::ContourProcessor() {}

void ContourProcessor::processContour(const std::vector<cv::Point2f>& contour) {
    // 1. 数据准备
    m_data.setSubpixelContour(contour);

    // 2. 去重
    auto deduplicated = ContourFeatureCalculator::removeDuplicatePoints(contour);
    m_data.setSubpixelContour(deduplicated);

    // 3. 计算开口方向
    auto direction = ContourFeatureCalculator::calculateOpeningDirection(deduplicated);
    m_data.setOpeningDirection(direction);

    // 4. 排序轮廓（由起始点逆时针）
    auto startPoint = ContourFeatureCalculator::calculateStartPoint(direction, deduplicated);
    int startIndex = ContourUtils::findPointIndex(startPoint, deduplicated);
    auto sortedContour = ContourFeatureCalculator::sortContour(deduplicated, startIndex);
    m_data.setSortedContour(sortedContour);

    // 5. 检测角点
    auto cornerPoints = ContourFeatureCalculator::detectCornerPoints(sortedContour);
    m_data.setCornerPoints(cornerPoints);

    // 6. 移除角点附近点
    auto filteredContour = ContourFeatureCalculator::removePointsNearCorners(sortedContour, cornerPoints);

    // 7. 分割轮廓
    auto segments = ContourSegmenter::segmentContour(filteredContour);
    m_data.setSegmentedContours(segments);

    // 8. 排序分割轮廓
    auto centroid = ContourUtils::calculateCentroid(filteredContour);
    auto sortedSegments = ContourSegmenter::sortSegmentsCounterClockwise(segments, centroid);
    m_data.setSortedSegments(sortedSegments);

    // 9. 拟合曲线
    m_curveSegments = ContourFitter::fitCurvesToSegments(sortedSegments);

    // 10. 计算端点
    m_endPoints = ContourFitter::calculateEndPoints(m_curveSegments, centroid, m_lines);

    // 11. 拟合直线（可选）
    m_lineSegments = ContourFitter::fitLinesToSegments(sortedSegments);
}

std::string ContourProcessor::getSummary() const {
    std::string summary;
    summary += "轮廓点数: " + std::to_string(m_data.getSubpixelContour().size()) + "\n";
    summary += "开口方向: " + ContourUtils::openingDirectionToString(m_data.getOpeningDirection()) + "\n";
    summary += "角点数: " + std::to_string(m_data.getCornerPoints().size()) + "\n";
    summary += "分割段数: " + std::to_string(m_data.getSegmentedContours().size()) + "\n";
    summary += "曲线段数: " + std::to_string(m_curveSegments.size()) + "\n";
    return summary;
}



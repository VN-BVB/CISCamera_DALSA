#include "curve_fitting_strategy.h"

CurveFittingStrategy::CurveFittingStrategy() {}

bool CurveFittingStrategy::process(ContourData& context) {
    auto sortedSegments = context.getSortedSegments();

    // 拟合曲线
    auto curveSegments = ContourFitter::fitCurvesToSegments(sortedSegments);

    // 计算拼缝端点
    auto centroid = ContourUtils::calculateCentralPoint(context.getSortedContour());
    std::vector<cv::Vec4f> tangentLines;
    std::vector<cv::Point2f> endPoints;
    ContourFitter::calculateEndPoints(curveSegments, centroid, endPoints, tangentLines);

    // 将端点转换为结构化的交点信息并分配ID
    std::vector<ContourIntersection> contourIntersections;
    for (size_t i = 0; i < endPoints.size(); ++i) {
        int endpointId = context.getId() * 2 + static_cast<int>(i);
        contourIntersections.emplace_back(endpointId, endPoints[i], context.getId());
    }

    // 存储结果到上下文
    context.setCurveSegments(curveSegments);
    context.setIntersections(contourIntersections);
    context.setTangentLines(tangentLines);

    return true;
}

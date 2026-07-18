#include "curve_fitting_strategy.h"

CurveFittingStrategy::CurveFittingStrategy() {}

bool CurveFittingStrategy::process(ContourData& context) {
    auto sortedSegments = context.getSortedSegments();
    std::vector<cv::Vec4f> tangentLines;
    std::vector<cv::Point2f> endPoints;

    if (context.isCollision()) {
        // 碰撞：跳过曲线拟合，直接对原始轮廓段拟合直线并与中心线求交
        ContourFitter::calculateEndPointsFromCenterLine(
            sortedSegments, endPoints, tangentLines, context.getCenterLine());
    } else {
        // 非碰撞：样条拟合 + 现有端点逻辑
        auto curveSegments = ContourFitter::fitCurvesToSegments(sortedSegments);
        auto centroid = ContourUtils::calculateCentralPoint(context.getSortedContour());
        ContourFitter::calculateEndPoints(curveSegments, centroid, endPoints, tangentLines);
        context.setCurveSegments(curveSegments);
    }

    // 将端点转换为结构化的交点信息并分配ID
    std::vector<ContourIntersection> contourIntersections;
    for (size_t i = 0; i < endPoints.size(); ++i) {
        int endpointId = context.getId() * 2 + static_cast<int>(i);
        contourIntersections.emplace_back(endpointId, endPoints[i], context.getId());
    }

    // 存储结果到上下文
    context.setIntersections(contourIntersections);
    context.setTangentLines(tangentLines);

    return true;
}

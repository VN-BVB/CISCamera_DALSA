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

    // 存储结果到上下文
    context.setCurveSegments(curveSegments);
    context.setIntersections(endPoints);
    context.setTangentLines(tangentLines);

    return true;
}

#include "curve_fitting_strategy.h"

CurveFittingStrategy::CurveFittingStrategy() {}

bool CurveFittingStrategy::process(ContourData& context) {
    auto sortedSegments = context.getSortedSegments();

    // 拟合曲线
    auto curveSegments = ContourFitter::fitCurvesToSegments(sortedSegments);

    // 计算端点
    auto centroid = ContourUtils::calculateCentroid(context.getSortedContour());
    std::vector<cv::Vec4f> tangentLines;
    auto endPoints = ContourFitter::calculateEndPoints(curveSegments, centroid, tangentLines);

    // 存储结果到上下文
    context.setCurveSegments(curveSegments);
    context.setEndPoints(endPoints);
    context.setTangentLines(tangentLines);

    return true;
}

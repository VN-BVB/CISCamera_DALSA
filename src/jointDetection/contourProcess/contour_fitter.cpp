#include "contour_fitter.h"

std::map<int, LineSeg> ContourFitter::fitLinesToSegments(const std::map<int, std::vector<cv::Point2f>>& segments) {
    std::map<int, LineSeg> lineSegments;
    for (const auto& [index, contour] : segments) {
        LineSeg ls;
        ls.initializeFromPoints(contour);
        lineSegments[index] = ls;
    }
    return lineSegments;
}

std::map<int, CurveSeg> ContourFitter::fitCurvesToSegments(const std::map<int, std::vector<cv::Point2f>>& segments) {
    std::map<int, CurveSeg> curveSegments;
    for (const auto& [index, contour] : segments) {
        CurveSeg curve;
        curve.initializeFromPoints(contour);
        curve.fitSplineCurve();
        curveSegments[index] = curve;
    }
    return curveSegments;
}

std::vector<cv::Point2f> ContourFitter::calculateEndPoints(const std::map<int, CurveSeg>& curveSegments) {
    std::vector<cv::Point2f> endPoints;
    // 这里实现端点计算逻辑
    // 简化的实现，实际需要根据你的具体需求
    for (const auto& [index, curve] : curveSegments) {
        // 获取曲线的端点
        // endPoints.push_back(...);
    }
    return endPoints;
}

cv::Point2f ContourFitter::calculateLineIntersection(const cv::Vec4f& line1, const cv::Vec4f& line2) {
    float vx1 = line1[0], vy1 = line1[1], x01 = line1[2], y01 = line1[3];
    float vx2 = line2[0], vy2 = line2[1], x02 = line2[2], y02 = line2[3];

    // 计算交点
    float denominator = vx1 * vy2 - vy1 * vx2;
    if (std::abs(denominator) < 1e-10) {
        return cv::Point2f(-1, -1); // 平行线
    }

    float t = ((x02 - x01) * vy2 - (y02 - y01) * vx2) / denominator;
    float x = x01 + t * vx1;
    float y = y01 + t * vy1;

    return cv::Point2f(x, y);
}


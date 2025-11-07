#include "contour_utils.h"

// ==================== 工具函数实现 ====================
std::string ContourUtils::openingDirectionToString(OpeningDirection direction) {
    switch (direction) {
    case OpeningDirection::UNKNOWN: return "未知";
    case OpeningDirection::UP: return "向上";
    case OpeningDirection::DOWN: return "向下";
    case OpeningDirection::LEFT: return "向左";
    case OpeningDirection::RIGHT: return "向右";
    default: return "未知";
    }
}

int ContourUtils::findPointIndex(const cv::Point2f& point, const std::vector<cv::Point2f>& contour, float tolerance) {
    for (int i = 0; i < contour.size(); ++i) {
        if (std::abs(contour[i].x - point.x) < tolerance && std::abs(contour[i].y - point.y) < tolerance) {
            return i;
        }
    }
    return -1;
}

cv::Point2f ContourUtils::calculateCentroid(const std::vector<cv::Point2f>& contour) {
    if (contour.empty()) return cv::Point2f(0, 0);

    // 计算质心
    cv::Point2f centroid;
    cv::Moments moments = cv::moments(contour);
    if (moments.m00 != 0) {
        centroid.x = static_cast<int>(moments.m10 / moments.m00);
        centroid.y = static_cast<int>(moments.m01 / moments.m00);
    }
    return centroid;
}

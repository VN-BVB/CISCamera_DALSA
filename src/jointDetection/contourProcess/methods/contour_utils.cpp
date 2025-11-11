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

    // 计算轮廓的最小外接矩形
    cv::Rect boundingRect = cv::boundingRect(contour);

    // 计算矩形的中心点
    cv::Point2f centroid;
    centroid.x = boundingRect.x + boundingRect.width / 2.0f;
    centroid.y = boundingRect.y + boundingRect.height / 2.0f;

    return centroid;
}

/**
* @brief 判断点A是否在点B的顺时针方向（相对于参考点）
* @param pointA 第一个点
* @param pointB 第二个点
* @param referencePoint 参考点
* @return 如果点A在点B的顺时针方向返回true，否则返回false
*/
bool ContourUtils::isPointClockwiseTo(const cv::Point2f& pointA, const cv::Point2f& pointB, const cv::Point2f& referencePoint)
{

    // 将参考点作为原点，计算相对坐标
    cv::Point2f relA = pointA - referencePoint;
    cv::Point2f relB = pointB - referencePoint;

    // 计算叉积 det = (ax * by - ay * bx)
    float det = relA.x * relB.y - relA.y * relB.x;

    // 如果叉积为正，b在a顺时针方向
    if (det > 0)
        return false;

    // 如果叉积为负，a在b顺时针方向
    if (det < 0)
        return true;

    // 叉积为0，共线情况，按距离排序（距离小的在顺时针方向）
    float d1 = relA.x * relA.x + relA.y * relA.y;
    float d2 = relB.x * relB.x + relB.y * relB.y;
    return d1 < d2;
}

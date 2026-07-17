#include "contour_utils.h"

namespace ContourUtils {
// ==================== 工具函数实现 ====================
std::string openingDirectionToString(OpeningDirection direction) {
    switch (direction) {
    case OpeningDirection::UNKNOWN: return "未知";
    case OpeningDirection::UP: return "向上";
    case OpeningDirection::DOWN: return "向下";
    case OpeningDirection::LEFT: return "向左";
    case OpeningDirection::RIGHT: return "向右";
    default: return "未知";
    }
}

/**
 * @brief 在轮廓中查找指定点的索引位置
 * @param point 要查找的目标点
 * @param contour 轮廓点集
 * @param tolerance 容差范围，用于浮点数比较
 * @return int 目标点在轮廓中的索引位置，如果未找到返回-1
 */
int findPointIndex(const cv::Point2f& point, const std::vector<cv::Point2f>& contour, float tolerance) {
    for (int i = 0; i < contour.size(); ++i) {
        if (std::abs(contour[i].x - point.x) < tolerance && std::abs(contour[i].y - point.y) < tolerance) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 计算轮廓的中心点坐标
 * @param contour 输入轮廓点集
 * @return cv::Point2f 轮廓的中心点坐标，如果轮廓为空返回(0,0)
 */
cv::Point2f calculateCentralPoint(const std::vector<cv::Point2f>& contour) {
    if (contour.empty()) return cv::Point2f(0, 0);

    // 计算轮廓的最小外接矩形
    cv::Rect boundingRect = cv::boundingRect(contour);

    // 计算矩形的中心点
    cv::Point2f centroid;
    centroid.x = boundingRect.x + boundingRect.width / 2.0f;
    centroid.y = boundingRect.y + boundingRect.height / 2.0f;

    return centroid;
}

// 半径滤波（Radius Outlier Removal）
std::vector<cv::Point2f> radiusOutlierRemoval(const std::vector<cv::Point2f>& points, float radius, int minPts) {
    std::vector<cv::Point2f> filteredPoints;

    // 如果输入点集为空或参数无效，直接返回空结果
    if (points.empty() || radius <= 0 || minPts <= 0) {
        return filteredPoints;
    }

    // 对每个点计算其在指定半径内的邻居数量
    for (size_t i = 0; i < points.size(); ++i) {
        int neighborCount = 0;

        // 遍历所有其他点，计算距离在半径内的点数
        for (size_t j = 0; j < points.size(); ++j) {
            if (i != j) {
                // 计算两点之间的欧氏距离
                float dx = points[i].x - points[j].x;
                float dy = points[i].y - points[j].y;
                float distance = std::sqrt(dx * dx + dy * dy);

                // 如果距离小于等于半径，增加邻居计数
                if (distance <= radius) {
                    neighborCount++;
                }
            }
        }

        // 如果邻居数量大于等于最小邻居数，则保留该点
        if (neighborCount >= minPts) {
            filteredPoints.push_back(points[i]);
        }
    }

    return filteredPoints;
}
}// namespace ContourUtils



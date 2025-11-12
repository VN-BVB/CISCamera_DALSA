#include "geometry_utils.h"

namespace GeometryUtils {

// 计算两个向量的叉积
float crossProduct(const cv::Point2f& a, const cv::Point2f& b) {
    return a.x * b.y - a.y * b.x;
}

// 判断点是否在线段上
bool isPointOnSegment(const cv::Point2f& p, const cv::Point2f& a, const cv::Point2f& b) {
    // 检查点p是否在线段ab的边界框内
    if (p.x < std::min(a.x, b.x) || p.x > std::max(a.x, b.x) ||
        p.y < std::min(a.y, b.y) || p.y > std::max(a.y, b.y)) {
        return false;
    }

    // 检查点p是否在直线ab上（叉积为0表示共线）
    float cross = crossProduct(b - a, p - a);
    return std::abs(cross) < 1e-6;
}

// 判断两条线段是否相交
bool doSegmentsIntersect(const cv::Point2f& p1, const cv::Point2f& p2,
                         const cv::Point2f& q1, const cv::Point2f& q2) {
    // 使用快速排斥实验和跨立实验判断线段相交

    // 快速排斥实验：检查两个线段的边界框是否相交
    if (std::max(p1.x, p2.x) < std::min(q1.x, q2.x) ||
        std::max(q1.x, q2.x) < std::min(p1.x, p2.x) ||
        std::max(p1.y, p2.y) < std::min(q1.y, q2.y) ||
        std::max(q1.y, q2.y) < std::min(p1.y, p2.y)) {
        return false;
    }

    // 跨立实验：检查点q1和q2是否在线段p1p2的两侧
    float cross1 = crossProduct(p2 - p1, q1 - p1);
    float cross2 = crossProduct(p2 - p1, q2 - p1);

    // 检查点p1和p2是否在线段q1q2的两侧
    float cross3 = crossProduct(q2 - q1, p1 - q1);
    float cross4 = crossProduct(q2 - q1, p2 - q1);

    // 如果两个叉积的乘积小于等于0，说明线段相交
    if (cross1 * cross2 <= 0 && cross3 * cross4 <= 0) {
        return true;
    }

    // 检查端点重合的情况
    if (isPointOnSegment(p1, q1, q2) || isPointOnSegment(p2, q1, q2) ||
        isPointOnSegment(q1, p1, p2) || isPointOnSegment(q2, p1, p2)) {
        return true;
    }

    return false;
}

// 判断点是否在旋转矩形内
bool isPointInRotatedRect(const cv::Point2f& point, const cv::RotatedRect& rotatedRect) {
    // 获取旋转矩形的四个角点
    cv::Point2f vertices[4];
    rotatedRect.points(vertices);

    // 计算点到四条边的向量积
    // 如果点都在四条边的同一侧（内部），则点在矩形内
    for (int i = 0; i < 4; i++) {
        cv::Point2f edge = vertices[(i + 1) % 4] - vertices[i];
        cv::Point2f pointToVertex = point - vertices[i];

        // 计算叉积（向量积）
        float crossProduct = edge.x * pointToVertex.y - edge.y * pointToVertex.x;

        // 如果叉积为负，说明点在边的右侧（外部）
        if (crossProduct < 0) {
            return false;
        }
    }

    return true;
}

} // namespace GeometryUtils


#include "geometry_utils.h"

#include <plog/Log.h>

#include <iostream>

#include "src/config/calibration_data_io.h"
#include "src/telecentricLineCalibrator/libcbdetect/lib_cb_detecor.h"
#include "src/ui/CISCamera_imageGrab/cameraImage_processor.h"

namespace GeometryUtils {

// 计算两个向量的叉积
float crossProduct(const cv::Point2f& a, const cv::Point2f& b) { return a.x * b.y - a.y * b.x; }

// 判断点是否在线段上
bool isPointOnSegment(const cv::Point2f& p, const cv::Point2f& a, const cv::Point2f& b) {
    // 检查点p是否在线段ab的边界框内
    if (p.x < std::min(a.x, b.x) || p.x > std::max(a.x, b.x) || p.y < std::min(a.y, b.y) || p.y > std::max(a.y, b.y)) {
        return false;
    }

    // 检查点p是否在直线ab上（叉积为0表示共线）
    float cross = crossProduct(b - a, p - a);
    return std::abs(cross) < 1e-6;
}

// 判断两条线段是否相交
bool doSegmentsIntersect(const cv::Point2f& p1, const cv::Point2f& p2, const cv::Point2f& q1, const cv::Point2f& q2) {
    // 使用快速排斥实验和跨立实验判断线段相交

    // 快速排斥实验：检查两个线段的边界框是否相交
    if (std::max(p1.x, p2.x) < std::min(q1.x, q2.x) || std::max(q1.x, q2.x) < std::min(p1.x, p2.x) || std::max(p1.y, p2.y) < std::min(q1.y, q2.y) ||
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
    if (isPointOnSegment(p1, q1, q2) || isPointOnSegment(p2, q1, q2) || isPointOnSegment(q1, p1, p2) || isPointOnSegment(q2, p1, p2)) {
        return true;
    }

    return false;
}

bool isPolygonSelfIntersecting(const std::vector<cv::Point2f>& polygon) {
    int M = static_cast<int>(polygon.size());
    if (M < 3) {
        return false;  // 少于3个点无法构成多边形，视为不自交
    }

    for (int i = 0; i < M; ++i) {
        for (int j = i + 1; j < M; ++j) {
            // 跳过相邻边
            if (j == i + 1) continue;
            // 跳过首尾相邻边
            if (i == 0 && j == M - 1) continue;

            // 边 i: polygon[i] -> polygon[(i + 1) % M]
            // 边 j: polygon[j] -> polygon[(j + 1) % M]
            if (doSegmentsIntersect(polygon[i], polygon[(i + 1) % M], polygon[j], polygon[(j + 1) % M])) {
                return true;
            }
        }
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

/**
 * @brief ImageProcessing_lineDetection     直线拟合Ransac
 * @param points                            输入亚像素点集
 * @param line                              输出直线参数(vx, vy, x0, y0), (vx, vy) 为方向向量, (x0, y0) 为直线上的一个点
 * @param inlierPoints                      输出直线内点
 * @param threshold                         阈值
 * @param iterations                        最大迭代次数
 */
void lineRansac(const std::vector<cv::Point2f>& points, cv::Vec4f& line, std::vector<cv::Point2f>& inlierPoints, const double& threshold,
                const int& iterations) {
    if (points.size() < 2) {
        PLOG_INFO << "Input points is empty!" << std::endl;
        return;
    }

    cv::RNG rng;  // 创建随机数生成器
    double bestScore = -1.;
    auto n = points.size();  // 获取点集大小
    for (int iter = 0; iter < iterations; iter++) {
        // 随机选择两个不同的点
        auto i1 = rng.uniform(0, static_cast<int>(n - 1));
        auto i2 = rng.uniform(0, static_cast<int>(n - 1));
        if (i1 == i2) continue;

        // 直线的方向向量
        const cv::Point2f& p1 = points[i1];
        const cv::Point2f& p2 = points[i2];
        cv::Point2f dp = p2 - p1;
        dp *= 1.0 / cv::norm(dp);

        // 计算内点
        double score = 0;
        std::vector<cv::Point2f> inliers;
        for (int i = 0; i < n; i++) {
            cv::Point2f v = points[i] - p1;
            double d = v.y * dp.x - v.x * dp.y;  // 向量a与b叉乘/向量b的摸.||b||=1./norm(dp)
            // 判断点到直线的距离是否小于阈值
            if (std::fabs(d) < threshold) {
                score += 1;
                inliers.push_back(points[i]);  // 存储内点
            }
        }

        // 如果当前拟合得分更高，则更新最优结果
        if (score > bestScore) {
            line = cv::Vec4f(static_cast<float>(dp.x), static_cast<float>(dp.y), static_cast<float>(p1.x), static_cast<float>(p1.y));
            bestScore = score;
            inlierPoints = inliers;  // 更新内点
        }
    }
}

cv::Point2f calculateLineIntersection(const cv::Vec4f& line1, const cv::Vec4f& line2) {
    float vx1 = line1[0], vy1 = line1[1], x01 = line1[2], y01 = line1[3];
    float vx2 = line2[0], vy2 = line2[1], x02 = line2[2], y02 = line2[3];

    // 计算交点
    float denominator = vx1 * vy2 - vy1 * vx2;
    if (std::abs(denominator) < 1e-10) {
        return cv::Point2f(-1, -1);  // 平行线
    }

    float t = ((x02 - x01) * vy2 - (y02 - y01) * vx2) / denominator;
    float x = x01 + t * vx1;
    float y = y01 + t * vy1;

    return cv::Point2f(x, y);
}

bool isPointClockwiseTo(const cv::Point2f& pointA, const cv::Point2f& pointB, const cv::Point2f& referencePoint) {
    // 将参考点作为原点，计算相对坐标
    cv::Point2f relA = pointA - referencePoint;
    cv::Point2f relB = pointB - referencePoint;

    // 计算叉积 det = (ax * by - ay * bx)
    float det = relA.x * relB.y - relA.y * relB.x;

    // 如果叉积为正，b在a顺时针方向
    if (det > 0) return false;

    // 如果叉积为负，a在b顺时针方向
    if (det < 0) return true;

    // 叉积为0，共线情况，按距离排序（距离小的在顺时针方向）
    float d1 = relA.x * relA.x + relA.y * relA.y;
    float d2 = relB.x * relB.x + relB.y * relB.y;
    return d1 < d2;
}

cv::Vec4f fitLine(const std::vector<cv::Point>& points) {
    if (points.empty()) {
        return cv::Vec4f(0, 0, 0, 0);
    }

    cv::Vec4f lineParams;
    cv::fitLine(points, lineParams, cv::DIST_L2, 0, 0.01, 0.01);

    // lineParams格式: [vx, vy, x0, y0]
    // 其中(vx, vy)是单位方向向量，(x0, y0)是直线上的一个点
    return lineParams;
}

cv::Vec4f fitLine(const std::vector<cv::Point2f>& points) {
    if (points.empty()) {
        return cv::Vec4f(0, 0, 0, 0);
    }

    cv::Vec4f lineParams;
    // 使用 M-估计算法拟合点集到直线，通过最小化点到直线的距离代价函数实现。
    // 多种距离类型：DIST_L2（标准最小二乘法，速度快但对异常值敏感）；
    // DIST_L1（最小绝对值误差，对异常值更具鲁棒性）；
    // DIST_HUBER、DIST_FAIR和 DIST_WELSCH（使用权重函数降低异常值影响，鲁棒性递增）；
    // DIST_L12（L1-L2混合度量）。
    // 拟合结果返回一个 Vec4f向量，格式为 (vx, vy, x0, y0)，其中 (vx, vy)是单位方向向量，(x0, y0)是直线上一点
    cv::fitLine(points, lineParams, cv::DIST_HUBER, 0, 0.01, 0.01);
    return lineParams;
}

// 将像素坐标转成世界坐标
std::vector<Eigen::Vector2d> pixel2World(const std::vector<cv::Point2f>& pix_pts) {
    // 将cv::Point2f格式转换为Eigen::Vector2d格式
    std::vector<Eigen::Vector2d> eigen_pix_pts;
    eigen_pix_pts.reserve(pix_pts.size());
    for (const auto& pt : pix_pts) {
        eigen_pix_pts.push_back(Eigen::Vector2d(pt.x, pt.y));
    }

    std::shared_ptr<CameraImageProcessor> imageProcessor;
    imageProcessor = std::make_shared<CameraImageProcessor>();
    imageProcessor->initCameraCalibrator();
    std::vector<Eigen::Vector2d> worldPoints = imageProcessor->convertToWorld(eigen_pix_pts);
    // 交换所有点的 x 和 y 坐标
    // for (auto &pt : worldPoints)
    // {
    //     std::swap(pt.x(), pt.y());
    // }
    // PLOG_INFO << "convert done";
    return worldPoints;
}

}  // namespace GeometryUtils

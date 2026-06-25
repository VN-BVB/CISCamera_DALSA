#include "workpiece_bounding_box.h"
#include <algorithm>
#include <cmath>
#include <opencv2/imgproc.hpp>

WorkpieceBoundingBox::WorkpieceBoundingBox() {}

/**
 * @brief 将旋转矩形转换为其四个顶点构成的多边形（逆时针顺序）
 * @param rect 旋转矩形
 * @return 包含四个顶点的多边形
 */
std::vector<cv::Point2f> getPolygonFromRotatedRect(const cv::RotatedRect& rect) {
    std::vector<cv::Point2f> poly(4);
    rect.points(poly.data()); // OpenCV内置方法，获取逆时针顶点
    return poly;
}

/**
 * @brief 计算二维向量的叉积
 * @param a 向量a
 * @param b 向量b
 * @return 叉积结果（a.x*b.y - a.y*b.x）
 */
inline float cross(const cv::Point2f& a, const cv::Point2f& b) {
    return a.x * b.y - a.y * b.x;
}

/**
 * @brief 判断点是否在裁剪边的内侧（基于逆时针多边形）
 * @param p 待判断的点
 * @param a 裁剪边的起点
 * @param b 裁剪边的终点
 * @return 若在外侧返回false，内侧返回true
 */
bool isInside(const cv::Point2f& p, const cv::Point2f& a, const cv::Point2f& b) {
    // 叉积 >= 0 表示点在逆时针边的内侧
    return cross(b - a, p - a) >= 0;
}

/**
 * @brief 计算两条线段的交点（仅当线段相交时有效）
 * @param s 线段1的起点
 * @param e 线段1的终点
 * @param a 线段2的起点
 * @param b 线段2的终点
 * @return 交点坐标
 */
cv::Point2f computeIntersection(const cv::Point2f& s, const cv::Point2f& e, const cv::Point2f& a, const cv::Point2f& b) {
    cv::Point2f d1 = b - a;  // 裁剪边的方向向量
    cv::Point2f d2 = e - s;  // 待裁剪线段的方向向量
    float denom = cross(d1, d2);

    if (denom == 0) {
        return cv::Point2f(0, 0); // 平行或共线，理论上不会进入此分支
    }

    cv::Point2f s_minus_a = s - a;
    float t = cross(s_minus_a, d2) / denom; // 交点在裁剪边上的参数
    return a + d1 * t; // 计算交点坐标
}

/**
 * @brief 计算两个凸多边形的交集（使用Sutherland-Hodgman算法）
 * @param subject 待裁剪的多边形
 * @param clip 裁剪窗口多边形
 * @return 交集多边形
 */
std::vector<cv::Point2f> polygonIntersection(const std::vector<cv::Point2f>& subject, const std::vector<cv::Point2f>& clip) {
    std::vector<cv::Point2f> output = subject;
    int clipSize = static_cast<int>(clip.size());

    for (int i = 0; i < clipSize; ++i) {
        int j = (i + 1) % clipSize; // 下一个顶点（闭合多边形）
        cv::Point2f a = clip[i];
        cv::Point2f b = clip[j];

        if (output.empty()) break; // 无交集，提前退出

        std::vector<cv::Point2f> input = output;
        output.clear();
        cv::Point2f s = input.back(); // 上一个顶点

        for (const cv::Point2f& e : input) {
            if (isInside(e, a, b)) {
                if (!isInside(s, a, b)) {
                    // 从外侧到内侧，计算交点并添加
                    output.push_back(computeIntersection(s, e, a, b));
                }
                output.push_back(e); // 添加当前顶点
            } else if (isInside(s, a, b)) {
                // 从内侧到外侧，计算交点并添加
                output.push_back(computeIntersection(s, e, a, b));
            }
            s = e; // 更新上一个顶点
        }
    }

    return output;
}

/**
 * @brief 计算rect1与rect2的交集占rect2的百分比
 * @param rect1 第一个旋转矩形
 * @param rect2 第二个旋转矩形
 * @return 交集占rect1的百分比（范围：0~100）
 */
float calculateOverlapPercentage(const cv::RotatedRect& rect1, const cv::RotatedRect& rect2) {
    // 转换为多边形
    std::vector<cv::Point2f> poly1 = getPolygonFromRotatedRect(rect1);
    std::vector<cv::Point2f> poly2 = getPolygonFromRotatedRect(rect2);

    // 计算交集多边形
    std::vector<cv::Point2f> intersection = polygonIntersection(poly1, poly2);
    // 检查交集多边形是否有效（至少需要3个点才能构成有效多边形）
    if (intersection.size() < 3) {
        return 0.0f; // 没有交集或交集无效
    }

    // 计算交集面积（取绝对值，避免方向影响）
    float intersectionArea = cv::contourArea(intersection);
    intersectionArea = std::abs(intersectionArea);

    // 计算rect1的面积
    float area1 = rect1.size.width * rect1.size.height;

    // 避免除零错误
    if (area1 <= 0) {
        return 0.0f;
    }

    // 计算百分比
    return (intersectionArea / area1) * 100.0f;
}

cv::RotatedRect WorkpieceBoundingBox::generateOuterBoundingBox(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs)
{
    if (cbbs.empty()) return cv::RotatedRect(cv::Point2f(0,0), cv::Size2f(0,0), 0);

    std::vector<cv::Point2f> allPoints;

    // 收集所有轮廓边界框的角点
    for (const auto& boundingBox : cbbs) {
        // 获取当前边界框的旋转矩形
        cv::RotatedRect rotatedRect = boundingBox->getBoundingRect();

        // 获取旋转矩形的四个角点
        cv::Point2f vertices[4];
        rotatedRect.points(vertices);

        // 将角点添加到总点集中
        for (int i = 0; i < 4; ++i) {
            allPoints.push_back(vertices[i]);
        }
    }

    // 使用所有点生成最小外接旋转矩形
    cv::RotatedRect outerBoundingBox = cv::minAreaRect(allPoints);
    return outerBoundingBox;
}

void WorkpieceBoundingBox::updateCenterPointConnections() {
    m_centerPointConnections.clear();

    // 如果轮廓边界框数量少于2个，则没有连接线段
    if (m_cbbs.size() < 2) {
        return;
    }

    // 生成所有可能的轮廓边界框对
    for (size_t i = 0; i < m_cbbs.size(); ++i) {
        for (size_t j = i + 1; j < m_cbbs.size(); ++j) {
            // 获取两个轮廓边界框的中心点
            cv::Point2f center1 = m_cbbs[i]->getCenterPoint();
            cv::Point2f center2 = m_cbbs[j]->getCenterPoint();

            // 将中心点对添加到线段集合中
            m_centerPointConnections.push_back(std::make_pair(center1, center2));
        }
    }
}

void WorkpieceBoundingBox::updateOuterBoundingBox() {
    m_outerBoundingBox = generateOuterBoundingBox(m_cbbs);
    // 同时更新中心点连接线段
    updateCenterPointConnections();
}

bool WorkpieceBoundingBox::addContourBoundingBox(std::shared_ptr<ContourBoundingBox> cbb) {
    // 检查是否存在对立关系的轮廓
    int candidateOppositeId = cbb->getOppositeId();
    for (const auto& existingCbb : m_cbbs) {
        if (existingCbb->getId() == candidateOppositeId) {
            return false;
        }
    }

    m_cbbs.push_back(cbb);
    updateOuterBoundingBox();
    return true;
}

bool WorkpieceBoundingBox::isLegal(std::shared_ptr<ContourBoundingBox> candidateCbb) const {
    if (!candidateCbb) {
        return false;
    }

    // 1、首先判断候选框是否是组成工件的边界框
    int candidateId = candidateCbb->getId();
    for (const auto& existingCbb : m_cbbs) {
        if (existingCbb->getId() == candidateId) {
            return true;
        }
    }

    // 2、新增判断：检查候选轮廓的边界框是否包含工件中某个中心线段的端点
    cv::RotatedRect candidateRect = candidateCbb->getBoundingRect();
    cv::Point2f vertices[4];
    candidateRect.points(vertices);

    // 将候选轮廓边界框的四个角点转换为std::vector<cv::Point2f>用于pointPolygonTest
    std::vector<cv::Point2f> candidatePolygon(vertices, vertices + 4);

    // 遍历所有中心线段，检查端点是否在候选轮廓边界框内
    for (const auto& centerSegment : m_centerPointConnections) {
        // 检查第一个端点
        double result1 = cv::pointPolygonTest(candidatePolygon, centerSegment.first, false);
        // 检查第二个端点
        double result2 = cv::pointPolygonTest(candidatePolygon, centerSegment.second, false);

        // 如果任何一个端点在边界框内（result >= 0表示点在内部或边上）
        if (result1 >= 0 || result2 >= 0) {
            return true;
        }
    }

    std::vector<std::pair<cv::Point2f, cv::Point2f>> candidateEdges;
    for (int i = 0; i < 4; ++i) {
        candidateEdges.push_back(std::make_pair(vertices[i], vertices[(i + 1) % 4]));
    }

    // 3、检查候选轮廓边界框的任意边是否与工件外接矩形中心→CBB中心的连线相交
    cv::Point2f outerCenter = m_outerBoundingBox.center;
    for (const auto& candidateEdge : candidateEdges) {
        for (const auto& cbb : m_cbbs) {
            cv::Point2f cbbCenter = cbb->getCenterPoint();
            if (GeometryUtils::doSegmentsIntersect(
                    candidateEdge.first, candidateEdge.second,
                    outerCenter, cbbCenter)) {
                return false;
            }
        }
    }

    return true;
}

std::vector<int> WorkpieceBoundingBox::getContourIds() const {
    std::vector<int> ids;
    for (const auto& cbb : m_cbbs) {
        ids.push_back(cbb->getId());
    }
    return ids;
}

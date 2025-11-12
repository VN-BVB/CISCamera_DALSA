#include "contour_feature_calculator.h"
#include "contour_segmenter.h"
#include "src/utils/geometry_utils.h"
#include <unordered_set>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief 计算轮廓的开口方向
 * @param contour 输入轮廓点集
 * @return 开口方向枚举值
 */
OpeningDirection ContourFeatureCalculator::calculateOpeningDirection(const std::vector<cv::Point2f>& contour) {
    if (contour.empty()) return OpeningDirection::UNKNOWN;

    cv::Point2f centroid = ContourUtils::calculateCentralPoint(contour);
    float avgX = centroid.x;
    float avgY = centroid.y;

    bool hasUp = false, hasDown = false, hasLeft = false, hasRight = false;

    for (const auto& point : contour) {
        if (std::abs(point.x - avgX) < 1 && point.y < avgY) hasUp = true;       // 检查正上方（x坐标相同，y坐标更小）
        if (std::abs(point.x - avgX) < 1 && point.y > avgY) hasDown = true;     // 检查正下方（x坐标相同，y坐标更大）
        if (std::abs(point.y - avgY) < 1 && point.x < avgX) hasLeft = true;     // 检查正左方（y坐标相同，x坐标更小）
        if (std::abs(point.y - avgY) < 1 && point.x > avgX) hasRight = true;    // 检查正右方（y坐标相同，x坐标更大）
    }

    if (!hasUp) return OpeningDirection::UP;
    if (!hasDown) return OpeningDirection::DOWN;
    if (!hasLeft) return OpeningDirection::LEFT;
    if (!hasRight) return OpeningDirection::RIGHT;

    return OpeningDirection::UNKNOWN;
}

/**
 * @brief 去除轮廓中的重复点
 * @param contour 输入轮廓点集
 * @return 去除重复点后的轮廓点集
 */
std::vector<cv::Point2f> ContourFeatureCalculator::removeDuplicatePoints(const std::vector<cv::Point2f>& contour) {
    // 定义点比较结构体
    struct PointCompare {
        bool operator()(const cv::Point2f& a, const cv::Point2f& b) const {
            if (a.x == b.x) return a.y < b.y;
            return a.x < b.x;
        }
    };
    using PointSet = std::set<cv::Point2f, PointCompare>;

    PointSet seen;
    std::vector<cv::Point2f> uniquePoints;

    for (const auto& point : contour) {
        if (seen.insert(point).second) {
            uniquePoints.push_back(point);
        }
    }
    return uniquePoints;
}

/**
 * @brief 根据开口方向计算轮廓的起始点
 * @param direction 开口方向
 * @param contour 输入轮廓点集
 * @return 起始点坐标，如果方向未知或轮廓为空则返回(-1, -1)
 *
 * @details 根据开口方向选择逆时针的起始点：
 *    - 开口向上：左上角
 *    - 开口向右：右上角
 *    - 开口向下：右下角
 *    - 开口向左：左下角
 */
cv::Point2f ContourFeatureCalculator::calculateStartPoint(OpeningDirection direction, const std::vector<cv::Point2f>& contour) {
    if (direction == OpeningDirection::UNKNOWN || contour.empty()) {
        return cv::Point2f(-1, -1);
    }

    float sumX = 0.0f, sumY = 0.0f;
    for (const auto& point : contour) {
        sumX += point.x;
        sumY += point.y;
    }
    float avgX = sumX / contour.size();
    float avgY = sumY / contour.size();

    cv::Point2f startPoint;
    switch (direction) {
    case OpeningDirection::UP: {
        std::vector<cv::Point2f> leftHalfPoints;
        for (const auto& point : contour) {
            if (point.x < avgX) leftHalfPoints.push_back(point);
        }
        if (leftHalfPoints.empty()) leftHalfPoints = contour;
        startPoint = *std::min_element(leftHalfPoints.begin(), leftHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.y == b.y) return a.x < b.x;
                                           return a.y < b.y;
                                       });
        break;
    }
    case OpeningDirection::RIGHT: {
        std::vector<cv::Point2f> topHalfPoints;
        for (const auto& point : contour) {
            if (point.y < avgY) topHalfPoints.push_back(point);
        }
        if (topHalfPoints.empty()) topHalfPoints = contour;
        startPoint = *std::min_element(topHalfPoints.begin(), topHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.x == b.x) return a.y < b.y;
                                           return a.x > b.x;
                                       });
        break;
    }
    case OpeningDirection::DOWN: {
        std::vector<cv::Point2f> rightHalfPoints;
        for (const auto& point : contour) {
            if (point.x > avgX) rightHalfPoints.push_back(point);
        }
        if (rightHalfPoints.empty()) rightHalfPoints = contour;
        startPoint = *std::min_element(rightHalfPoints.begin(), rightHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.y == b.y) return a.x > b.x;
                                           return a.y > b.y;
                                       });
        break;
    }
    case OpeningDirection::LEFT: {
        std::vector<cv::Point2f> bottomHalfPoints;
        for (const auto& point : contour) {
            if (point.y > avgY) bottomHalfPoints.push_back(point);
        }
        if (bottomHalfPoints.empty()) bottomHalfPoints = contour;
        startPoint = *std::min_element(bottomHalfPoints.begin(), bottomHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.x == b.x) return a.y > b.y;
                                           return a.x < b.x;
                                       });
        break;
    }
    default:
        return cv::Point2f(-1, -1);
    }
    return startPoint;
}

/**
 * @brief 根据开口方向计算轮廓的终点
 * @param direction 开口方向
 * @param contour 输入轮廓点集
 * @return 终点坐标，如果方向未知或轮廓为空则返回(-1, -1)
 *
 * @details 根据开口方向选择逆时针的终止点：
 *    - 开口向上：终点在右上角（右侧区域的最小Y值点）
 *    - 开口向右：终点在右下角（下方区域的最大X值点）
 *    - 开口向下：终点在左下角（左侧区域的最大Y值点）
 *    - 开口向左：终点在左上角（上方区域的最小X值点）
 */
cv::Point2f ContourFeatureCalculator::calculateEndPoint(OpeningDirection direction, const std::vector<cv::Point2f>& contour) {
    if (direction == OpeningDirection::UNKNOWN || contour.empty()) {
        return cv::Point2f(-1, -1);
    }

    float sumX = 0.0f, sumY = 0.0f;
    for (const auto& point : contour) {
        sumX += point.x;
        sumY += point.y;
    }
    float avgX = sumX / contour.size();
    float avgY = sumY / contour.size();

    cv::Point2f endPoint;
    switch (direction) {
    case OpeningDirection::UP: {
        // 开口向上：终点在右上角
        std::vector<cv::Point2f> rightHalfPoints;
        for (const auto& point : contour) {
            if (point.x > avgX) rightHalfPoints.push_back(point);
        }
        if (rightHalfPoints.empty()) rightHalfPoints = contour;
        endPoint = *std::min_element(rightHalfPoints.begin(), rightHalfPoints.end(),
                                     [](const cv::Point2f& a, const cv::Point2f& b) {
                                         if (a.y == b.y) return a.x < b.x;
                                         return a.y < b.y;
                                     });
        break;
    }
    case OpeningDirection::RIGHT: {
        // 开口向右：终点在右下角
        std::vector<cv::Point2f> bottomHalfPoints;
        for (const auto& point : contour) {
            if (point.y > avgY) bottomHalfPoints.push_back(point);
        }
        if (bottomHalfPoints.empty()) bottomHalfPoints = contour;
        endPoint = *std::max_element(bottomHalfPoints.begin(), bottomHalfPoints.end(),
                                     [](const cv::Point2f& a, const cv::Point2f& b) {
                                         if (a.x == b.x) return a.y < b.y;
                                         return a.x < b.x;
                                     });
        break;
    }
    case OpeningDirection::DOWN: {
        // 开口向下：终点在左下角
        std::vector<cv::Point2f> leftHalfPoints;
        for (const auto& point : contour) {
            if (point.x < avgX) leftHalfPoints.push_back(point);
        }
        if (leftHalfPoints.empty()) leftHalfPoints = contour;
        endPoint = *std::min_element(leftHalfPoints.begin(), leftHalfPoints.end(),
                                     [](const cv::Point2f& a, const cv::Point2f& b) {
                                         if (a.y == b.y) return a.x > b.x;
                                         return a.y > b.y;
                                     });
        break;
    }
    case OpeningDirection::LEFT: {
        // 开口向左：终点在左上角
        std::vector<cv::Point2f> topHalfPoints;
        for (const auto& point : contour) {
            if (point.y < avgY) topHalfPoints.push_back(point);
        }
        if (topHalfPoints.empty()) topHalfPoints = contour;
        endPoint = *std::min_element(topHalfPoints.begin(), topHalfPoints.end(),
                                     [](const cv::Point2f& a, const cv::Point2f& b) {
                                         if (a.x == b.x) return a.y < b.y;
                                         return a.x < b.x;
                                     });
        break;
    }
    default:
        return cv::Point2f(-1, -1);
    }
    return endPoint;
}

/**
 * @brief 使用最近邻算法对轮廓点进行排序
 * @param contour 输入轮廓点集
 * @param firstPointIdx 起始点的索引
 * @return 排序后的轮廓点集，如果轮廓为空则返回空向量
 *
 * @details 实现过程：
 * 1. 首先检查输入轮廓是否为空
 * 2. 初始化访问标记数组和排序结果数组
 * 3. 从指定的起始点开始，将其添加到排序结果中
 * 4. 使用最近邻搜索算法：
 *    - 在当前点的所有未访问邻居中，找到距离最近的点
 *    - 将该点添加到排序结果中，并标记为已访问
 *    - 重复此过程直到所有点都被访问
 * 5. 返回排序后的轮廓点集
 */
std::vector<cv::Point2f> ContourFeatureCalculator::sortContour(const std::vector<cv::Point2f>& contour, int firstPointIdx) {
    if (contour.empty()) return {};

    std::vector<cv::Point2f> sortedContour;
    std::vector<bool> visited(contour.size(), false);

    int currentIndex = firstPointIdx;
    sortedContour.push_back(contour[currentIndex]);
    visited[currentIndex] = true;

    while (sortedContour.size() < contour.size()) {
        double minDistance = std::numeric_limits<double>::max();
        int nearestIndex = -1;

        for (int i = 0; i < contour.size(); ++i) {
            if (!visited[i]) {
                double distance = cv::norm(contour[currentIndex] - contour[i]);
                if (distance < minDistance) {
                    minDistance = distance;
                    nearestIndex = i;
                }
            }
        }

        if (nearestIndex != -1) {
            sortedContour.push_back(contour[nearestIndex]);
            visited[nearestIndex] = true;
            currentIndex = nearestIndex;
        } else {
            break;
        }
    }
    return sortedContour;
}

/**
 * @brief 基于质心的逆时针排序方法对轮廓点进行排序
 * @param contour 输入轮廓点集
 * @return 按逆时针方向排序后的轮廓点集，如果轮廓为空则返回空向量
 */
std::vector<cv::Point2f> ContourFeatureCalculator::sortContourByCentroid(const std::vector<cv::Point2f>& contour) {
    if (contour.empty()) return {};

    // 计算轮廓质心
    cv::Point2f centroid = ContourUtils::calculateCentralPoint(contour);

    // 创建点的副本用于排序
    std::vector<cv::Point2f> sortedContour = contour;

    // 按逆时针方向排序（相对于质心）
    std::sort(sortedContour.begin(), sortedContour.end(),
              [&centroid](const cv::Point2f& a, const cv::Point2f& b) {
                  return !GeometryUtils::isPointClockwiseTo(a, b, centroid);
              });

    return sortedContour;
}

/**
 * @brief 基于最近邻搜索和角度排序的轮廓排序方法
 * @param contour 输入轮廓点集
 * @param startIndex 起始点索引
 * @param endIndex 终点索引（可选，如果有效则在到达该点时停止排序）
 * @return std::vector<cv::Point2f> 排序后的轮廓点集
 * @details 该函数结合最近邻搜索和角度优化策略对轮廓点进行排序，支持在指定终点处停止排序。
 *          算法流程：
 *          1. 参数有效性检查（空轮廓、起始点索引、终点索引）
 *          2. 初始化访问标记数组和排序结果数组
 *          3. 从起始点开始，通过最近邻搜索找到候选点
 *          4. 在候选点中使用角度优化策略选择最合适的下一个点
 *          5. 重复步骤3-4直到所有点被访问或到达指定终点
 *          6. 返回排序后的轮廓点集
 */
std::vector<cv::Point2f> ContourFeatureCalculator::sortContourByNearestNeighbor(const std::vector<cv::Point2f>& contour, int startIndex, int endIndex) {
    if (contour.empty()) return {};

    // 验证起始点索引
    if (startIndex < 0 || startIndex >= contour.size()) {
        // 如果索引无效，使用默认排序方法
        return sortContour(contour, 0);
    }

    // 验证终点索引，如果无效则排序所有点
    bool hasValidEndIndex = (endIndex >= 0 && endIndex < contour.size() && endIndex != startIndex);

    std::vector<cv::Point2f> sortedContour;
    std::vector<bool> visited(contour.size(), false);

    // 添加起始点
    int currentIndex = startIndex;
    sortedContour.push_back(contour[currentIndex]);
    visited[currentIndex] = true;

    // 如果起始点就是终点，直接返回
    if (hasValidEndIndex && currentIndex == endIndex) {
        return sortedContour;
    }

    // 计算轮廓质心（用于角度计算）
    cv::Point2f centroid = ContourUtils::calculateCentralPoint(contour);

    while (sortedContour.size() < contour.size()) {
        // 检查是否到达终点
        if (hasValidEndIndex && currentIndex == endIndex) {
            break;
        }

        // 找到当前点的最近邻候选点（最近的k个点）
        const int k = 1; // 考虑最近的1个点
        std::vector<std::pair<int, double>> candidates; // <索引, 距离>

        for (int i = 0; i < contour.size(); ++i) {
            if (!visited[i]) {
                // 正常计算距离，不给终点特殊优先级
                double distance = cv::norm(contour[currentIndex] - contour[i]);
                candidates.push_back({i, distance});
            }
        }

        // 如果没有候选点，结束循环
        if (candidates.empty()) break;

        // 按距离排序，取最近的k个点
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) {
                      return a.second < b.second;
                  });

        if (candidates.size() > k) {
            candidates.resize(k);
        }

        // 在最近的k个点中，按角度选择最合适的下一个点
        int bestCandidateIndex = -1;
        double bestAngleScore = std::numeric_limits<double>::max();

        cv::Point2f currentPoint = contour[currentIndex];

        for (const auto& candidate : candidates) {
            int candidateIndex = candidate.first;
            cv::Point2f candidatePoint = contour[candidateIndex];

            // 计算从当前点到候选点的向量
            cv::Point2f direction = candidatePoint - currentPoint;

            // 计算角度（相对于水平方向）
            double angle = std::atan2(direction.y, direction.x);
            if (angle < 0) angle += 2 * M_PI; // 归一化到 [0, 2π)

            // 计算角度得分（我们希望保持逆时针方向）
            // 使用当前点到质心的向量作为参考方向
            cv::Point2f toCentroid = centroid - currentPoint;
            double referenceAngle = std::atan2(toCentroid.y, toCentroid.x);
            if (referenceAngle < 0) referenceAngle += 2 * M_PI;

            // 计算角度差异（考虑逆时针方向）
            double angleDiff = angle - referenceAngle;
            if (angleDiff < 0) angleDiff += 2 * M_PI;

            // 选择角度差异最小的点（保持平滑的逆时针方向）
            if (angleDiff < bestAngleScore) {
                bestAngleScore = angleDiff;
                bestCandidateIndex = candidateIndex;
            }
        }

        if (bestCandidateIndex != -1) {
            sortedContour.push_back(contour[bestCandidateIndex]);
            visited[bestCandidateIndex] = true;
            currentIndex = bestCandidateIndex;

            // 检查是否到达终点
            if (hasValidEndIndex && currentIndex == endIndex) {
                break;
            }
        } else {
            // 如果没有找到合适的候选点，选择最近的点
            int nearestIndex = candidates[0].first;
            sortedContour.push_back(contour[nearestIndex]);
            visited[nearestIndex] = true;
            currentIndex = nearestIndex;

            // 检查是否到达终点
            if (hasValidEndIndex && currentIndex == endIndex) {
                break;
            }
        }
    }

    return sortedContour;
}

std::vector<cv::Point2f> ContourFeatureCalculator::detectCornerPoints(const std::vector<cv::Point2f>& contour) {
    // 使用DouglasPeucker多边形拟合算法
    // return detectCornerPointsByDouglasPeucker(contour);
    // 使用RANSAC方法检测角点
    return detectCornerPointsByRansac(contour);
}

/**
 * @brief 使用RANSAC方法检测轮廓角点
 * @param contour 输入轮廓点集
 * @return std::vector<cv::Point2f> 检测到的角点集合
 * @details 该函数通过RANSAC算法拟合三条直线，然后计算这些直线的交点作为角点。
 */
std::vector<cv::Point2f> ContourFeatureCalculator::detectCornerPointsByRansac(const std::vector<cv::Point2f>& contour) {
    if (contour.size() < 6) {
        // 如果点数不足，使用默认方法
        return detectCornerPointsByDouglasPeucker(contour);
    }

    std::vector<cv::Point2f> cornerPoints;

    // 使用ContourSegmenter中的RANSAC方法拟合三条直线
    std::vector<std::vector<cv::Point2f>> segments;
    std::vector<cv::Vec4f> lines;
    double threshold = 8.0;
    int maxIterations = 100;

    // 调用ContourSegmenter的RANSAC方法
    ContourSegmenter::sequentialRansac3Times(contour, segments, lines, threshold, maxIterations);

    // 计算三条直线的交点作为角点
    // 交点1: 直线1和直线2的交点
    cv::Point2f corner1 = GeometryUtils::calculateLineIntersection(lines[0], lines[1]);
    // 交点2: 直线2和直线3的交点
    cv::Point2f corner2 = GeometryUtils::calculateLineIntersection(lines[1], lines[2]);
    // 交点3: 直线3和直线1的交点
    cv::Point2f corner3 = GeometryUtils::calculateLineIntersection(lines[2], lines[0]);

    // 检查交点是否有效（不是平行线）
    if (corner1.x >= 0 && corner1.y >= 0) {
        cornerPoints.push_back(corner1);
    }
    if (corner2.x >= 0 && corner2.y >= 0) {
        cornerPoints.push_back(corner2);
    }
    if (corner3.x >= 0 && corner3.y >= 0) {
        cornerPoints.push_back(corner3);
    }

    return cornerPoints;
}

/**
 * @brief 使用Douglas-Peucker算法检测轮廓角点
 * @param contour 输入轮廓点集
 * @param epsilon 逼近精度参数，控制简化程度（值越大简化越严重）
 * @return std::vector<cv::Point2f> 检测到的角点集合
 * @details 该函数通过Douglas-Peucker多边形逼近算法简化轮廓，将多边形顶点作为角点。
 */
std::vector<cv::Point2f> ContourFeatureCalculator::detectCornerPointsByDouglasPeucker(const std::vector<cv::Point2f>& contour, double epsilon) {
    if (contour.size() < 3) return {};

    std::vector<cv::Point> intContour;
    for (const auto& pt : contour) {
        intContour.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
    }

    std::vector<cv::Point> approx;
    cv::approxPolyDP(intContour, approx, epsilon, false);

    std::vector<cv::Point2f> cornerPoints;
    for (const auto& pt : approx) {
        cornerPoints.push_back(cv::Point2f(static_cast<float>(pt.x), static_cast<float>(pt.y)));
    }
    return cornerPoints;
}

/**
 * @brief 移除轮廓中靠近角点的点
 * @param contour 输入轮廓点集
 * @param cornerPoints 角点集合
 * @param radius 过滤半径，指定角点周围的排除区域大小
 * @return std::vector<cv::Point2f> 过滤后的轮廓点集（移除靠近角点的点）
 */
std::vector<cv::Point2f> ContourFeatureCalculator::removePointsNearCorners(const std::vector<cv::Point2f>& contour,
                                                                           const std::vector<cv::Point2f>& cornerPoints,
                                                                           double radius) {
    if (contour.empty() || cornerPoints.empty()) return contour;

    std::vector<cv::Point2f> filteredContour;
    double radiusSquared = radius * radius;

    for (const auto& point : contour) {
        bool isNearCorner = false;
        // 检查当前点是否在任何一个角点的半径范围内
        for (const auto& corner : cornerPoints) {
            double dx = point.x - corner.x;
            double dy = point.y - corner.y;
            if (dx * dx + dy * dy <= radiusSquared) {
                isNearCorner = true;
                break;
            }
        }
        if (!isNearCorner) {
            filteredContour.push_back(point);
        }
    }
    return filteredContour;
}

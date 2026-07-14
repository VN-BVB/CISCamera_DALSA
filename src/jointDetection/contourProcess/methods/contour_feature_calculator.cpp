#include <unordered_set>
#include <cmath>

#include "contour_feature_calculator.h"
#include "contour_segmenter.h"
#include "src/utils/geometry_utils.h"
#include "src/utils/scoped_timer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
 * @brief 对轮廓降采样一倍，每两个点保留一个
 * @param contour 输入轮廓（去重后的有序点集）
 * @return 降采样后的轮廓，包含原序列下标为 0,2,4,... 的点
 * @details 不依赖空间顺序，仅按 vector 下标隔点取样。
 *          空输入返回空；单点输入原样返回。
 *          reserve 容量为 (N+1)/2，对应奇数点数时取 ceil(N/2)。
 */
std::vector<cv::Point2f> ContourFeatureCalculator::downsampleByTwo(const std::vector<cv::Point2f>& contour) {
    std::vector<cv::Point2f> result;
    result.reserve((contour.size() + 1) / 2);
    for (size_t i = 0; i < contour.size(); i += 2) {
        result.push_back(contour[i]);
    }
    return result;
}

/**
 * @brief 极角差最大 gap 辅助函数：返回 gap 的索引和角平分线角度
 * @param contour 输入轮廓点集
 * @return {maxGapIdx, gapCenterAngle, valid}
 */
ContourFeatureCalculator::MaxGapResult ContourFeatureCalculator::computeMaxGapAngle(const std::vector<cv::Point2f>& contour) {
    MaxGapResult result{-1, 0.0f, false};

    if (contour.size() < 2) {
        return result;
    }

    // 以包围盒中心作为极点
    cv::Point2f center = ContourUtils::calculateCentralPoint(contour);

    // 计算极角并归一化到 [0, 2π)
    const float kTwoPi = 2.0f * static_cast<float>(M_PI);
    std::vector<std::pair<float, cv::Point2f>> anglePoint;
    anglePoint.reserve(contour.size());
    for (const auto& p : contour) {
        float raw = std::atan2(p.y - center.y, p.x - center.x);
        float theta = (raw < 0.0f) ? raw + kTwoPi : raw;
        anglePoint.emplace_back(theta, p);
    }

    // 按角度升序排列
    std::sort(anglePoint.begin(), anglePoint.end(),
              [](const std::pair<float, cv::Point2f>& a, const std::pair<float, cv::Point2f>& b) {
                  return a.first < b.first;
              });

    // 找相邻角度差最大的位置（含首尾环绕差）
    int n = static_cast<int>(anglePoint.size());
    int maxGapIdx = 0;
    float maxGap = -1.0f;
    for (int i = 0; i < n; ++i) {
        float gap = (i + 1 < n)
                        ? (anglePoint[i + 1].first - anglePoint[i].first)
                        : (anglePoint[0].first + kTwoPi - anglePoint[i].first);
        if (gap > maxGap) {
            maxGap = gap;
            maxGapIdx = i;
        }
    }

    // 退化：全部点重合于极点（最大角度差为0）
    if (maxGap <= 0.0f) {
        return result;
    }

    // 计算 gap 角平分线
    float beforeGap = anglePoint[maxGapIdx].first;
    float afterGap;
    if (maxGapIdx + 1 < n) {
        afterGap = anglePoint[maxGapIdx + 1].first;
    } else {
        afterGap = anglePoint[0].first + kTwoPi;  // 环绕到下一周
    }
    float gapCenterAngle = (beforeGap + afterGap) / 2.0f;
    if (gapCenterAngle >= kTwoPi) gapCenterAngle -= kTwoPi;

    result.maxGapIdx = maxGapIdx;
    result.gapCenterAngle = gapCenterAngle;
    result.valid = true;
    return result;
}

/**
 * @brief 角度法定位C型轮廓开口的起终点
 * @param contour 输入轮廓点集
 * @return {起点, 终点}。size<2 返回 {(-1,-1),(-1,-1)}；退化（全部点重合）返回 {首点, 首点}
 *
 * @details 算法步骤：
 *   1. 以包围盒中心（ContourUtils::calculateCentralPoint）为极点，对C型轮廓开口侧稀疏点不敏感
 *   2. 每个点计算极角 atan2(dy, dx) 并归一化到 [0, 2π)
 *   3. 按角度升序排列
 *   4. 相邻角度差（含首尾环绕差）最大的位置即为开口
 *   5. 起终点赋值：sortContourByNearestNeighbor 按逆时针（角度递增）遍历并在终点停止，
 *      故 gap 之后的小角度端点为起点，gap 之前的大角度端点为终点
 */
std::pair<cv::Point2f, cv::Point2f> ContourFeatureCalculator::findOpeningEndsByAngle(const std::vector<cv::Point2f>& contour) {
    if (contour.size() < 2) {
        return {cv::Point2f(-1, -1), cv::Point2f(-1, -1)};
    }

    // 以包围盒中心作为极点
    cv::Point2f center = ContourUtils::calculateCentralPoint(contour);

    // 计算极角并归一化到 [0, 2π)
    const float kTwoPi = 2.0f * static_cast<float>(M_PI);
    std::vector<std::pair<float, cv::Point2f>> anglePoint;
    anglePoint.reserve(contour.size());
    for (const auto& p : contour) {
        float raw = std::atan2(p.y - center.y, p.x - center.x);
        float theta = (raw < 0.0f) ? raw + kTwoPi : raw;
        anglePoint.emplace_back(theta, p);
    }

    // 按角度升序排列
    std::sort(anglePoint.begin(), anglePoint.end(),
              [](const std::pair<float, cv::Point2f>& a, const std::pair<float, cv::Point2f>& b) {
                  return a.first < b.first;
              });

    // 复用 computeMaxGapAngle 获取 maxGapIdx
    auto gap = computeMaxGapAngle(contour);
    if (!gap.valid) {
        return {contour.front(), contour.front()};
    }

    int n = static_cast<int>(anglePoint.size());
    // gap 之前的大角度端点为终点，gap 之后的小角度端点为起点
    cv::Point2f endPoint = anglePoint[gap.maxGapIdx].second;
    cv::Point2f startPoint = anglePoint[(gap.maxGapIdx + 1) % n].second;
    return {startPoint, endPoint};
}

/**
 * @brief 计算开口方向单位向量
 * @param contour 输入轮廓点集
 * @return 开口方向单位向量（从轮廓中心指向开口外侧），失败或退化时返回 (0, 0)
 *
 * @details 算法步骤：
 *   1. 计算极角差最大 gap 的角平分线方向（gapDir）
 *   2. 计算外接矩形长轴方向（longAxisDir）
 *   3. 取长轴法线两个候选中与 gapDir 点积 ≥ 0 的那个作为最终开口方向
 */
cv::Point2f ContourFeatureCalculator::calculateOpeningDirectionVector(const std::vector<cv::Point2f>& contour) {
    if (contour.size() < 2) {
        return cv::Point2f(0, 0);
    }

    // 1. 极角差最大 gap 方向
    auto gap = computeMaxGapAngle(contour);
    if (!gap.valid) {
        return cv::Point2f(0, 0);
    }
    cv::Point2f gapDir(std::cos(gap.gapCenterAngle), std::sin(gap.gapCenterAngle));

    // 2. 外接矩形长轴方向
    cv::RotatedRect box = cv::minAreaRect(contour);
    if (box.size.width * box.size.height < 1e-6f) {
        return cv::Point2f(0, 0);
    }
    cv::Point2f verts[4];
    box.points(verts);
    cv::Point2f edge01 = verts[1] - verts[0];
    cv::Point2f edge12 = verts[2] - verts[1];
    cv::Point2f longAxisDir = (cv::norm(edge01) >= cv::norm(edge12)) ? edge01 : edge12;
    float len = static_cast<float>(cv::norm(longAxisDir));
    if (len < 1e-6f) {
        return cv::Point2f(0, 0);
    }
    longAxisDir /= len;

    // 3. 长轴法线两个候选，选与 gapDir 同向的那个
    cv::Point2f normal(-longAxisDir.y, longAxisDir.x);
    return (normal.dot(gapDir) >= 0) ? normal : -normal;
}

std::pair<cv::Point2f, cv::Point2f> ContourFeatureCalculator::calculateStartAndEndPoint(const std::vector<cv::Point2f>& contour) {
    return findOpeningEndsByAngle(contour);
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
 * @brief 获取当前点的所有未访问候选点（按距离排序）
 * @param contour 轮廓点集
 * @param currentIndex 当前点索引
 * @param visited 访问标记数组
 * @param k 返回最邻近点的个数
 * @return std::vector<std::pair<int, double>> 候选点列表<索引, 距离>
 */
std::vector<std::pair<int, double>> ContourFeatureCalculator::getSortedCandidatesByDistance(const std::vector<cv::Point2f>& contour,
                                                                                            int currentIndex,
                                                                                            const std::vector<bool>& visited,
                                                                                            int k) {
    SCOPED_TIMER("getSortedCandidatesByDistance");
    std::vector<std::pair<int, double>> candidates;

    for (int i = 0; i < contour.size(); ++i) {
        if (!visited[i]) {
            double distance = cv::norm(contour[currentIndex] - contour[i]);
            candidates.push_back({i, distance});
        }
    }

    // 按距离排序（从近到远）
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) {
                  return a.second < b.second;
              });

    // 只返回最近的k个点
    if (k > 0 && k < candidates.size()) {
        candidates.resize(k);
    }

    return candidates;
}

/**
 * @brief 选择下一个合适的候选点（优先选择逆时针方向的点）
 * @param contour 轮廓点集
 * @param currentIndex 当前点索引
 * @param candidates 候选点列表
 * @param centroid 轮廓质心
 * @return int 选择的候选点索引，-1表示没有找到合适的点
 */
int ContourFeatureCalculator::selectNextCandidate(const std::vector<cv::Point2f>& contour,
                                                  int currentIndex,
                                                  const std::vector<std::pair<int, double>>& candidates,
                                                  const cv::Point2f& centroid) {
    if (candidates.empty()) return -1;

    cv::Point2f currentPoint = contour[currentIndex];

    // 初始化最近的逆时针方向点索引和距离
    int nearestCounterclockwiseIndex = -1;
    double minDistance = std::numeric_limits<double>::max();

    // 遍历所有候选点，找出最近的逆时针方向点
    for (const auto& candidate : candidates) {
        int candidateIndex = candidate.first;
        cv::Point2f candidatePoint = contour[candidateIndex];
        double candidateDistance = candidate.second;

        // 检查是否在逆时针方向
        if (!GeometryUtils::isPointClockwiseTo(candidatePoint, currentPoint, centroid)) {
            // 如果是逆时针方向且距离更近，则更新
            if (candidateDistance < minDistance) {
                minDistance = candidateDistance;
                nearestCounterclockwiseIndex = candidateIndex;
            }
        }
    }

    // 如果找到逆时针方向的点，则返回最近的那个
    if (nearestCounterclockwiseIndex != -1) {
        return nearestCounterclockwiseIndex;
    }

    // 如果没有找到逆时针方向的点，返回最近的点（即第一个候选点）
    return candidates[0].first;
}

/**
 * @brief 在轮廓点集中查找下一个合适的点（直接取最近点）
 * @param contour 输入轮廓点集，包含所有待处理的点
 * @param currentIndex 当前处理点的索引
 * @param visited 访问标记数组，标记哪些点已经被处理过
 * @param centroid 轮廓的质心点（保留接口，未使用）
 * @return int 找到的下一个点的索引，如果没有找到则返回-1
 * @details 遍历所有未访问点，返回距离当前点最近的点索引。
 *          方向修正由 SortingStrategy 中的叉积判定统一处理。
 */
int ContourFeatureCalculator::findNextPoint(const std::vector<cv::Point2f>& contour,
                                            int currentIndex,
                                            const std::vector<bool>& visited,
                                            const cv::Point2f& centroid) {
    (void)centroid;  // 保留接口，不再使用

    cv::Point2f currentPoint = contour[currentIndex];
    int bestIndex = -1;
    double bestDistance = std::numeric_limits<double>::max();

    for (int i = 0; i < contour.size(); ++i) {
        if (visited[i]) continue;
        double distance = cv::norm(currentPoint - contour[i]);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return bestIndex;
}

/**
 * @brief 使用最近邻算法对轮廓点进行排序，优先选择逆时针方向的点
 * @param contour 输入轮廓点集
 * @param startIndex 起始点索引
 * @param endIndex 终点索引（可选，如果为-1则排序所有点）
 * @return std::vector<cv::Point2f> 排序后的轮廓点集
 * @details 该函数通过最近邻算法对轮廓点进行排序，优先选择位于当前点逆时针方向的点。
 *          如果逆时针方向的点距离超过阈值，则选择最近的点。
 */
std::vector<cv::Point2f> ContourFeatureCalculator::sortContourByNearestNeighbor(const std::vector<cv::Point2f>& contour, int startIndex, int endIndex) {

    if (contour.empty()) return {};
    if (startIndex < 0 || startIndex >= contour.size()) {
        return sortContour(contour, 0);
    }
    bool hasValidEndIndex = (endIndex >= 0 && endIndex < contour.size() && endIndex != startIndex);

    std::vector<cv::Point2f> sortedContour;
    std::vector<bool> visited(contour.size(), false);
    int currentIndex = startIndex;
    sortedContour.push_back(contour[currentIndex]);
    visited[currentIndex] = true;

    // 计算轮廓中心点
    cv::Point2f centroid = ContourUtils::calculateCentralPoint(contour);
    while (sortedContour.size() < contour.size()) {
        // 检查是否到达终点
        if (hasValidEndIndex && currentIndex == endIndex) {
            break;
        }

        int selectedCandidateIndex = findNextPoint(contour, currentIndex, visited, centroid);
        if (selectedCandidateIndex == -1) break;

        // 添加选中的点到结果中
        sortedContour.push_back(contour[selectedCandidateIndex]);
        visited[selectedCandidateIndex] = true;
        currentIndex = selectedCandidateIndex;
    }

    return sortedContour;
}

std::vector<cv::Point2f> ContourFeatureCalculator::detectCornerPoints(const std::vector<cv::Point2f>& contour) {
    // 使用DouglasPeucker多边形拟合算法
    // return detectCornerPointsByDouglasPeucker(contour);
    // 使用RANSAC方法检测角点
    // return detectCornerPointsByRansac(contour);
    // 使用RANSAC方法检测角点,只进行一次RANSAC，取内点首尾作为角点
    return detectCornerPointsByRansacEndpoints(contour);
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
    double threshold = 50.0;
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
 * @brief 单次 RANSAC 直线拟合，取内点首末点作为角点
 * @param contour 输入轮廓点集（须已按轮廓顺序排序）
 * @return 检测到的角点集合（2 个点：内点序列的首末）；点数不足或拟合失败时返回空
 * @details 对整段轮廓只做一次 RANSAC 直线拟合，把参与该直线的内点中
 *          按轮廓顺序的第一个和最后一个点作为角点。
 *          因为 GeometryUtils::lineRansac 的内点保持输入点集顺序，
 *          所以 inliers.front() / inliers.back() 即首末轮廓点。
 *          适合"一段长边 + 两端转角"的工件形状。
 */
std::vector<cv::Point2f> ContourFeatureCalculator::detectCornerPointsByRansacEndpoints(
    const std::vector<cv::Point2f>& contour)
{
    if (contour.size() < 2) return {};

    constexpr double kThreshold = 8.0;       // 与 detectCornerPointsByRansac 保持一致
    constexpr int kMaxIterations = 100;

    cv::Vec4f line;
    std::vector<cv::Point2f> inliers;
    GeometryUtils::lineRansac(contour, line, inliers, kThreshold, kMaxIterations);

    if (inliers.size() < 2) return {};

    return { inliers.front(), inliers.back() };
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

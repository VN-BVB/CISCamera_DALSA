#include "contour_segmenter.h"
#include "src/utils/geometry_utils.h"
#include "contour_utils.h"
#include "contour_feature_calculator.h"

/**
 * @brief 将轮廓分割为三段子轮廓
 * @param contour 输入轮廓点集
 * @return std::vector<std::vector<cv::Point2f>> 分割后的三段子轮廓集合
 * @details 通过三次RANSAC直线拟合，每次拟合后移除内点，最终得到三个轮廓段。
 */
std::vector<std::vector<cv::Point2f>> ContourSegmenter::segmentContour(const std::vector<cv::Point2f>& contour) {
    std::vector<std::vector<cv::Point2f>> segmentedContours;
    std::vector<cv::Vec4f> lines;
    double threshold = 5;
    int maxIterations = 100;

    sequentialRansac3Times(contour, segmentedContours, lines, threshold, maxIterations);
    return segmentedContours;
}

/**
 * @brief 将轮廓段按逆时针方向排序并确定三条关键轮廓
 * @param segments 输入轮廓段集合，包含多个轮廓点集
 * @param referencePoint 参考点坐标，用于逆时针排序的基准点
 * @return std::map<int, std::vector<cv::Point2f>> 排序后的轮廓段集合，键为轮廓索引(1,2,3)
 * @details 该函数将输入轮廓段按逆时针方向排序，并确定三条关键轮廓：
 *          索引1：点数最多的轮廓的前一个轮廓
 *          索引2：点数最多的轮廓（中间轮廓）
 *          索引3：点数最多的轮廓的后一个轮廓
 */
std::map<int, std::vector<cv::Point2f>> ContourSegmenter::sortSegmentsCounterClockwise(
    const std::vector<std::vector<cv::Point2f>>& segments,
    const cv::Point2f& referencePoint) {

    std::map<int, std::vector<cv::Point2f>> sortedContours;
    if (segments.empty()) return sortedContours;

    // 1. 从每段轮廓中选取中间点
    std::vector<std::pair<cv::Point2f, std::vector<cv::Point2f>>> contoursWithMidPoints;
    for (const auto& contour : segments) {
        if (contour.empty()) continue;
        int midIndex = static_cast<int>(contour.size() / 2);
        contoursWithMidPoints.push_back({contour[midIndex], contour});
    }

    if (contoursWithMidPoints.empty()) return sortedContours;

    // 2. 对每段轮廓中间点进行逆时针排序（相对于参考点）
    // 使用冒泡排序进行逆时针排序
    for (int i = 0; i < contoursWithMidPoints.size() - 1; i++) {
        for (int j = 0; j < contoursWithMidPoints.size() - i - 1; j++) {
            const cv::Point2f& pointA = contoursWithMidPoints[j].first;
            const cv::Point2f& pointB = contoursWithMidPoints[j + 1].first;
            // 如果pointA在pointB的顺时针方向，交换位置
            if (GeometryUtils::isPointClockwiseTo(pointA, pointB, referencePoint)) {
                std::swap(contoursWithMidPoints[j], contoursWithMidPoints[j + 1]);
            }
        }
    }

    // 3. 创建循环链表结构（使用vector模拟循环链表）
    std::vector<std::pair<cv::Point2f, std::vector<cv::Point2f>>> circularList = contoursWithMidPoints;
    // 4. 找到点数最多的轮廓
    auto maxPointContour = std::max_element(circularList.begin(), circularList.end(),
                                            [](const auto& a, const auto& b) { return a.second.size() < b.second.size(); });

    if (maxPointContour == circularList.end()) return sortedContours;

    // 5. 确定三条轮廓的位置
    int maxIndex = static_cast<int>(std::distance(circularList.begin(), maxPointContour));
    int n = static_cast<int>(circularList.size());
    int firstIndex = (maxIndex - 1 + n) % n;
    int thirdIndex = (maxIndex + 1) % n;

    // 6. 将三条轮廓存入map
    sortedContours[1] = circularList[firstIndex].second;
    sortedContours[2] = circularList[maxIndex].second;
    sortedContours[3] = circularList[thirdIndex].second;

    return sortedContours;
}

/**
 * @brief sequentialRansac3Times 三次顺序RANSAC直线拟合
 * @param points 输入亚像素点集
 * @param segments 输出三段轮廓亚像素点集
 * @param lines 输出三段直线参数
 * @param threshold 内点距离阈值
 * @param maxIterations 单次RANSAC最大迭代次数
 */
void ContourSegmenter::sequentialRansac3Times(const std::vector<cv::Point2f>& points,
                                              std::vector<std::vector<cv::Point2f>>& segments,
                                              std::vector<cv::Vec4f>& lines,
                                              double threshold,
                                              int maxIterations)
{
    // 初始化输出容器
    segments.clear();
    lines.clear();

    // 确保有足够的点进行三次拟合
    if (points.size() < 6) {
        std::cerr << "Not enough points for 3 sequential RANSAC fits. Need at least 6 points." << std::endl;
        return;
    }

    // 复制点集用于处理
    std::vector<cv::Point2f> remainingPoints = points;

    // 进行三次RANSAC拟合
    for (int segmentIndex = 0; segmentIndex < 3; ++segmentIndex) {
        if (remainingPoints.size() < 2) {
            std::cerr << "Not enough remaining points for segment " << segmentIndex + 1 << std::endl;
            break;
        }

        cv::Vec4f currentLine;
        std::vector<cv::Point2f> currentInliers;

        GeometryUtils::lineRansac(remainingPoints, currentLine, currentInliers, threshold, maxIterations);


        // 存储结果
        lines.push_back(currentLine);
        segments.push_back(currentInliers);

        // 从剩余点中移除当前段的内点
        if (segmentIndex < 2) { // 前两次需要移除内点
            remainingPoints.erase(
                std::remove_if(remainingPoints.begin(), remainingPoints.end(),
                               [&currentInliers](const cv::Point2f& p) {
                                   for (const auto& inlier : currentInliers) {
                                       if (cv::norm(p - inlier) < 1e-6) { // 浮点数比较容差
                                           return true;
                                       }
                                   }
                                   return false;
                               }),
                remainingPoints.end()
                );
        }
    }

    // 如果第三次拟合后还有剩余点，直接丢弃
    if (!remainingPoints.empty() && segments.size() == 3) {
        remainingPoints.clear();
    }
}

/**
 * @brief 按两个角点索引将 U 形开放轮廓切分为 3 段，并在每段内剔除角点邻域
 * @param contour      已排序的开放轮廓（CCW）
 * @param cornerPoints 恰好 2 个角点（须为 contour 成员点）
 * @param radius       每段内角点邻域剔除半径（像素），默认 10.0
 * @return 轮廓遍历顺序的 3 段子轮廓；退化输入返回空 vector
 * @details 通过 findPointIndex 定位两个角点在排序轮廓中的索引，按索引将轮廓
 *          切分为三个连续子段，然后对每个子段调用 removePointsNearCorners
 *          剔除角点邻域内的点，确保后续直线拟合不受拐点影响。
 */
std::vector<std::vector<cv::Point2f>> ContourSegmenter::splitContourByCorners(
    const std::vector<cv::Point2f>& contour,
    const std::vector<cv::Point2f>& cornerPoints,
    double radius)
{
    std::vector<std::vector<cv::Point2f>> segments;

    // 退化输入检查
    if (contour.size() < 3) {
        std::cerr << "splitContourByCorners: contour too small (" << contour.size() << " points)" << std::endl;
        return segments;
    }
    if (cornerPoints.size() != 2) {
        std::cerr << "splitContourByCorners: expected 2 corner points, got " << cornerPoints.size() << std::endl;
        return segments;
    }

    // 查找两个角点在轮廓中的索引
    int idx1 = ContourUtils::findPointIndex(cornerPoints[0], contour);
    int idx2 = ContourUtils::findPointIndex(cornerPoints[1], contour);

    if (idx1 < 0 || idx2 < 0) {
        std::cerr << "splitContourByCorners: corner point not found in contour (idx1=" << idx1 << ", idx2=" << idx2 << ")" << std::endl;
        return segments;
    }
    if (idx1 == idx2) {
        std::cerr << "splitContourByCorners: both corners resolve to same index (" << idx1 << ")" << std::endl;
        return segments;
    }

    // 确保 idx1 < idx2
    if (idx1 > idx2) {
        std::swap(idx1, idx2);
    }

    const int n = static_cast<int>(contour.size());

    // 原始切分：三个连续子段（角点在边界处被两个段共享，后续裁剪会处理）
    // seg1: [0, idx1]
    // seg2: [idx1, idx2]
    // seg3: [idx2, n-1]
    std::vector<cv::Point2f> seg1(contour.begin(), contour.begin() + idx1 + 1);
    std::vector<cv::Point2f> seg2(contour.begin() + idx1, contour.begin() + idx2 + 1);
    std::vector<cv::Point2f> seg3(contour.begin() + idx2, contour.end());

    // 对每段剔除角点邻域（复用现有工具，保持 radius≈10 行为一致）
    segments.push_back(ContourFeatureCalculator::removePointsNearCorners(seg1, cornerPoints, radius));
    segments.push_back(ContourFeatureCalculator::removePointsNearCorners(seg2, cornerPoints, radius));
    segments.push_back(ContourFeatureCalculator::removePointsNearCorners(seg3, cornerPoints, radius));

    return segments;
}

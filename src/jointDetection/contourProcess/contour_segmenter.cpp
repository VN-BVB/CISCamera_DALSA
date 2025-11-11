#include "contour_segmenter.h"

std::vector<std::vector<cv::Point2f>> ContourSegmenter::segmentContour(const std::vector<cv::Point2f>& contour) {
    std::vector<std::vector<cv::Point2f>> segmentedContours;
    std::vector<cv::Vec4f> lines;
    double threshold = 5;
    int maxIterations = 100;

    sequentialRansac3Times(contour, segmentedContours, lines, threshold, maxIterations);
    return segmentedContours;
}

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
            if (isPointClockwiseTo(pointA, pointB, referencePoint)) {
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

bool ContourSegmenter::isPointClockwiseTo(const cv::Point2f& a, const cv::Point2f& b, const cv::Point2f& reference) {
    cv::Point2f relA = a - reference;
    cv::Point2f relB = b - reference;
    float det = relA.x * relB.y - relA.y * relB.x;

    if (det > 0) return false;
    if (det < 0) return true;

    float d1 = relA.x * relA.x + relA.y * relA.y;
    float d2 = relB.x * relB.x + relB.y * relB.y;
    return d1 < d2;
}

/**
 * @brief ImageProcessing_lineDetection     直线拟合Ransac
 * @param points                            输入亚像素点集
 * @param line                              输出直线参数(vx, vy, x0, y0), (vx, vy) 为方向向量, (x0, y0) 为直线上的一个点
 * @param inlierPoints                      输出直线内点
 * @param threshold                         阈值
 * @param iterations                        最大迭代次数
 */
void ContourSegmenter::lineRansac(const std::vector<cv::Point2f> &points,
                                cv::Vec4f &line,
                                std::vector<cv::Point2f> &inlierPoints,
                                const double &threshold,
                                const int &iterations)
{
    if(points.size() < 2){
        std::cerr<<"Input points is empty!"<<std::endl;
        return;
    }

    cv::RNG rng;// 创建随机数生成器
    double bestScore = -1.;
    auto n = points.size();  // 获取点集大小
    for(int iter = 0; iter < iterations; iter++){
        // 随机选择两个不同的点
        auto i1 = rng.uniform(0, static_cast<int>(n-1));
        auto i2 = rng.uniform(0, static_cast<int>(n-1));
        if (i1 == i2)
            continue;

        // 直线的方向向量
        const cv::Point2f& p1 = points[i1];
        const cv::Point2f& p2 = points[i2];
        cv::Point2f dp = p2-p1;
        dp *= 1.0/cv::norm(dp);

        // 计算内点
        double score = 0;
        std::vector<cv::Point2f> inliers;
        for(int i = 0; i< n; i++){
            cv::Point2f v = points[i] - p1;
            double d = v.y * dp.x - v.x * dp.y;//向量a与b叉乘/向量b的摸.||b||=1./norm(dp)
            // 判断点到直线的距离是否小于阈值
            if( std::fabs(d) < threshold){
                score += 1;
                inliers.push_back(points[i]);  // 存储内点
            }
        }

        // 如果当前拟合得分更高，则更新最优结果
        if(score > bestScore) {
            line = cv::Vec4f(static_cast<float>(dp.x), static_cast<float>(dp.y),
                             static_cast<float>(p1.x), static_cast<float>(p1.y));
            bestScore = score;
            inlierPoints = inliers;//更新内点
        }
    }
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

        lineRansac(remainingPoints, currentLine, currentInliers, threshold, maxIterations);


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

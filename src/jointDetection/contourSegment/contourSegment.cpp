#include "contourSegment.h"

ContourSegment::ContourSegment(const std::vector<cv::Point> &contour)
    :m_contour(contour)
{}

ContourSegment::ContourSegment(const std::vector<cv::Point2f> &contour)
    :m_subpixelContour(contour)
{}

ContourSegment::~ContourSegment()
{}

/**
 * @brief ImageProcessing_lineDetection     直线拟合Ransac
 * @param points                            输入点集
 * @param line                              输出直线参数(vx, vy, x0, y0), (vx, vy) 为方向向量, (x0, y0) 为直线上的一个点
 * @param inlierPoints                      输出直线内点
 * @param threshold                         阈值
 * @param iterations                        最大迭代次数
 */
void ContourSegment::lineRansac(const std::vector<cv::Point> &points,
                                cv::Vec4f &line,
                                std::vector<cv::Point> &inlierPoints,
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
        auto i1 = rng.uniform(0, n-1);
        auto i2 = rng.uniform(0, n-1);
        if (i1 == i2)
            continue;

        // 直线的方向向量
        const cv::Point& p1 = points[i1];
        const cv::Point& p2 = points[i2];
        cv::Point2f dp = p2-p1;
        dp *= 1.0/cv::norm(dp);

        // 计算内点
        double score = 0;
        std::vector<cv::Point> inliers;
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
            line = cv::Vec4f(dp.x, dp.y, p1.x, p1.y);
            bestScore = score;
            inlierPoints = inliers;//更新内点
        }
    }
}

/**
 * @brief ImageProcessing_lineDetection     直线拟合Ransac
 * @param points                            输入亚像素点集
 * @param line                              输出直线参数(vx, vy, x0, y0), (vx, vy) 为方向向量, (x0, y0) 为直线上的一个点
 * @param inlierPoints                      输出直线内点
 * @param threshold                         阈值
 * @param iterations                        最大迭代次数
 */
void ContourSegment::lineRansac(const std::vector<cv::Point2f> &points,
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
        auto i1 = rng.uniform(0, n-1);
        auto i2 = rng.uniform(0, n-1);
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
            line = cv::Vec4f(dp.x, dp.y, p1.x, p1.y);
            bestScore = score;
            inlierPoints = inliers;//更新内点
        }
    }
}

/**
 * @brief sequentialRansac3Times 三次顺序RANSAC直线拟合
 * @param points 输入点集
 * @param segments 输出三段轮廓点集
 * @param lines 输出三段直线参数
 * @param threshold 内点距离阈值
 * @param maxIterations 单次RANSAC最大迭代次数
 */
void ContourSegment::sequentialRansac3Times(const std::vector<cv::Point>& points,
                            std::vector<std::vector<cv::Point>>& segments,
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
    std::vector<cv::Point> remainingPoints = points;

    // 进行三次RANSAC拟合
    for (int segmentIndex = 0; segmentIndex < 3; ++segmentIndex) {
        if (remainingPoints.size() < 2) {
            std::cerr << "Not enough remaining points for segment " << segmentIndex + 1 << std::endl;
            break;
        }

        cv::Vec4f currentLine;
        std::vector<cv::Point> currentInliers;

        lineRansac(remainingPoints, currentLine, currentInliers, threshold, maxIterations);


        // 存储结果
        lines.push_back(currentLine);
        segments.push_back(currentInliers);

        // 从剩余点中移除当前段的内点
        if (segmentIndex < 2) { // 前两次需要移除内点
            remainingPoints.erase(
                std::remove_if(remainingPoints.begin(), remainingPoints.end(),
                               [&currentInliers](const cv::Point& p) {
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
 * @brief sequentialRansac3Times 三次顺序RANSAC直线拟合
 * @param points 输入亚像素点集
 * @param segments 输出三段轮廓亚像素点集
 * @param lines 输出三段直线参数
 * @param threshold 内点距离阈值
 * @param maxIterations 单次RANSAC最大迭代次数
 */
void ContourSegment::sequentialRansac3Times(const std::vector<cv::Point2f>& points,
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

/**
 * @brief segmentContourByApproxPoints 根据近似点分割轮廓
 * @param contour 输入轮廓点集
 * @param approxPoints 输入近似点集
 * @return 返回分割后的轮廓段集合
 */
std::vector<std::vector<cv::Point2f>> ContourSegment::segmentContourByApproxPoints(const std::vector<cv::Point2f>& contour,
                                                                                   const std::vector<cv::Point2f>& approxPoints)
{
    std::vector<std::vector<cv::Point2f>> segmentedContours;

    if (contour.empty() || approxPoints.empty()) {
        return segmentedContours;
    }

    // 如果只有一个拟合点，返回整个轮廓
    if (approxPoints.size() == 1) {
        segmentedContours.push_back(contour);
        return segmentedContours;
    }

    // 为每个拟合点在原始轮廓中找到最近的点
    std::vector<int> approxIndices;
    for (const auto& approxPoint : approxPoints) {
        int bestIndex = 0;
        double minDist = std::numeric_limits<double>::max();

        for (int i = 0; i < contour.size(); ++i) {
            double dist = cv::norm(contour[i] - cv::Point2f(approxPoint.x, approxPoint.y));
            if (dist < minDist) {
                minDist = dist;
                bestIndex = i;
            }
        }
        approxIndices.push_back(bestIndex);
    }

    // 对索引进行排序，确保按轮廓顺序分割
    std::sort(approxIndices.begin(), approxIndices.end());

    // 根据拟合点索引分割轮廓
    // 对于闭合轮廓，我们只需要在拟合点之间分割，不需要包含起点到终点的段
    for (int i = 0; i < approxIndices.size() - 1; ++i) {
        int startIdx = approxIndices[i];
        int endIdx = approxIndices[i + 1];

        // 确保索引有效
        if (startIdx >= 0 && endIdx >= 0 && startIdx < contour.size() && endIdx < contour.size()) {
            std::vector<cv::Point2f> segment;
            for (int j = startIdx; j <= endIdx; ++j) {
                segment.push_back(contour[j]);
            }

            // 确保段不为空
            if (!segment.empty()) {
                segmentedContours.push_back(segment);
            }
        }
    }

    // 处理最后一个段（从最后一个拟合点到轮廓结束）
    if (approxIndices.size() > 1) {
        int lastStartIdx = approxIndices.back();
        if (lastStartIdx >= 0 && lastStartIdx < contour.size()) {
            std::vector<cv::Point2f> lastSegment;
            for (int j = lastStartIdx; j < contour.size(); ++j) {
                lastSegment.push_back(contour[j]);
            }

            if (!lastSegment.empty()) {
                segmentedContours.push_back(lastSegment);
            }
        }
    }

    return segmentedContours;
}








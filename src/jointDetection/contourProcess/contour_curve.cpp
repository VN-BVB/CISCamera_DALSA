#define _USE_MATH_DEFINES

#include "contour_curve.h"
#include <QDebug>

/******************************
 *********ContourCurve*******
 ******************************/
ContourCurve::ContourCurve(): m_area(0.0), m_perimeter(0.0), m_aspectRatio(0.0),m_approxError(0.0) {}

[[deprecated("此类应处理亚像素级坐标，像素级初始化操作应弃用，并且此初始化操作计算内容不全")]]
/**
* @brief 从像素级轮廓点集初始化
* @param contour 输入轮廓像素点集
*/
void ContourCurve::initializePixelContour(const std::vector<cv::Point>& contour) {
    m_pixelContour = contour;
    m_deduplicatedPixelContour = removeDuplicateContourPoints(m_pixelContour);
    calculateBasicFeatures();
    calculateOpeningDirection();
    segment();
}

/**
* @brief 从像素级轮廓点集初始化
* @param contour 输入轮廓亚像素点集
*/
void ContourCurve::initializeSubpixelContour(const std::vector<cv::Point2f>& contour)
{
    m_subpixelContour = contour;
    m_deduplicatedSubpixelContour = removeDuplicateContourPoints(m_subpixelContour);    // 去重
    calculateOpeningDirection();   // 计算开口方向
    sortContour();                                      // 逆时针排序，相当于二次扫描轮廓
    detectCornerPoints();              // 检测角点
    removeCorners();                                    // 移除角点区域轮廓
    calculateBasicFeatures();                           // 计算基本特征
    segment();                                          // 分割轮廓
    sortSegmentedContours();                            // 逆时针排序分割轮廓
    calculateLines();                                   // 分区域直线拟合
    // calculateEndPointsByFittedLines();                  // 计算端点
    calculateBSplines();                                // 拟合样条曲线
    calculateEndPointsByFittedCurves();                 // 计算端点
}

/**
* @brief 计算基本几何特征
*/
void ContourCurve::calculateBasicFeatures() {
    if (!m_pixelContour.empty())
    {
        // 计算外接矩形
        m_boundingRect = cv::boundingRect(m_pixelContour);

        // 计算面积和周长
        m_area = cv::contourArea(m_pixelContour);
        m_perimeter = cv::arcLength(m_pixelContour, true);

        // 计算长宽比
        if (m_boundingRect.height > 0) {
            m_aspectRatio = static_cast<double>(m_boundingRect.width) / m_boundingRect.height;
        }

        // 计算质心
        cv::Moments moments = cv::moments(m_pixelContour);
        if (moments.m00 != 0) {
            m_centroid.x = static_cast<int>(moments.m10 / moments.m00);
            m_centroid.y = static_cast<int>(moments.m01 / moments.m00);
        }
    }
    else if (!m_subpixelContour.empty())
    {
        m_boundingRect = cv::boundingRect(m_pixelContour);
        m_area = cv::contourArea(m_pixelContour);
        m_perimeter = cv::arcLength(m_pixelContour, true);
        if (m_boundingRect.height > 0) {
            m_aspectRatio = static_cast<double>(m_boundingRect.width) / m_boundingRect.height;
        }
        cv::Moments moments = cv::moments(m_pixelContour);
        if (moments.m00 != 0) {
            m_centroid.x = static_cast<int>(moments.m10 / moments.m00);
            m_centroid.y = static_cast<int>(moments.m01 / moments.m00);
        }
    }
    else
    {
        return;
    }
}

/**
* @brief 清空所有特征数据
*/
void ContourCurve::clear() {
    m_pixelContour.clear();
    m_subpixelContour.clear();
    m_segmentedPixelContours.clear();
    m_segmentedSubpixelContours.clear();
    m_cornerPoints.clear();
    m_lineSegments.clear();
    m_lineSegments.clear();
    m_curveSegments.clear();
    m_curveSegments.clear();
    m_approxPolygon.clear();
    m_sortedSubpixelContour.clear();
    m_counterClockwiseContours.clear();
    m_openingDirection = OpeningDirection::UNKNOWN;
    m_area = 0.0;
    m_perimeter = 0.0;
    m_aspectRatio = 0.0;
    m_approxError = 0.0;
}

/**
* @brief 检查特征是否有效
* @return 如果轮廓不为空则返回true
*/
bool ContourCurve::isValid() const {
    return (!m_pixelContour.empty() || !m_subpixelContour.empty());
}

/**
* @brief 将开口方向枚举值转换为字符串
* @param direction 开口方向枚举值
* @return 对应的字符串描述
*/
std::string ContourCurve::openingDirectionToString(OpeningDirection direction) const
{
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
* @brief 获取特征摘要信息
* @return 特征摘要字符串
*/
std::string ContourCurve::getSummary() const {
    std::string summary;
    summary += "轮廓点数: " + std::to_string(m_pixelContour.size()) + "\n";
    summary += "去重后点数: " + std::to_string(m_deduplicatedPixelContour.size()) + "\n";
    summary += "开口方向: " + openingDirectionToString(m_openingDirection) + "\n";
    summary += "面积: " + std::to_string(m_area) + "\n";
    summary += "周长: " + std::to_string(m_perimeter) + "\n";
    summary += "长宽比: " + std::to_string(m_aspectRatio) + "\n";
    summary += "分割段数: " + std::to_string(m_segmentedPixelContours.size()) + "\n";
    summary += "拟合直线数: " + std::to_string(m_lineSegments.size()) + "\n";
    summary += "角点数: " + std::to_string(m_cornerPoints.size()) + "\n";
    return summary;
}

[[deprecated("弃用像素级坐标去重")]]
/**
* @brief removeDuplicateContourPoints 轮廓点去重
* @param contour 输入轮廓点集
* @return 去重后的轮廓点集
*/
std::vector<cv::Point> ContourCurve::removeDuplicateContourPoints(
    const std::vector<cv::Point>& contour)
{
    return removeDuplicateContourPointsImpl(contour);
}

/**
* @brief removeDuplicateContourPoints 轮廓点去重
* @param contour 输入轮廓点集
* @return 去重后的轮廓点集
*/
std::vector<cv::Point2f> ContourCurve::removeDuplicateContourPoints(
    const std::vector<cv::Point2f>& contour)
{
    return removeDuplicateContourPointsImpl(contour);
}

/**
* @brief 计算轮廓的开口方向
* @return 开口方向枚举值
*/
void ContourCurve::calculateOpeningDirection()
{
    // 优先使用亚像素级坐标，如果为空则使用像素级坐标
    if (!m_deduplicatedSubpixelContour.empty()) {
        m_openingDirection = calculateOpeningDirectionImpl(m_deduplicatedSubpixelContour);
    } else if (!m_deduplicatedPixelContour.empty()) {
        m_openingDirection = calculateOpeningDirectionImpl(m_deduplicatedPixelContour);
    }
}

/**
* @brief 根据开口方向获取曲线的起始点
* @return 起始点坐标
* 开口向上：最左上角为起始点
* 开口向右：右上角为起始点
* 开口向下：右下角为起始点
* 开口向左：左下角为起始点
*/
cv::Point2f ContourCurve::calculateStartPointByOpeningDirection(OpeningDirection openingDirection,
                                                                const std::vector<cv::Point2f> &contour)
{
    if (openingDirection == OpeningDirection::UNKNOWN) {
        qDebug() << "警告：开口方向未知，无法确定起始点";
        return cv::Point2f(-1, -1);
    }

    if (contour.empty()) {
        qDebug() << "警告：轮廓为空，无法确定起始点";
        return cv::Point2f(-1, -1);
    }
    // 计算轮廓点的x、y坐标平均值
    float sumX = 0.0f, sumY = 0.0f;
    for (const auto& point : contour) {
        sumX += point.x;
        sumY += point.y;
    }
    float avgX = sumX / contour.size();
    float avgY = sumY / contour.size();
    // 根据开口方向选择起始点
    cv::Point2f startPoint;
    switch (openingDirection) {
    case OpeningDirection::UP: {
        // 开口向上：取最左上角（y最小，在y最小的点中x最小）
        // 先过滤出左半部分的点（x < avgX）
        std::vector<cv::Point2f> leftHalfPoints;
        for (const auto& point : contour) {
            if (point.x < avgX) {
                leftHalfPoints.push_back(point);
            }
        }

        if (leftHalfPoints.empty()) {
            qDebug() << "警告：左半部分轮廓点为空，使用全部轮廓点";
            leftHalfPoints = contour;
        }

        // 在左半部分点中寻找最左上角的点（y最小，在y最小的点中x最小）
        startPoint = *std::min_element(leftHalfPoints.begin(), leftHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.y == b.y) return a.x < b.x;  // y相同时，x小的优先
                                           return a.y < b.y;                 // 优先比较y坐标（小的优先）
                                       });
        break;
    }

    case OpeningDirection::RIGHT: {
        // 开口向右：取右上角（x最大，在x最大的点中y最小）
        // 先过滤出上半部分的点（y < avgY）
        std::vector<cv::Point2f> topHalfPoints;
        for (const auto& point : contour) {
            if (point.y < avgY) {
                topHalfPoints.push_back(point);
            }
        }

        if (topHalfPoints.empty()) {
            qDebug() << "警告：上半部分轮廓点为空，使用全部轮廓点";
            topHalfPoints = contour;
        }

        // 在上半部分点中寻找右上角的点（x最大，在x最大的点中y最小）
        startPoint = *std::min_element(topHalfPoints.begin(), topHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.x == b.x) return a.y < b.y;  // x相同时，y小的优先
                                           return a.x > b.x;                 // 优先比较x坐标（大的优先）
                                       });
        break;
    }

    case OpeningDirection::DOWN: {
        // 开口向下：取右下角（y最大，在y最大的点中x最大）
        // 先过滤出右半部分的点（x > avgX）
        std::vector<cv::Point2f> rightHalfPoints;
        for (const auto& point : contour) {
            if (point.x > avgX) {
                rightHalfPoints.push_back(point);
            }
        }

        if (rightHalfPoints.empty()) {
            qDebug() << "警告：右半部分轮廓点为空，使用全部轮廓点";
            rightHalfPoints = contour;
        }

        // 在右半部分点中寻找右下角的点（y最大，在y最大的点中x最大）
        startPoint = *std::min_element(rightHalfPoints.begin(), rightHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.y == b.y) return a.x > b.x;  // y相同时，x大的优先
                                           return a.y > b.y;                 // 优先比较y坐标（大的优先）
                                       });
        break;
    }

    case OpeningDirection::LEFT: {
        // 开口向左：取左下角（x最小，在x最小的点中y最大）
        // 先过滤出下半部分的点（y > avgY）
        std::vector<cv::Point2f> bottomHalfPoints;
        for (const auto& point : contour) {
            if (point.y > avgY) {
                bottomHalfPoints.push_back(point);
            }
        }

        if (bottomHalfPoints.empty()) {
            qDebug() << "警告：下半部分轮廓点为空，使用全部轮廓点";
            bottomHalfPoints = contour;
        }

        // 在下半部分点中寻找左下角的点（x最小，在x最小的点中y最大）
        startPoint = *std::min_element(bottomHalfPoints.begin(), bottomHalfPoints.end(),
                                       [](const cv::Point2f& a, const cv::Point2f& b) {
                                           if (a.x == b.x) return a.y > b.y;  // x相同时，y大的优先
                                           return a.x < b.x;                 // 优先比较x坐标
                                       });
        break;
    }
    default:
        qDebug() << "警告：未知的开口方向";
        return cv::Point2f(-1, -1);
    }

    return startPoint;
}

/**
* @brief 获取点在轮廓点集中的索引
* @param point 要查找的点
* @param tolerance 容差范围，用于判断两点是否相同
* @return 点的索引，如果未找到则返回-1
*/
int ContourCurve::calculatePointIndex(const cv::Point2f &point,
                                      const std::vector<cv::Point2f> &contour,
                                      float tolerance)
{
    // 优先在亚像素级轮廓中查找
    if (!contour.empty()) {
        for (int i = 0; i < contour.size(); ++i) {
            if (std::abs(contour[i].x - point.x) < tolerance &&
                std::abs(contour[i].y - point.y) < tolerance) {
                return i;
            }
        }
    }
    // 如果都未找到，返回-1表示未找到
    qDebug() << "警告：未找到点 (" << point.x << ", " << point.y << ") 在轮廓点集中";
    return -1;
}

/**
* @brief 按最近邻算法对轮廓点进行排序
* @param contour 输入轮廓点集
* @return 排序后的轮廓点集
*/
std::vector<cv::Point2f> ContourCurve::sortContourByNearestNeighbor(const std::vector<cv::Point2f>& contour, int firstPointIdx)
{
    if (contour.empty()) {
        return std::vector<cv::Point2f>();
    }

    std::vector<cv::Point2f> sortedContour;
    std::vector<bool> visited(contour.size(), false);

    // 从第一个点开始
    int currentIndex = firstPointIdx;
    sortedContour.push_back(contour[currentIndex]);
    visited[currentIndex] = true;

    // 继续排序剩余的点
    while (sortedContour.size() < contour.size()) {
        double minDistance = std::numeric_limits<double>::max();
        int nearestIndex = -1;

        // 寻找距离当前点最近的点
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
            break; // 没有找到未访问的点
        }
    }

    return sortedContour;
}

/**
* @brief 对轮廓按开口方向确定起始点，并按逆时针排序，可理解为扫描
*/
void ContourCurve::sortContour() {
    m_startPoint = calculateStartPointByOpeningDirection(m_openingDirection, m_deduplicatedSubpixelContour);
    int pIndex = calculatePointIndex(m_startPoint, m_deduplicatedSubpixelContour);
    m_sortedSubpixelContour = sortContourByNearestNeighbor(m_deduplicatedSubpixelContour, pIndex);
}

/**
* @brief 检测单条轮廓的角点
* @return 检测到的角点集合
*/
void ContourCurve::detectCornerPoints()
{
    if (m_sortedSubpixelContour.empty()) {
        return;
    }

    // 使用多边形拟合方法检测角点（可以根据需要切换其他方法）
    m_cornerPoints = detectCornerPointsByDouglasPeucker(m_sortedSubpixelContour);
}

/**
* @brief 使用曲率方法检测角点
* @param contour 输入轮廓
* @param curvatureThreshold 曲率阈值，用于判断是否为角点
* @return 检测到的角点
*/
std::vector<cv::Point2f> ContourCurve::detectCornerPointsByCurvature(const std::vector<cv::Point2f>& contour,
                                                                     double curvatureThreshold) const
{
    if (contour.size() < 3) {
        return {};
    }

    std::vector<cv::Point2f> cornerPoints;
    const int windowSize = 5; // 曲率计算窗口大小

    for (int i = windowSize; i < contour.size() - windowSize; ++i) {
        cv::Point2f prev = contour[i - windowSize];
        cv::Point2f curr = contour[i];
        cv::Point2f next = contour[i + windowSize];

        double curvature = calculateCurvature(prev, curr, next);

        // 如果曲率超过阈值，认为是角点
        if (curvature > curvatureThreshold) {
            cornerPoints.push_back(curr);
        }
    }

    return cornerPoints;
}

/**
* @brief 使用Douglas-Peucker算法检测角点
* @param contour 输入轮廓
* @param epsilon 简化阈值
* @return 检测到的角点
*/
std::vector<cv::Point2f> ContourCurve::detectCornerPointsByDouglasPeucker(const std::vector<cv::Point2f>& contour,
                                                                          double epsilon) const
{
    if (contour.size() < 3) {
        return {};
    }

    // 将Point2f转换为Point用于OpenCV的approxPolyDP
    std::vector<cv::Point> intContour;
    for (const auto& pt : contour) {
        intContour.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
    }

    std::vector<cv::Point> approx;
    cv::approxPolyDP(intContour, approx, epsilon, false);

    // 转换回Point2f
    std::vector<cv::Point2f> cornerPoints;
    for (const auto& pt : approx) {
        cornerPoints.push_back(cv::Point2f(static_cast<float>(pt.x), static_cast<float>(pt.y)));
    }

    return cornerPoints;
}

/**
* @brief 使用Harris角点检测方法
* @param contour 输入轮廓
* @param threshold Harris响应阈值
* @return 检测到的角点
*/
std::vector<cv::Point2f> ContourCurve::detectCornerPointsByHarris(const std::vector<cv::Point2f>& contour,
                                                                  double threshold) const
{
    if (contour.empty()) {
        return {};
    }

    // 创建图像用于Harris检测
    cv::Rect boundingRect = cv::boundingRect(contour);
    cv::Mat image = cv::Mat::zeros(boundingRect.height + 20, boundingRect.width + 20, CV_8UC1);

    // 绘制轮廓
    std::vector<cv::Point> intContour;
    for (const auto& pt : contour) {
        intContour.push_back(cv::Point(static_cast<int>(pt.x - boundingRect.x + 10),
                                       static_cast<int>(pt.y - boundingRect.y + 10)));
    }

    cv::polylines(image, intContour, false, cv::Scalar(255), 1);

    // Harris角点检测
    cv::Mat corners, cornersNorm;
    cv::cornerHarris(image, corners, 2, 3, 0.04);
    cv::normalize(corners, cornersNorm, 0, 255, cv::NORM_MINMAX, CV_32FC1);

    std::vector<cv::Point2f> cornerPoints;
    for (int i = 0; i < cornersNorm.rows; i++) {
        for (int j = 0; j < cornersNorm.cols; j++) {
            if (cornersNorm.at<float>(i, j) > threshold * 255) {
                cornerPoints.push_back(cv::Point2f(static_cast<float>(j + boundingRect.x - 10),
                                                   static_cast<float>(i + boundingRect.y - 10)));
            }
        }
    }

    return cornerPoints;
}

/**
* @brief 计算三点之间的曲率
* @param prev 前一个点
* @param curr 当前点
* @param next 后一个点
* @return 曲率值
*/
double ContourCurve::calculateCurvature(const cv::Point2f& prev, const cv::Point2f& curr, const cv::Point2f& next) const
{
    // 计算向量
    cv::Point2f v1 = curr - prev;
    cv::Point2f v2 = next - curr;

    // 计算向量长度
    double len1 = cv::norm(v1);
    double len2 = cv::norm(v2);

    if (len1 < 1e-10 || len2 < 1e-10) {
        return 0.0;
    }

    // 归一化向量
    v1 /= len1;
    v2 /= len2;

    // 计算夹角余弦值
    double cosAngle = v1.dot(v2);

    // 限制在有效范围内
    cosAngle = std::max(-1.0, std::min(1.0, cosAngle));

    // 计算夹角（弧度）
    double angle = std::acos(cosAngle);

    // 曲率与夹角成正比
    return angle;
}


/**
* @brief 移除角点附近指定半径范围内的轮廓点
* @param contour 输入轮廓点集
* @param cornerPoints 角点集合
* @param radius 移除半径（像素），默认为10
* @return 移除角点附近点后的轮廓点集
*/
std::vector<cv::Point2f> ContourCurve::removePointsNearCorners(const std::vector<cv::Point2f>& contour,
                                                               const std::vector<cv::Point2f>& cornerPoints,
                                                               double radius) const
{
    if (contour.empty() || cornerPoints.empty()) {
        return contour;
    }

    std::vector<cv::Point2f> filteredContour;
    double radiusSquared = radius * radius;  // 使用平方距离避免开方运算

    for (const auto& point : contour) {
        bool isNearCorner = false;

        // 检查当前点是否在任何一个角点的半径范围内
        for (const auto& corner : cornerPoints) {
            double dx = point.x - corner.x;
            double dy = point.y - corner.y;
            double distanceSquared = dx * dx + dy * dy;

            if (distanceSquared <= radiusSquared) {
                isNearCorner = true;
                break;  // 如果靠近任何一个角点，就跳过该点
            }
        }

        // 如果点不在任何角点的半径范围内，则保留
        if (!isNearCorner) {
            filteredContour.push_back(point);
        }
    }

    return filteredContour;
}

/**
* @brief 移除角点和干扰点区域
*/
void ContourCurve::removeCorners() {
    m_noConersContour = removePointsNearCorners(m_sortedSubpixelContour, m_cornerPoints, 15);
}

/**
* @brief 对轮廓进行线段分割
* @param contour 待分割轮廓点集
* @param segmentedContours 分割后的轮廓点集
*/
void ContourCurve::segmentContour(const std::vector<cv::Point2f> &contour, std::vector<std::vector<cv::Point2f>> &segmentedContours)
{
    std::vector<cv::Vec4f> lines;
    double threshold = 8;
    int maxIterations = 100;
    ContourSegment cs{contour};
    cs.sequentialRansac3Times(contour, segmentedContours, lines, threshold, maxIterations);
}

/**
* @brief 对轮廓进行分割
*/
void ContourCurve::segment()
{
    if (!m_noConersContour.empty())
    {
        segmentContour(m_noConersContour, m_segmentedSubpixelContours);
    } else {
        return;
    }
}

/**
* @brief 判断点A是否在点B的顺时针方向（相对于参考点）
* @param pointA 第一个点
* @param pointB 第二个点
* @param referencePoint 参考点
* @return 如果点A在点B的顺时针方向返回true，否则返回false
*/
bool ContourCurve::isPointClockwiseTo(const cv::Point2f& pointA, const cv::Point2f& pointB, const cv::Point2f& referencePoint) const
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

/**
* @brief 将分割后的轮廓进行逆时针排序，返回键值对格式
* @param segmentedContours 分割后的轮廓集合
* @param referencePoint 参考点，用于计算轮廓的相对角度
* @return 键值对，键为1、2、3表示逆时针的第1、2、3条轮廓，值为对应的轮廓点集合
*/
std::map<int, std::vector<cv::Point2f>> ContourCurve::sortContoursCounterClockwise(const std::vector<std::vector<cv::Point2f>>& segmentedContours,
                                                                                   const cv::Point2f& referencePoint) const
{
    std::map<int, std::vector<cv::Point2f>> sortedContours;

    if (segmentedContours.empty()) {
        return sortedContours;
    }

    // 1. 从每段轮廓中选取中间点
    std::vector<std::pair<cv::Point2f, std::vector<cv::Point2f>>> contoursWithMidPoints;
    for (const auto& contour : segmentedContours) {
        if (contour.empty()) {
            continue;
        }

        // 计算轮廓的中间点
        int midIndex = static_cast<int>(contour.size() / 2);
        cv::Point2f midPoint = contour[midIndex];

        contoursWithMidPoints.push_back({midPoint, contour});
    }

    if (contoursWithMidPoints.empty()) {
        return sortedContours;
    }

    // 2. 对中间点进行逆时针排序（相对于参考点）
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
                                            [](const std::pair<cv::Point2f, std::vector<cv::Point2f>>& a,
                                               const std::pair<cv::Point2f, std::vector<cv::Point2f>>& b) {
                                                return a.second.size() < b.second.size();
                                            });

    if (maxPointContour == circularList.end()) {
        return sortedContours;
    }

    // 5. 确定三条轮廓的位置
    int maxIndex = std::distance(circularList.begin(), maxPointContour);
    int n = static_cast<int>(circularList.size());

    // 计算前一项（第一条轮廓）- 顺时针方向的前一个
    int firstIndex = (maxIndex - 1 + n) % n;

    // 计算后一项（第三条轮廓）- 顺时针方向的后一个
    int thirdIndex = (maxIndex + 1) % n;

    // 6. 将三条轮廓存入map
    sortedContours[1] = circularList[firstIndex].second;  // 第一条轮廓（顺时针方向的前一个）
    sortedContours[2] = circularList[maxIndex].second;    // 第二条轮廓（点数最多的）
    sortedContours[3] = circularList[thirdIndex].second;  // 第三条轮廓（顺时针方向的后一个）

    return sortedContours;
}

/**
* @brief 对分割后的轮廓进行整体排序
*/
void ContourCurve::sortSegmentedContours() {
    if (m_segmentedSubpixelContours.empty()) return;
    m_counterClockwiseContours = sortContoursCounterClockwise(m_segmentedSubpixelContours, m_centroid);
}

/**
* @brief 对所有排序后的分割轮廓进行直线拟合
*/
void ContourCurve::calculateLines()
{
    if (m_counterClockwiseContours.empty()) return;

    // 清空原有的直线拟合结果
    m_lineSegments.clear();
    // 使用m_counterClockwiseContours进行直线拟合
    if (!m_counterClockwiseContours.empty()) {
        for (const auto& [index, contour] : m_counterClockwiseContours) {
            LineSeg ls;
            ls.initializeFromPoints(contour);
            m_lineSegments[index] = ls;
        }
    }
}

/**
* @brief 对所有排序后的分割轮廓进行样条曲线拟合
*/
void ContourCurve::calculateBSplines()
{
    if (m_counterClockwiseContours.empty()) return;
    // 使用逆时针排序的m_counterClockwiseContours进行拟合
    for (const auto& [index, contour] : m_counterClockwiseContours) {
        CurveSeg curve;
        curve.initializeFromPoints(contour);
        curve.fitSplineCurve();
        m_curveSegments[index] = curve;
    }
}

/**
* @brief calculateLineIntersection 计算两条直线的交点
* @param line1 直线1方程
* @param line2 直线2方程
* @return 交点坐标
*/
cv::Point2f ContourCurve::calculateLineIntersection(const cv::Vec4f &line1, const cv::Vec4f &line2) {
    // 提取直线参数
    float vx1 = line1[0], vy1 = line1[1], x01 = line1[2], y01 = line1[3];
    float vx2 = line2[0], vy2 = line2[1], x02 = line2[2], y02 = line2[3];

    // 检查两条直线是否平行
    float cross = vx1 * vy2 - vy1 * vx2;
    if (std::abs(cross) < 1e-10) {
        // 直线平行或重合，返回无效点
        qDebug() << "警告：两条直线平行或重合，无法计算交点";
        return cv::Point2f(-1, -1);
    }

    // 使用参数方程求解交点
    // 直线1: (x, y) = (x01, y01) + t1 * (vx1, vy1)
    // 直线2: (x, y) = (x02, y02) + t2 * (vx2, vy2)

    // 解方程组:
    // x01 + t1 * vx1 = x02 + t2 * vx2
    // y01 + t1 * vy1 = y02 + t2 * vy2

    // 整理得:
    // t1 * vx1 - t2 * vx2 = x02 - x01
    // t1 * vy1 - t2 * vy2 = y02 - y01

    float dx = x02 - x01;
    float dy = y02 - y01;

    // 使用克莱姆法则求解t1
    float t1 = (dx * vy2 - dy * vx2) / cross;

    // 计算交点坐标
    float intersectX = x01 + t1 * vx1;
    float intersectY = y01 + t1 * vy1;

    return cv::Point2f(intersectX, intersectY);
}

/**
* @brief calculateEndPointsByFittedLines 通过拟合的直线求属于拼缝的端点
*/
void ContourCurve::calculateEndPointsByFittedLines()
{
    if (m_lineSegments.empty()) return;
    cv::Vec4f line1 = m_lineSegments[1].getLineEquation();
    cv::Vec4f line2 = m_lineSegments[2].getLineEquation();
    cv::Vec4f line3 = m_lineSegments[3].getLineEquation();
    cv::Point2f cornerPoint1 = calculateLineIntersection(line1, line2);
    m_endPoints.push_back(cornerPoint1);
    cv::Point2f cornerPoint2 = calculateLineIntersection(line3, line2);
    m_endPoints.push_back(cornerPoint2);
    m_lines.push_back(line1);
    m_lines.push_back(line2);
    m_lines.push_back(line3);
    m_lines.push_back(line2);
}

/**
* @brief calculateEndPointsByFittedLines 通过拟合的样条曲线求属于拼缝的端点
*/
void ContourCurve::calculateEndPointsByFittedCurves()
{
    // 先按逆时针标记线，再获得每条线端点的逆时针标记，最后根据这个确定选取轮廓的哪端切线进行计算
    // 清空之前的计算结果
    m_lines.clear();
    m_endPoints.clear();

    // 优先使用逆时针排序的曲线段
    if (!m_curveSegments.empty()) {
        // 获取三条曲线段
        CurveSeg& curve1 = m_curveSegments[1];  // 第一条轮廓（逆时针方向的前一个）
        CurveSeg& curve2 = m_curveSegments[2];  // 第二条轮廓（点数最多的）
        CurveSeg& curve3 = m_curveSegments[3];  // 第三条轮廓（逆时针方向的后一个）

        // 使用质心作为参考点
        cv::Point2f referencePoint(m_centroid.x, m_centroid.y);

        // 对每条曲线段的端点进行逆时针排序
        std::pair<EndpointInfo, EndpointInfo> sortedEndpoints1 = curve1.sortEndpoints(referencePoint);
        std::pair<EndpointInfo, EndpointInfo> sortedEndpoints2 = curve2.sortEndpoints(referencePoint);
        std::pair<EndpointInfo, EndpointInfo> sortedEndpoints3 = curve3.sortEndpoints(referencePoint);

        // 键为1的曲线：取相对于参考点更逆时针的端点（即排序后的第一个端点）
        EndpointInfo endpoint1_ccw = sortedEndpoints1.first;  // 更逆时针的端点
        // 键为2的曲线：取相对于参考点更顺时针的端点（即排序后的第二个端点）
        EndpointInfo endpoint2_cw = sortedEndpoints2.second;  // 更顺时针的端点
        // 键为3的曲线：取相对于参考点更顺时针的端点（即排序后的第一个端点）
        EndpointInfo endpoint3_ccw = sortedEndpoints3.second;  // 更顺时针的端点
        // 键为2的曲线：取相对于参考点更逆时针的端点（即排序后的第一个端点）
        EndpointInfo endpoint2_ccw = sortedEndpoints2.first;  // 更逆时针的端点

        // 获取对应端点的切线
        cv::Vec4f tangent1 = curve1.getTangent(endpoint1_ccw.u);  // 键为1的曲线更逆时针端点的切线
        cv::Vec4f tangent2_cw = curve2.getTangent(endpoint2_cw.u);  // 键为2的曲线更顺时针端点的切线
        cv::Vec4f tangent3 = curve3.getTangent(endpoint3_ccw.u);  // 键为3的曲线更顺时针端点的切线
        cv::Vec4f tangent2_ccw = curve2.getTangent(endpoint2_ccw.u);  // 键为2的曲线更逆时针端点的切线

        // 保存切线用于后续使用
        m_lines.push_back(tangent1);
        m_lines.push_back(tangent2_cw);
        m_lines.push_back(tangent3);
        m_lines.push_back(tangent2_ccw);

        // 计算交点：键为1的曲线更逆时针端点的切线与键为2的曲线更顺时针端点的切线求交点
        cv::Point2f cornerPoint1 = calculateLineIntersection(tangent1, tangent2_cw);
        // 计算交点：键为3的曲线更顺时针端点的切线与键为2的曲线更逆时针端点的切线求交点
        cv::Point2f cornerPoint2 = calculateLineIntersection(tangent3, tangent2_ccw);

        m_endPoints.push_back(cornerPoint1);
        m_endPoints.push_back(cornerPoint2);

        qDebug() << "使用逆时针排序曲线段计算端点完成";
        qDebug() << "端点1坐标: (" << cornerPoint1.x << ", " << cornerPoint1.y << ")";
        qDebug() << "端点2坐标: (" << cornerPoint2.x << ", " << cornerPoint2.y << ")";

    } else {
        qDebug() << "警告：没有可用的曲线段数据，无法计算端点";
    }
}

/******************************
 *********拷贝控制成员*********
 ******************************/

// 拷贝构造函数
ContourCurve::ContourCurve(const ContourCurve& other)
    : m_pixelContour(other.m_pixelContour),
    m_subpixelContour(other.m_subpixelContour),
    m_openingDirection(other.m_openingDirection),
    m_deduplicatedPixelContour(other.m_deduplicatedPixelContour),
    m_deduplicatedSubpixelContour(other.m_deduplicatedSubpixelContour),
    m_startPoint(other.m_startPoint),
    m_sortedSubpixelContour(other.m_sortedSubpixelContour),
    m_cornerPoints(other.m_cornerPoints),
    m_noConersContour(other.m_noConersContour),
    m_segmentedPixelContours(other.m_segmentedPixelContours),
    m_segmentedSubpixelContours(other.m_segmentedSubpixelContours),
    m_counterClockwiseContours(other.m_counterClockwiseContours),
    m_lineSegments(other.m_lineSegments),
    m_curveSegments(other.m_curveSegments),
    m_lines(other.m_lines),
    m_endPoints(other.m_endPoints),
    m_boundingRect(other.m_boundingRect),
    m_area(other.m_area),
    m_perimeter(other.m_perimeter),
    m_aspectRatio(other.m_aspectRatio),
    m_centroid(other.m_centroid),
    m_approxPolygon(other.m_approxPolygon),
    m_approxError(other.m_approxError)
{}

// 拷贝赋值运算符
ContourCurve& ContourCurve::operator=(const ContourCurve& other)
{
    if (this != &other) {
        m_pixelContour = other.m_pixelContour;
        m_subpixelContour = other.m_subpixelContour;
        m_openingDirection = other.m_openingDirection;
        m_deduplicatedPixelContour = other.m_deduplicatedPixelContour;
        m_deduplicatedSubpixelContour = other.m_deduplicatedSubpixelContour;
        m_startPoint = other.m_startPoint;
        m_sortedSubpixelContour = other.m_sortedSubpixelContour;
        m_cornerPoints = other.m_cornerPoints;
        m_noConersContour = other.m_noConersContour;
        m_segmentedPixelContours = other.m_segmentedPixelContours;
        m_segmentedSubpixelContours = other.m_segmentedSubpixelContours;
        m_counterClockwiseContours = other.m_counterClockwiseContours;
        m_lineSegments = other.m_lineSegments;
        m_lineSegments = other.m_lineSegments;
        m_curveSegments = other.m_curveSegments;
        m_curveSegments = other.m_curveSegments;
        m_lines = other.m_lines;
        m_endPoints = other.m_endPoints;
        m_boundingRect = other.m_boundingRect;
        m_area = other.m_area;
        m_perimeter = other.m_perimeter;
        m_aspectRatio = other.m_aspectRatio;
        m_centroid = other.m_centroid;
        m_approxPolygon = other.m_approxPolygon;
        m_approxError = other.m_approxError;
    }
    return *this;
}

// 移动构造函数
ContourCurve::ContourCurve(ContourCurve&& other) noexcept
    : m_pixelContour(std::move(other.m_pixelContour)),
    m_subpixelContour(std::move(other.m_subpixelContour)),
    m_openingDirection(std::move(other.m_openingDirection)),
    m_deduplicatedPixelContour(std::move(other.m_deduplicatedPixelContour)),
    m_deduplicatedSubpixelContour(std::move(other.m_deduplicatedSubpixelContour)),
    m_startPoint(std::move(other.m_startPoint)),
    m_sortedSubpixelContour(std::move(other.m_sortedSubpixelContour)),
    m_cornerPoints(std::move(other.m_cornerPoints)),
    m_noConersContour(std::move(other.m_noConersContour)),
    m_segmentedPixelContours(std::move(other.m_segmentedPixelContours)),
    m_segmentedSubpixelContours(std::move(other.m_segmentedSubpixelContours)),
    m_counterClockwiseContours(std::move(other.m_counterClockwiseContours)),
    m_lineSegments(std::move(other.m_lineSegments)),
    m_curveSegments(std::move(other.m_curveSegments)),
    m_lines(std::move(other.m_lines)),
    m_endPoints(std::move(other.m_endPoints)),
    m_boundingRect(std::move(other.m_boundingRect)),
    m_area(std::move(other.m_area)),
    m_perimeter(std::move(other.m_perimeter)),
    m_aspectRatio(std::move(other.m_aspectRatio)),
    m_centroid(std::move(other.m_centroid)),
    m_approxPolygon(std::move(other.m_approxPolygon)),
    m_approxError(std::move(other.m_approxError))
{
    // 清空源对象的资源
    other.clear();
}

// 移动赋值运算符
ContourCurve& ContourCurve::operator=(ContourCurve&& other) noexcept
{
    if (this != &other) {
        m_pixelContour = std::move(other.m_pixelContour);
        m_subpixelContour = std::move(other.m_subpixelContour);
        m_openingDirection = std::move(other.m_openingDirection);
        m_deduplicatedPixelContour = std::move(other.m_deduplicatedPixelContour);
        m_deduplicatedSubpixelContour = std::move(other.m_deduplicatedSubpixelContour);
        m_startPoint = std::move(other.m_startPoint);
        m_sortedSubpixelContour = std::move(other.m_sortedSubpixelContour);
        m_cornerPoints = std::move(other.m_cornerPoints);
        m_noConersContour = std::move(other.m_noConersContour);
        m_segmentedPixelContours = std::move(other.m_segmentedPixelContours);
        m_segmentedSubpixelContours = std::move(other.m_segmentedSubpixelContours);
        m_counterClockwiseContours = std::move(other.m_counterClockwiseContours);
        m_lineSegments = std::move(other.m_lineSegments);
        m_lineSegments = std::move(other.m_lineSegments);
        m_curveSegments = std::move(other.m_curveSegments);
        m_curveSegments = std::move(other.m_curveSegments);
        m_lines = std::move(other.m_lines);
        m_endPoints = std::move(other.m_endPoints);
        m_boundingRect = std::move(other.m_boundingRect);
        m_area = std::move(other.m_area);
        m_perimeter = std::move(other.m_perimeter);
        m_aspectRatio = std::move(other.m_aspectRatio);
        m_centroid = std::move(other.m_centroid);
        m_approxPolygon = std::move(other.m_approxPolygon);
        m_approxError = std::move(other.m_approxError);

        // 清空源对象的资源
        other.clear();
    }
    return *this;
}













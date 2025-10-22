#include "contour_curve.h"
#include <QDebug>

/******************************
 *********ContourCurve*******
 ******************************/
ContourCurve::ContourCurve(): m_area(0.0), m_perimeter(0.0), m_aspectRatio(0.0),m_approxError(0.0) {}

/**
* @brief 从像素级轮廓点集初始化
* @param contour 输入轮廓像素点集
*/
void ContourCurve::initializePixelContour(const std::vector<cv::Point>& contour) {
    m_pixelContour = contour;
    m_deduplicatedPixelContour = removeDuplicateContourPoints(m_pixelContour);
    calculateBasicFeatures();
    m_openingDirection = calculateOpeningDirection();
    segment();
}

/**
* @brief 从像素级轮廓点集初始化
* @param contour 输入轮廓亚像素点集
*/
void ContourCurve::initializeSubpixelContour(const std::vector<cv::Point2f>& contour)
{
    m_subpixelContour = contour;
    m_deduplicatedSubpixelContour = removeDuplicateContourPoints(m_subpixelContour);
    m_openingDirection = calculateOpeningDirection();
    sortContour();
    calculateBasicFeatures();
    segment();
    calculateLines();
    calculateCornerPoints();
    calculateBSplines();
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
    m_approxPolygon.clear();
    m_sortedSubpixelContour.clear();

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
    return !m_pixelContour.empty();
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
OpeningDirection ContourCurve::calculateOpeningDirection()
{
    // 优先使用亚像素级坐标，如果为空则使用像素级坐标
    if (!m_deduplicatedSubpixelContour.empty()) {
        return calculateOpeningDirectionImpl(m_deduplicatedSubpixelContour);
    } else if (!m_deduplicatedPixelContour.empty()) {
        return calculateOpeningDirectionImpl(m_deduplicatedPixelContour);
    }

    return OpeningDirection::UNKNOWN;
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
* @brief 对轮廓进行线段分割
*/
void ContourCurve::segment()
{
    if (!m_sortedSubpixelContour.empty())
    {
        segmentContour(m_sortedSubpixelContour, m_segmentedSubpixelContours);
    } else {
        return;
    }
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

void ContourCurve::calculateLines()
{
    if (m_segmentedSubpixelContours.empty()) return;

    for (auto contour : m_segmentedSubpixelContours)
    {
        LineSeg ls;
        ls.initializeFromPoints(contour);
        m_lineSegments.push_back(ls);
    }
}

// 计算两条直线的交点
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

void ContourCurve::calculateCornerPoints()
{
    if (m_lineSegments.empty()) return;
    cv::Point2f cornerPoint1 = calculateLineIntersection(m_lineSegments[0].getLineEquation(), m_lineSegments[1].getLineEquation());
    m_cornerPoints.push_back(cornerPoint1);
    cv::Point2f cornerPoint2 = calculateLineIntersection(m_lineSegments[0].getLineEquation(), m_lineSegments[2].getLineEquation());
    m_cornerPoints.push_back(cornerPoint2);
}

// 计算拟合的B样条曲线
void ContourCurve::calculateBSplines() {
    for (auto& contour : m_segmentedSubpixelContours) {
        CurveSeg curve;
        curve.initializeFromPoints(contour);
        curve.fitSplineCurve();
        m_curveSegments.push_back(curve);
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

void ContourCurve::sortContour() {
    m_startPoint = calculateStartPointByOpeningDirection(m_openingDirection, m_deduplicatedSubpixelContour);
    int pIndex = calculatePointIndex(m_startPoint, m_deduplicatedSubpixelContour);
    m_sortedSubpixelContour = sortContourByNearestNeighbor(m_deduplicatedSubpixelContour, pIndex);
}






















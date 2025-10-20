#include "contourCurve.h"
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
    calculateBasicFeatures();
    m_openingDirection = calculateOpeningDirection();
    segment();
    calculateLines();
    calculateCornerPoints();
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
* @brief 对轮廓进行线段分割
*/
void ContourCurve::segment()
{
    std::vector<cv::Vec4f> lines;
    double threshold = 8;
    int maxIterations = 100;
    if (!m_deduplicatedPixelContour.empty())
    {
        ContourSegment cs{m_deduplicatedPixelContour};
        cs.sequentialRansac3Times(m_deduplicatedPixelContour, m_segmentedPixelContours, lines, threshold, maxIterations);
    }
    if (!m_deduplicatedSubpixelContour.empty())
    {
        ContourSegment cs{m_deduplicatedSubpixelContour};
        cs.sequentialRansac3Times(m_deduplicatedSubpixelContour, m_segmentedSubpixelContours, lines, threshold, maxIterations);
    }
}

void ContourCurve::calculateLines()
{
    if (m_segmentedSubpixelContours.empty()) return;

    for (auto contour : m_segmentedSubpixelContours)
    {
        LineSegment ls;
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






























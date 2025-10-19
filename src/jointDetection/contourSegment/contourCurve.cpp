#include "contourCurve.h"

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
    m_deduplicatedSubpixelContour = removeDuplicateContourPoints(m_subpixelContour);
    calculateBasicFeatures();
    m_openingDirection = calculateOpeningDirection();
    segment();
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




































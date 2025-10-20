#include "lineSeg.h"

/******************************
 *********LineSeg*********
 ******************************/
LineSeg::LineSeg(): m_length(0.0), m_angle(0.0) {}

/**
* @brief 从点集初始化线段特征
* @param points 输入像素点集
*/
void LineSeg::initializeFromPoints(const std::vector<cv::Point>& pixelPoints) {
    m_pixelPoints = pixelPoints;
    calculateBasicFeatures();
    m_lineEquation = fitLine(m_pixelPoints);
}

/**
* @brief 从点集初始化线段特征
* @param points 输入亚像素点集
*/
void LineSeg::initializeFromPoints(const std::vector<cv::Point2f>& subpixelPoints) {
    m_subpixelPoints = subpixelPoints;
    calculateBasicFeatures();
    m_lineEquation = fitLine(m_subpixelPoints);
}

/**
* @brief 计算基本几何特征
*/
void LineSeg::calculateBasicFeatures() {
    if (m_pixelPoints.empty()) return;

    // 计算起点和终点
    m_startPoint = m_pixelPoints.front();
    m_endPoint = m_pixelPoints.back();

    // 计算长度
    m_length = cv::norm(cv::Point2f(m_endPoint - m_startPoint));

    // 计算角度
    cv::Point2f direction = cv::Point2f(m_endPoint - m_startPoint);
    if (m_length > 0) {
        m_angle = std::atan2(direction.y, direction.x);
    }

    // 计算中点
    m_midpoint = cv::Point((m_startPoint.x + m_endPoint.x) / 2, (m_startPoint.y + m_endPoint.y) / 2);
}

/**
* @brief 清空所有特征数据
*/
void LineSeg::clear() {
    m_pixelPoints.clear();
    m_subpixelPoints.clear();

    m_lineEquation = cv::Vec4f(0, 0, 0, 0);
    m_length = 0.0;
    m_angle = 0.0;
}

/**
* @brief 检查线段是否有效
* @return 如果点集不为空且长度大于0则返回true
*/
bool LineSeg::isValid() const
{
    return (m_lineEquation != cv::Vec4f{0,0,0,0});
}

/**
* @brief 获取线段特征摘要信息
* @return 特征摘要字符串
*/
std::string LineSeg::getSummary() const {
    std::string summary;
    summary += "线段点数: " + std::to_string(m_pixelPoints.size()) + "\n";
    summary += "线段长度: " + std::to_string(m_length) + "\n";
    summary += "线段角度: " + std::to_string(m_angle * 180 / CV_PI) + "度\n";
    summary += "起点: (" + std::to_string(m_startPoint.x) + ", " + std::to_string(m_startPoint.y) + ")\n";
    summary += "终点: (" + std::to_string(m_endPoint.x) + ", " + std::to_string(m_endPoint.y) + ")\n";
    return summary;
}

cv::Vec4f LineSeg::fitLine(const std::vector<cv::Point> &points)
{
    if (points.empty()) {
        return cv::Vec4f(0, 0, 0, 0);
    }

    cv::Vec4f lineParams;
    cv::fitLine(points, lineParams, cv::DIST_L2, 0, 0.01, 0.01);

    // lineParams格式: [vx, vy, x0, y0]
    // 其中(vx, vy)是单位方向向量，(x0, y0)是直线上的一个点
    return lineParams;
}

cv::Vec4f LineSeg::fitLine(const std::vector<cv::Point2f> &points)
{
    if (points.empty()) {
        return cv::Vec4f(0, 0, 0, 0);
    }

    cv::Vec4f lineParams;
    cv::fitLine(points, lineParams, cv::DIST_L2, 0, 0.01, 0.01);

    // lineParams格式: [vx, vy, x0, y0]
    // 其中(vx, vy)是单位方向向量，(x0, y0)是直线上的一个点
    return lineParams;
}
































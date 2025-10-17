#include "contourCurve.h"

/******************************
 *********Line*********
 ******************************/
Line::Line(): fitError(0.0), length(0.0), angle(0.0) {}

/**
* @brief 从点集初始化线段特征
* @param points 输入点集
*/
void Line::initializeFromPoints(const std::vector<cv::Point>& points) {
    pixelPoints = points;
    calculateBasicFeatures();
}

/**
* @brief 计算基本几何特征
*/
void Line::calculateBasicFeatures() {
    if (pixelPoints.empty()) return;

    // 计算起点和终点
    startPoint = pixelPoints.front();
    endPoint = pixelPoints.back();

    // 计算长度
    length = cv::norm(cv::Point2f(endPoint - startPoint));

    // 计算角度
    cv::Point2f direction = cv::Point2f(endPoint - startPoint);
    if (length > 0) {
        angle = std::atan2(direction.y, direction.x);
    }

    // 计算中点
    midpoint = cv::Point((startPoint.x + endPoint.x) / 2, (startPoint.y + endPoint.y) / 2);
}

/**
* @brief 清空所有特征数据
*/
void Line::clear() {
    pixelPoints.clear();
    subpixelPoints.clear();
    inlierPoints.clear();

    lineEquation = cv::Vec4f(0, 0, 0, 0);
    fitError = 0.0;
    length = 0.0;
    angle = 0.0;
}

/**
* @brief 检查线段是否有效
* @return 如果点集不为空且长度大于0则返回true
*/
bool Line::isValid() const
{
    return !pixelPoints.empty() && length > 0;
}

/**
* @brief 获取线段特征摘要信息
* @return 特征摘要字符串
*/
std::string Line::getSummary() const {
    std::string summary;
    summary += "线段点数: " + std::to_string(pixelPoints.size()) + "\n";
    summary += "线段长度: " + std::to_string(length) + "\n";
    summary += "线段角度: " + std::to_string(angle * 180 / CV_PI) + "度\n";
    summary += "拟合误差: " + std::to_string(fitError) + "\n";
    summary += "起点: (" + std::to_string(startPoint.x) + ", " + std::to_string(startPoint.y) + ")\n";
    summary += "终点: (" + std::to_string(endPoint.x) + ", " + std::to_string(endPoint.y) + ")\n";
    return summary;
}

/******************************
 *********ContourCurve*******
 ******************************/
ContourCurve::ContourCurve(): area(0.0), perimeter(0.0), aspectRatio(0.0),approxError(0.0) {}

/**
* @brief 从像素级轮廓点集初始化
* @param contour 输入轮廓点集
*/
void ContourCurve::initializeFromContour(const std::vector<cv::Point>& contour) {
    pixelContour = contour;
    calculateBasicFeatures();
}

/**
* @brief 计算基本几何特征
*/
void ContourCurve::calculateBasicFeatures() {
    if (pixelContour.empty()) return;

    // 计算外接矩形
    boundingRect = cv::boundingRect(pixelContour);

    // 计算面积和周长
    area = cv::contourArea(pixelContour);
    perimeter = cv::arcLength(pixelContour, true);

    // 计算长宽比
    if (boundingRect.height > 0) {
        aspectRatio = static_cast<double>(boundingRect.width) / boundingRect.height;
    }

    // 计算质心
    cv::Moments moments = cv::moments(pixelContour);
    if (moments.m00 != 0) {
        centroid.x = static_cast<int>(moments.m10 / moments.m00);
        centroid.y = static_cast<int>(moments.m01 / moments.m00);
    }
}

/**
* @brief 清空所有特征数据
*/
void ContourCurve::clear() {
    pixelContour.clear();
    subpixelContour.clear();
    segmentedPixelContours.clear();
    segmentedSubpixelContours.clear();
    cornerPoints.clear();
    fittedLines.clear();
    lineInliers.clear();
    lineFitErrors.clear();
    approxPolygon.clear();

    openingDirection = OpeningDirection::UNKNOWN;
    area = 0.0;
    perimeter = 0.0;
    aspectRatio = 0.0;
    approxError = 0.0;
}

/**
* @brief 检查特征是否有效
* @return 如果轮廓不为空则返回true
*/
bool ContourCurve::isValid() const {
    return !pixelContour.empty();
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
    summary += "轮廓点数: " + std::to_string(pixelContour.size()) + "\n";
    summary += "开口方向: " + openingDirectionToString(openingDirection) + "\n";
    summary += "面积: " + std::to_string(area) + "\n";
    summary += "周长: " + std::to_string(perimeter) + "\n";
    summary += "长宽比: " + std::to_string(aspectRatio) + "\n";
    summary += "分割段数: " + std::to_string(segmentedPixelContours.size()) + "\n";
    summary += "拟合直线数: " + std::to_string(fittedLines.size()) + "\n";
    summary += "角点数: " + std::to_string(cornerPoints.size()) + "\n";
    return summary;
}

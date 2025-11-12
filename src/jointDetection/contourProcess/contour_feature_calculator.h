#ifndef CONTOUR_FEATURE_CALCULATOR_H
#define CONTOUR_FEATURE_CALCULATOR_H

#include "contour_utils.h"
/**
 * @brief 轮廓特征计算器 - 负责各种特征计算
 */
class ContourFeatureCalculator {
public:
    static OpeningDirection calculateOpeningDirection(const std::vector<cv::Point2f>& contour);
    static std::vector<cv::Point2f> removeDuplicatePoints(const std::vector<cv::Point2f>& contour);
    static cv::Point2f calculateStartPoint(OpeningDirection direction, const std::vector<cv::Point2f>& contour);
    static std::vector<cv::Point2f> sortContour(const std::vector<cv::Point2f>& contour, int startIndex);
    static std::vector<cv::Point2f> detectCornerPoints(const std::vector<cv::Point2f>& contour);
    static std::vector<cv::Point2f> removePointsNearCorners(const std::vector<cv::Point2f>& contour,
                                                            const std::vector<cv::Point2f>& cornerPoints,
                                                            double radius = 15.0);

    // 几何特征计算
    static cv::Rect calculateBoundingRect(const std::vector<cv::Point2f>& contour);
    static double calculateArea(const std::vector<cv::Point2f>& contour);
    static double calculatePerimeter(const std::vector<cv::Point2f>& contour);

private:
    static double calculateCurvature(const cv::Point2f& prev, const cv::Point2f& curr, const cv::Point2f& next);
    static std::vector<cv::Point2f> detectCornerPointsByDouglasPeucker(const std::vector<cv::Point2f>& contour, double epsilon = 10.0);
};

#endif // CONTOUR_FEATURE_CALCULATOR_H

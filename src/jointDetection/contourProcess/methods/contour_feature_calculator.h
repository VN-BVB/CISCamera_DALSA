#ifndef CONTOUR_FEATURE_CALCULATOR_H
#define CONTOUR_FEATURE_CALCULATOR_H

#include "contour_utils.h"
/**
 * @brief 轮廓特征计算器 - 负责各种特征计算
 */
class ContourFeatureCalculator {
public:
    // =============计算开口方向=============
    static OpeningDirection calculateOpeningDirection(const std::vector<cv::Point2f>& contour);

    // =============去重=============
    static std::vector<cv::Point2f> removeDuplicatePoints(const std::vector<cv::Point2f>& contour);

    // =============计算起始点=============
    static cv::Point2f calculateStartPoint(OpeningDirection direction, const std::vector<cv::Point2f>& contour);
    static cv::Point2f calculateEndPoint(OpeningDirection direction, const std::vector<cv::Point2f>& contour);

    // =============排序轮廓=============
    static std::vector<cv::Point2f> sortContour(const std::vector<cv::Point2f>& contour, int startIndex);
    static std::vector<cv::Point2f> sortContourByCentroid(const std::vector<cv::Point2f>& contour);
    static std::vector<std::pair<int, double>> getSortedCandidatesByDistance(const std::vector<cv::Point2f>& contour,
                                                                             int currentIndex,
                                                                             const std::vector<bool>& visited,
                                                                             int k);
    static int selectNextCandidate(const std::vector<cv::Point2f>& contour,
                                   int currentIndex,
                                   const std::vector<std::pair<int, double>>& candidates,
                                   const cv::Point2f& centroid);
    static int findNextPoint(const std::vector<cv::Point2f>& contour,
                             int currentIndex,
                             const std::vector<bool>& visited,
                             const cv::Point2f& centroid);
    static std::vector<cv::Point2f> sortContourByNearestNeighbor(const std::vector<cv::Point2f>& contour, int startIndex, int endIndex);

    // =============检测角点=============
    static std::vector<cv::Point2f> detectCornerPoints(const std::vector<cv::Point2f>& contour);
    static std::vector<cv::Point2f> removePointsNearCorners(const std::vector<cv::Point2f>& contour,
                                                            const std::vector<cv::Point2f>& cornerPoints,
                                                            double radius = 10.0);

private:
    // =============检测角点辅助函数=============
    static std::vector<cv::Point2f> detectCornerPointsByDouglasPeucker(const std::vector<cv::Point2f>& contour, double epsilon = 10.0);
    static std::vector<cv::Point2f> detectCornerPointsByRansac(const std::vector<cv::Point2f>& contour);
};

#endif // CONTOUR_FEATURE_CALCULATOR_H

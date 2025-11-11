#ifndef CONTOUR_SEGMENTER_H
#define CONTOUR_SEGMENTER_H

#include <opencv2/opencv.hpp>

/**
 * @brief 轮廓分割器 - 负责轮廓分割操作
 */
class ContourSegmenter {
public:
    static std::vector<std::vector<cv::Point2f>> segmentContour(const std::vector<cv::Point2f>& contour);

    static std::map<int, std::vector<cv::Point2f>> sortSegmentsCounterClockwise(const std::vector<std::vector<cv::Point2f>>& segments,
                                                                                const cv::Point2f& referencePoint);

    static void sequentialRansac3Times(const std::vector<cv::Point2f>& points,
                                       std::vector<std::vector<cv::Point2f>>& segments,
                                       std::vector<cv::Vec4f>& lines,
                                       double threshold = 0.5,
                                       int maxIterations = 100);

private:
    static bool isPointClockwiseTo(const cv::Point2f& a, const cv::Point2f& b, const cv::Point2f& reference);
    static void lineRansac(const std::vector<cv::Point2f> &points,
                           cv::Vec4f &line,
                           std::vector<cv::Point2f> &inlierPoints,
                           const double &threshold = 5,
                           const int &iterations = 100);

    // 根据点分割轮廓
    std::vector<std::vector<cv::Point2f>> segmentContourByApproxPoints(const std::vector<cv::Point2f>& contour,
                                                                       const std::vector<cv::Point2f>& approxPoints);
};
#endif // CONTOUR_SEGMENTER_H

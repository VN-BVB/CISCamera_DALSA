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

    static std::vector<std::vector<cv::Point2f>> splitContourByCorners(
        const std::vector<cv::Point2f>& contour,
        const std::vector<cv::Point2f>& cornerPoints,
        double radius = 10.0);
};
#endif // CONTOUR_SEGMENTER_H

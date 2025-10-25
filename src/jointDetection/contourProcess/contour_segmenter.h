#ifndef CONTOUR_SEGMENTER_H
#define CONTOUR_SEGMENTER_H

#include <opencv2/opencv.hpp>

/**
 * @brief 轮廓分割器 - 负责轮廓分割操作
 */
class ContourSegmenter {
public:
    static std::vector<std::vector<cv::Point2f>> segmentContour(
        const std::vector<cv::Point2f>& contour,
        const std::vector<cv::Point2f>& cornerPoints);

    static std::map<int, std::vector<cv::Point2f>> sortSegmentsCounterClockwise(
        const std::vector<std::vector<cv::Point2f>>& segments,
        const cv::Point2f& referencePoint);

private:
    static bool isPointClockwiseTo(const cv::Point2f& a, const cv::Point2f& b, const cv::Point2f& reference);
};
#endif // CONTOUR_SEGMENTER_H

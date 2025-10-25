#include "contour_segmenter.h"

std::vector<std::vector<cv::Point2f>> ContourSegmenter::segmentContour(
    const std::vector<cv::Point2f>& contour,
    const std::vector<cv::Point2f>& cornerPoints) {

    std::vector<std::vector<cv::Point2f>> segmentedContours;
    std::vector<cv::Vec4f> lines;
    double threshold = 8;
    int maxIterations = 100;

    ContourSegment cs{contour};
    cs.sequentialRansac3Times(contour, segmentedContours, lines, threshold, maxIterations);
    return segmentedContours;
}

std::map<int, std::vector<cv::Point2f>> ContourSegmenter::sortSegmentsCounterClockwise(
    const std::vector<std::vector<cv::Point2f>>& segments,
    const cv::Point2f& referencePoint) {

    std::map<int, std::vector<cv::Point2f>> sortedContours;
    if (segments.empty()) return sortedContours;

    std::vector<std::pair<cv::Point2f, std::vector<cv::Point2f>>> contoursWithMidPoints;
    for (const auto& contour : segments) {
        if (contour.empty()) continue;
        int midIndex = static_cast<int>(contour.size() / 2);
        contoursWithMidPoints.push_back({contour[midIndex], contour});
    }

    if (contoursWithMidPoints.empty()) return sortedContours;

    // 冒泡排序进行逆时针排序
    for (int i = 0; i < contoursWithMidPoints.size() - 1; i++) {
        for (int j = 0; j < contoursWithMidPoints.size() - i - 1; j++) {
            const cv::Point2f& pointA = contoursWithMidPoints[j].first;
            const cv::Point2f& pointB = contoursWithMidPoints[j + 1].first;
            if (isPointClockwiseTo(pointA, pointB, referencePoint)) {
                std::swap(contoursWithMidPoints[j], contoursWithMidPoints[j + 1]);
            }
        }
    }

    std::vector<std::pair<cv::Point2f, std::vector<cv::Point2f>>> circularList = contoursWithMidPoints;
    auto maxPointContour = std::max_element(circularList.begin(), circularList.end(),
                                            [](const auto& a, const auto& b) { return a.second.size() < b.second.size(); });

    if (maxPointContour == circularList.end()) return sortedContours;

    int maxIndex = std::distance(circularList.begin(), maxPointContour);
    int n = static_cast<int>(circularList.size());
    int firstIndex = (maxIndex - 1 + n) % n;
    int thirdIndex = (maxIndex + 1) % n;

    sortedContours[1] = circularList[firstIndex].second;
    sortedContours[2] = circularList[maxIndex].second;
    sortedContours[3] = circularList[thirdIndex].second;

    return sortedContours;
}

bool ContourSegmenter::isPointClockwiseTo(const cv::Point2f& a, const cv::Point2f& b, const cv::Point2f& reference) {
    cv::Point2f relA = a - reference;
    cv::Point2f relB = b - reference;
    float det = relA.x * relB.y - relA.y * relB.x;

    if (det > 0) return false;
    if (det < 0) return true;

    float d1 = relA.x * relA.x + relA.y * relA.y;
    float d2 = relB.x * relB.x + relB.y * relB.y;
    return d1 < d2;
}

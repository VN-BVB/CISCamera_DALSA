#ifndef CONTOUR_SEGMENT_H
#define CONTOUR_SEGMENT_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// 轮廓分割类
class ContourSegment
{
public:
    explicit ContourSegment(const std::vector<cv::Point> &contour);
    explicit ContourSegment(const std::vector<cv::Point2f> &contour);
    ~ContourSegment();

    void lineRansac(const std::vector<cv::Point> &points,
                    cv::Vec4f &line,
                    std::vector<cv::Point> &inlierPoints,
                    const double &threshold = 5,
                    const int &iterations = 100);
    void lineRansac(const std::vector<cv::Point2f> &points,
                    cv::Vec4f &line,
                    std::vector<cv::Point2f> &inlierPoints,
                    const double &threshold = 5,
                    const int &iterations = 100);
    // 三次顺序RANSAC直线拟合
    void sequentialRansac3Times(const std::vector<cv::Point>& points,
                                std::vector<std::vector<cv::Point>>& segments,
                                std::vector<cv::Vec4f>& lines,
                                double threshold = 0.5,
                                int maxIterations = 100);
    void sequentialRansac3Times(const std::vector<cv::Point2f>& points,
                                std::vector<std::vector<cv::Point2f>>& segments,
                                std::vector<cv::Vec4f>& lines,
                                double threshold = 0.5,
                                int maxIterations = 100);
    // 根据点分割轮廓
    std::vector<std::vector<cv::Point2f>> segmentContourByApproxPoints(const std::vector<cv::Point2f>& contour,
                                                                       const std::vector<cv::Point2f>& approxPoints);
    // 轮廓分割
    void segment();

private:
    std::vector<cv::Point> m_contour;
    std::vector<cv::Point2f> m_subpixelContour;
    std::vector<std::vector<cv::Point2f>> m_approxContours; // 逼近后的多边形
};

#endif // CONTOUR_SEGMENT_H

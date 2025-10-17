#ifndef CONTOURSEGMENT_H
#define CONTOURSEGMENT_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// 利用多边形拟合算法实现轮廓分割
class ContourSegment
{
public:
    explicit ContourSegment(const std::vector<cv::Point> &contour);
    ~ContourSegment();

    // 判断轮廓开口方向
    std::string getOpeningDirection();
    void lineRansac(const std::vector<cv::Point> &points,
                    cv::Vec4f &line,
                    std::vector<cv::Point> &inlierPoints,
                    const double &threshold = 5,
                    const int &iterations = 100);
    // 三次顺序RANSAC直线拟合
    void sequentialRansac3Times(const std::vector<cv::Point>& points,
                                std::vector<std::vector<cv::Point>>& segments,
                                std::vector<cv::Vec4f>& lines,
                                double threshold = 0.5,
                                int maxIterations = 100);
    // 根据点分割轮廓
    std::vector<std::vector<cv::Point2f>> ContourSegment::segmentContourByApproxPoints(const std::vector<cv::Point2f>& contour,
                                                                                       const std::vector<cv::Point2f>& approxPoints);
    // 轮廓分割
    void segment();
private:
    std::vector<cv::Point> m_contour;
    std::vector<std::vector<cv::Point2f>> m_approxContours; // 逼近后的多边形
};

#endif // CONTOURSEGMENT_H

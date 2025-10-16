#ifndef CONTOURSEGMENT_H
#define CONTOURSEGMENT_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// 利用多边形拟合算法实现轮廓分割
class ContourSegment
{
public:
    explicit ContourSegment(const std::vector<cv::Point2f> contour);
    ~ContourSegment();

    // 轮廓分割
    void segment();

private:
    std::vector<cv::Point2f> m_contour;
    std::vector<std::vector<cv::Point2f>> m_approxContours; // 逼近后的多边形
};

#endif // CONTOURSEGMENT_H

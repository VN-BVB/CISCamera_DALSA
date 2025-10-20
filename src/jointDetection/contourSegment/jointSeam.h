#ifndef JOINTSEAM_H
#define JOINTSEAM_H

#include "contourCurve.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

/**
 * @brief   拼缝类
 */
class JointSeam
{
public:
    std::vector<ContourCurve> getContourCurves() const {return m_contourCurves;}
public:
    explicit JointSeam(const cv::Mat &image);
    cv::Point2f zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius);
    std::vector<cv::Point2f> getSubpixelContourZernike(const cv::Mat &src, const std::vector<cv::Point> &contour);
    double adaptiveCannyThresholdByOtsu(const cv::Mat &srcImage);

    void run();
private:
    cv::Mat m_image;
    std::vector<ContourCurve> m_contourCurves;
};

#endif // JOINTSEAM_H

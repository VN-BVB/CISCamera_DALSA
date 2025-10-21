#ifndef JOINT_SEAM_H
#define JOINT_SEAM_H

#include "contour_curve.h"
#include "src/jointDetection/edgeDetection/edge_detector.h"
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

    void run();
private:
    cv::Mat m_image;
    std::vector<ContourCurve> m_contourCurves;
};

#endif // JOINT_SEAM_H

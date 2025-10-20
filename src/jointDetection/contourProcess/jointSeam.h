#ifndef JOINTSEAM_H
#define JOINTSEAM_H

#include "contourCurve.h"
#include "src/jointDetection/edgeDetection/edgeDetector.h"
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

#endif // JOINTSEAM_H

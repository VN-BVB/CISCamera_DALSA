#ifndef JOINT_SEAM_H
#define JOINT_SEAM_H

#include "src/jointDetection/edgeDetection/edge_detector.h"
#include "contour_processor.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

/**
 * @brief   拼缝类
 */
class JointSeam
{
public:
    // std::vector<ContourCurve> getContourCurves() const {return m_contourCurves;}
public:
    explicit JointSeam(const cv::Mat &image);

    void run();
private:
    cv::Mat m_image;                                // 拼缝roi处图像
    // std::vector<ContourCurve> m_contourCurves;      // 拼缝两边轮廓
    cv::Point2f m_position;                         // 拼缝roi位置
    // std::vector<cv::Vec4f> m_lines;                 // 拼缝两侧直线
    std::vector<cv::Point2f> m_endPoints;           // 拼缝四个端点

public:
    std::vector<ContourProcessor> getContourProcessor() const {return m_contourProcessor;}
private:
    std::vector<ContourProcessor> m_contourProcessor;

};


#endif // JOINT_SEAM_H

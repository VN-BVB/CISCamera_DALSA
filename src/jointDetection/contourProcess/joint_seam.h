#ifndef JOINT_SEAM_H
#define JOINT_SEAM_H

#include "src/jointDetection/edgeDetection/edge_detector.h"
#include "src/utils/geometry_utils.h"
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

    // 计算中间缝隙中心线（中轴变换 + RANSAC）
    cv::Vec4f calculateCenterLineBySkeletonAndRANSAC(const cv::Mat& image);
    // 根据方向向量格式 (vx, vy, x0, y0) 绘制直线
    void drawLineFromDirectionVector(cv::Mat& image, const cv::Vec4f& directionVector);

public:
    std::vector<ContourProcessor> getContourProcessor() const {return m_contourProcessor;}
private:
    std::vector<ContourProcessor> m_contourProcessor;

};


#endif // JOINT_SEAM_H

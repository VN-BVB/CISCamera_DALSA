#ifndef JOINT_SEAM_H
#define JOINT_SEAM_H


#include "src/utils/geometry_utils.h"
#include "src/jointDetection/edgeDetection/abstract_contour_detector.h"
#include "src/jointDetection/edgeDetection/contour_detector_context.h"
#include "contourProcess/methods/contour_data.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

/**
 * @brief   拼缝类
 */
class JointSeam
{
public:
    explicit JointSeam(const cv::Mat &image, const cv::Point2f position);

    std::vector<ContourData> getContourDatas() const {return m_contourDatas;}
    std::vector<cv::Vec4f> getLines() const {return m_lines;}
    std::vector<cv::Point2f> getEndPoints() const { return m_endPoints;}

    void run();
private:
    cv::Mat m_image;                                // 拼缝roi处图像
    std::vector<ContourData> m_contourDatas;        // 拼缝两边轮廓
    cv::Point2f m_position;                         // 拼缝roi图像左上角坐标
    std::vector<cv::Vec4f> m_lines;                 // 拼缝两侧所有直线
    std::vector<cv::Point2f> m_endPoints;           // 拼缝四个端点
};


#endif // JOINT_SEAM_H

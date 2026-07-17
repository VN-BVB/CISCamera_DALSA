#ifndef JOINT_SEAM_H
#define JOINT_SEAM_H


#include "src/utils/geometry_utils.h"
#include "src/jointDetection/edgeDetection/abstract_contour_detector.h"
#include "src/jointDetection/edgeDetection/contour_detector_context.h"
#include "contourProcess/methods/contour_data.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

struct SeamEndpoint {
    int id;                                 // 端点ID
    cv::Point2f coordinates;                // 端点坐标
    int contourId;                          // 所属轮廓ID
    int correspondingIntersectionId;        // 对应的交点ID

    // 默认构造函数
    SeamEndpoint() : id(-1), contourId(-1), correspondingIntersectionId(-1) {}

    // 完整构造函数
    SeamEndpoint(int _id, const cv::Point2f& _coords, int _contourId, int _correspondingIntersectionId)
        : id(_id), coordinates(_coords), contourId(_contourId), correspondingIntersectionId(_correspondingIntersectionId) {}
};

/**
 * @brief   拼缝类
 */
class JointSeam
{
public:
    explicit JointSeam(const cv::Mat &image, const cv::Point2f position, const int id);

    std::vector<ContourData> getContourDatas() const {return m_contourDatas;}
    std::vector<cv::Vec4f> getLines() const {return m_lines;}
    std::vector<SeamEndpoint> getEndPoints() const { return m_endPoints;}

    void run();

private:
    // 计算端点之间的对应关系
    void calculateEndpointCorrespondences();
private:
    int m_id;                                       // 拼缝id
    cv::Mat m_image;                                // 拼缝roi处图像
    std::vector<ContourData> m_contourDatas;        // 拼缝两边轮廓
    cv::Point2f m_position;                         // 拼缝roi图像左上角坐标
    std::vector<cv::Vec4f> m_lines;                 // 拼缝两侧所有直线
    std::vector<SeamEndpoint> m_endPoints;          // 拼缝四个端点
};

#endif // JOINT_SEAM_H

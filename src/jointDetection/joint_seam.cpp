#include "joint_seam.h"
#include "src/utils/image_tools.h"
#include "src/jointDetection/edgeDetection/canny_zernike_detector.h"
#include "contourProcess/contour_processor.h"
#include <iostream>

JointSeam::JointSeam(const cv::Mat &image, const cv::Point2f position)
    : m_image(image),
    m_position(position)
{}

void JointSeam::run() {
    // 拼缝两侧亚像素轮廓检测
    std::unique_ptr<AbstractContourDetector> s1;
    std::unique_ptr<ContourDetectorContext> c = std::make_unique<ContourDetectorContext>();
    s1 = std::make_unique<CannyZernikeDetector>();
    std::vector<std::vector<cv::Point2f>> contours;
    c->setDetector(std::move(s1));
    contours = c->detectContours(m_image);

    // 添加m_position偏移量,映射到整体图像坐标
    for (auto& contour : contours) {
        for (auto& point : contour) {
            point.x += m_position.x;
            point.y += m_position.y;
        }
    }

    // 轮廓信息整处理
    for (auto& contour : contours) {
        ContourProcessor processor;
        if(processor.processContour(contour))
        {
            // 获取处理结果
            auto result = processor.getResult();
            m_contourDatas.push_back(result);
            std::vector<cv::Vec4f> tangentLines = processor.getTangentLines();
            std::vector<cv::Point2f> endPoints = processor.getEndPoints();
            m_lines.insert(m_lines.end(), tangentLines.begin(), tangentLines.end());
            m_endPoints.insert(m_endPoints.end(), endPoints.begin(), endPoints.end());
        }
    }
}



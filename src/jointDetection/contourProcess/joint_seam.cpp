#include "joint_seam.h"
#include "src/jointDetection/image_tools.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <set>


JointSeam::JointSeam(const cv::Mat &image) : m_image(image)
{}

void JointSeam::run() {
    EdgeDetector ed(m_image);
    std::vector<std::vector<cv::Point>> contours;
    contours = ed.run();


    // @TODO:将以上步骤放在edgeDetector中去，返回两条轮廓的亚像素点集，后续直接将这两条轮廓送到ContourProcessor中

    // 轮廓信息整理
    for (auto& contour : contours) {
        ContourProcessor cProcessor;
        std::vector<cv::Point2f> subpixelContour = ed.getSubpixelContourZernike(m_image, contour);
        cProcessor.processContour(subpixelContour);
        m_contourProcessor.push_back(cProcessor);
    }
}

























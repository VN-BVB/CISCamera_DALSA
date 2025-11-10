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
    std::unique_ptr<AbstractContourDetector> s1;
    std::unique_ptr<ContourDetectorContext> c = std::make_unique<ContourDetectorContext>();
    s1 = std::make_unique<CannyZernikeDetector>();
    std::vector<std::vector<cv::Point2f>> contours;
    c->setDetector(std::move(s1));
    contours = c->detectContours(m_image);
    // // 拼缝两侧亚像素轮廓检测
    // EdgeDetector ed(m_image);
    // std::vector<std::vector<cv::Point2f>> contours;
    // contours = ed.run();

    // 轮廓信息整理
    for (auto& contour : contours) {
        ContourProcessor cProcessor;
        cProcessor.processContour(contour);
        m_contourProcessor.push_back(cProcessor);
    }
}

























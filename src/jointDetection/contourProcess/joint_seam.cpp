#include "joint_seam.h"
#include "src/jointDetection/image_tools.h"
#include "src/jointDetection/edgeDetection/canny_zernike_detector.h"

JointSeam::JointSeam(const cv::Mat &image) : m_image(image)
{}

void JointSeam::run() {
    // 拼缝两侧亚像素轮廓检测
    std::unique_ptr<AbstractContourDetector> s1;
    std::unique_ptr<ContourDetectorContext> c = std::make_unique<ContourDetectorContext>();
    s1 = std::make_unique<CannyZernikeDetector>();
    std::vector<std::vector<cv::Point2f>> contours;
    c->setDetector(std::move(s1));
    contours = c->detectContours(m_image);

    // 轮廓信息整处理
    for (auto& contour : contours) {
        ContourProcessor cProcessor;
        cProcessor.processContour(contour);
        m_contourProcessor.push_back(cProcessor);
    }
}




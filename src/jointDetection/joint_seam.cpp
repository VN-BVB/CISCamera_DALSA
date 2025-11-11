#include "joint_seam.h"
#include "src/utils/image_tools.h"
#include "src/jointDetection/edgeDetection/canny_zernike_detector.h"
#include "contourProcess/contour_processor.h"
#include <iostream>

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
        ContourProcessorV2 processor;
        if(processor.processContour(contour))
        {
            // 获取处理结果
            auto result = processor.getResult();
            m_contourDatas.push_back(result);
            std::cout << "dsafdsf" << std::endl;
        }
    }
}




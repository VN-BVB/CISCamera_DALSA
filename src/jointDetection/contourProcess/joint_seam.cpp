#include "joint_seam.h"
#include "src/jointDetection/image_tools.h"

JointSeam::JointSeam(const cv::Mat &image) : m_image(image){

}

void JointSeam::run() {
    ImageTools imageTools;
    EdgeDetector ed;
    cv::Mat grayImage;
    if (m_image.channels() > 1) {
        cv::cvtColor(m_image, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = m_image.clone();
    }
    cv::GaussianBlur(grayImage, grayImage, cv::Size(7, 7), 0, 0);
    // 双阈值处理--根据Otsu算出的阈值确定为高阈值，取高阈值的一半记为低阈值
    double TH = ed.adaptiveCannyThresholdByOtsu(grayImage);
    double TL = TH * 0.5;

    cv::Mat edge;
    cv::Canny(grayImage, edge, TL, TH);

    // 提取轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE); // 轮廓近似方法设为保存所有点，也可以选择只保存端点，具体见源码注释
    std::vector<std::vector<cv::Point>> filteredContours;
    filteredContours = imageTools.filterContours(contours); // 筛选出来拼缝两侧的轮廓

    // 轮廓信息整理
    for (auto& contour : filteredContours) {
        ContourCurve contourCurve;
        contourCurve.initializePixelContour(contour);
        std::vector<cv::Point2f> subpixelContour = ed.getSubpixelContourZernike(grayImage, contour);
        contourCurve.initializeSubpixelContour(subpixelContour);
        m_contourCurves.push_back(contourCurve);
    }
}





























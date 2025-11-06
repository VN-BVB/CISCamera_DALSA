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

    // 对背光图去除工件外杂乱边缘，对正光图去除工件内杂乱边缘
    cv::Mat binaryImage;
    cv::threshold(grayImage, binaryImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // // 先进行闭运算去除二值图中白色区域的空洞，可处理工件外有少量杂物的情况
    // cv::Mat closedBinary;
    // cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    // cv::morphologyEx(binaryImage, closedBinary, cv::MORPH_CLOSE, closeKernel);
    // cv::imwrite("E:/work/车门门环拼接/image/test/frontLight/closedBinary.bmp", closedBinary);

    // 对二值图进行腐蚀，减小边缘无关区域面积，对背光和正光都有用
    cv::Mat erodedBinary;
    cv::Mat erodeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    // 进行三次腐蚀，确保缩小边缘无关区域
    cv::erode(binaryImage, erodedBinary, erodeKernel);
    cv::erode(erodedBinary, erodedBinary, erodeKernel);
    cv::erode(erodedBinary, erodedBinary, erodeKernel);
    cv::bitwise_not(erodedBinary, erodedBinary);
    // 将腐蚀后的二值图翻转，与edge相乘，保留边缘区域，去除无关区域
    cv::Mat connectedEdge;
    cv::multiply(edge, erodedBinary / 255.0, connectedEdge, 1, CV_8U);

    // 提取轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(connectedEdge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE); // 轮廓近似方法设为保存所有点，也可以选择只保存端点，具体见源码注释
    std::vector<std::vector<cv::Point>> filteredContours;
    filteredContours = imageTools.filterContours(contours); // 筛选出来拼缝两侧的轮廓
    imageTools.drawColorfulContoursAndSave(grayImage, filteredContours,
                                           "E:/work/车门门环拼接/image/test/frontLight/front_light_filted_edge.bmp");

    // 轮廓信息整理
    for (auto& contour : filteredContours) {
        ContourProcessor cProcessor;
        std::vector<cv::Point2f> subpixelContour = ed.getSubpixelContourZernike(grayImage, contour);
        cProcessor.processContour(subpixelContour);
        m_contourProcessor.push_back(cProcessor);
    }
}





























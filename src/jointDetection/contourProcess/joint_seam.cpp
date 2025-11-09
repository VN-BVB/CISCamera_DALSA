#include "joint_seam.h"
#include "src/jointDetection/image_tools.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <set>


JointSeam::JointSeam(const cv::Mat &image) : m_image(image){

}

// 计算中间缝隙中心线（中轴变换 + RANSAC）
cv::Vec4f JointSeam::calculateCenterLineBySkeletonAndRANSAC(const cv::Mat& image) {
    cv::Mat grayImage;
    if (image.channels() > 1) {
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = image.clone();
    }

    // 1. 图像二值化
    cv::Mat binary;
    cv::threshold(grayImage, binary, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    // 反转二值图（将黑色变为白色，白色变为黑色）(正光和背光不一样)
    // cv::bitwise_not(binary, binary);

    // 2. 中轴变换（Skeletonization）
    cv::Mat skel = cv::Mat::zeros(binary.size(), CV_8UC1);
    cv::Mat temp = cv::Mat::zeros(binary.size(), CV_8UC1);
    cv::Mat eroded = cv::Mat::zeros(binary.size(), CV_8UC1);

    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
    bool done = false;

    while (!done) {
        cv::erode(binary, eroded, element);
        cv::dilate(eroded, temp, element);
        cv::subtract(binary, temp, temp);
        cv::bitwise_or(skel, temp, skel);
        eroded.copyTo(binary);

        if (cv::countNonZero(binary) == 0) {
            done = true;
        }
    }

    // 3. 提取中心线坐标点
    std::vector<cv::Point2f> centerLinePoints;
    for (int y = 0; y < skel.rows; y++) {
        for (int x = 0; x < skel.cols; x++) {
            if (skel.at<uchar>(y, x) > 0) {
                centerLinePoints.push_back(cv::Point2f(x, y));
            }
        }
    }

    // 4.RANSAC计算中心线直线方程
    cv::Vec4f centerLine;
    std::vector<cv::Point2f> inlierPoints;
    double threshold = 5;
    int iterations = 100;
    GeometryUtils::lineRansac(centerLinePoints, centerLine, inlierPoints, threshold, iterations);

    return centerLine;
}

// 根据中心线将轮廓分类到两侧
std::pair<std::vector<std::vector<cv::Point>>, std::vector<std::vector<cv::Point>>>
JointSeam::classifyContoursByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine) {
    std::vector<std::vector<cv::Point>> leftContours;  // 中心线左侧的轮廓
    std::vector<std::vector<cv::Point>> rightContours; // 中心线右侧的轮廓

    // 提取直线参数：方向向量(vx, vy)和直线上点(x0, y0)
    float vx = centerLine[0];
    float vy = centerLine[1];
    float x0 = centerLine[2];
    float y0 = centerLine[3];

    // 计算直线的法向量（用于判断点在直线的哪一侧）
    // 法向量为(-vy, vx)或(vy, -vx)，这里使用(-vy, vx)
    float nx = -vy;
    float ny = vx;

    // 归一化法向量
    float length = std::sqrt(nx * nx + ny * ny);
    if (length > 1e-6) {
        nx /= length;
        ny /= length;
    }

    // 遍历所有轮廓
    for (const auto& contour : contours) {
        if (contour.empty()) {
            continue;
        }

        // 计算轮廓的重心（质心）
        cv::Moments moments = cv::moments(contour);
        if (moments.m00 == 0) {
            continue;
        }

        float centroidX = moments.m10 / moments.m00;
        float centroidY = moments.m01 / moments.m00;

        // 计算重心到直线上点(x0, y0)的向量
        float dx = centroidX - x0;
        float dy = centroidY - y0;

        // 计算向量与法向量的点积
        float dotProduct = dx * nx + dy * ny;

        // 根据点积的正负判断轮廓在直线的哪一侧
        // 点积 > 0：在法向量方向（右侧）
        // 点积 < 0：在法向量反方向（左侧）
        if (dotProduct > 0) {
            rightContours.push_back(contour);
        } else {
            leftContours.push_back(contour);
        }
    }

    return std::make_pair(leftContours, rightContours);
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
    cv::imwrite("E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/edge.bmp", edge);

    // 对背光图去除工件外杂乱边缘，对正光图去除工件内杂乱边缘
    cv::Mat binaryImage;
    cv::threshold(grayImage, binaryImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::imwrite("E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/binaryImage.bmp", binaryImage);

    // 先进行闭运算去除二值图中白色区域的空洞，可处理工件外有少量杂物的情况
    cv::Mat closedBinary;
    cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
    cv::morphologyEx(binaryImage, closedBinary, cv::MORPH_CLOSE, closeKernel);
    cv::imwrite("E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/closedBinary.bmp", closedBinary);

    // 对二值图进行腐蚀，减小边缘无关区域面积，对背光和正光都有用
    cv::Mat erodedBinary;
    cv::Mat erodeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    // 进行三次腐蚀，确保缩小边缘无关区域
    cv::erode(closedBinary, erodedBinary, erodeKernel);
    cv::erode(erodedBinary, erodedBinary, erodeKernel);
    cv::erode(erodedBinary, erodedBinary, erodeKernel);
    cv::bitwise_not(erodedBinary, erodedBinary);
    cv::imwrite("E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/erodedBinary.bmp", erodedBinary);
    // 将腐蚀后的二值图翻转，与edge相乘，保留边缘区域，去除无关区域
    cv::Mat connectedEdge;
    cv::multiply(edge, erodedBinary / 255.0, connectedEdge, 1, CV_8U);
    cv::imwrite("E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/connectedEdge.bmp", connectedEdge);

    // 计算中间缝隙中心线
    cv::Vec4f centerLine = calculateCenterLineBySkeletonAndRANSAC(m_image);
    imageTools.drawLineAndSave(m_image, centerLine, "E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/result_with_center_line.bmp");


    // 提取轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(connectedEdge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<std::vector<cv::Point>> filteredContours;
    filteredContours = imageTools.filterContours(contours); // 筛选出来拼缝两侧的轮廓
    // 使用中心线将轮廓分类到两侧
    auto [leftContours, rightContours] = classifyContoursByCenterLine(filteredContours, centerLine);
    imageTools.drawColorfulContoursAndSave(grayImage, filteredContours,
                                           "E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/front_light_filted_edge_connected.bmp");
    // 可选：保存分类后的轮廓用于调试
    imageTools.drawColorfulContoursAndSave(grayImage, leftContours,
                                           "E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/left_contours.bmp");
    imageTools.drawColorfulContoursAndSave(grayImage, rightContours,
                                           "E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/right_contours.bmp");



    // 轮廓信息整理
    for (auto& contour : filteredContours) {
        ContourProcessor cProcessor;
        std::vector<cv::Point2f> subpixelContour = ed.getSubpixelContourZernike(grayImage, contour);
        cProcessor.processContour(subpixelContour);
        m_contourProcessor.push_back(cProcessor);
    }
}

























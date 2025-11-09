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
    cv::bitwise_not(binary, binary);

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


    cv::Vec4f centerLine;
    std::vector<cv::Point2f> inlierPoints;
    double threshold = 5;
    int iterations = 100;
    GeometryUtils::lineRansac(centerLinePoints, centerLine, inlierPoints, threshold, iterations);

    return centerLine;
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


    // 提取轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(connectedEdge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<std::vector<cv::Point>> filteredContours;
    filteredContours = imageTools.filterContours(contours); // 筛选出来拼缝两侧的轮廓
    imageTools.drawColorfulContoursAndSave(grayImage, filteredContours,
                                           "E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/front_light_filted_edge_connected.bmp");

    // 计算中间缝隙中心线并绘制在原图上
    cv::Vec4f centerLine = calculateCenterLineBySkeletonAndRANSAC(m_image);
    // 检查是否找到有效的中心线
    if (centerLine[0] != 0 || centerLine[1] != 0 || centerLine[2] != 0 || centerLine[3] != 0) {
        // 创建原图的彩色副本用于绘制
        cv::Mat resultImage;
        if (m_image.channels() == 1) {
            cv::cvtColor(m_image, resultImage, cv::COLOR_GRAY2BGR);
        } else {
            resultImage = m_image.clone();
        }

        // 直接使用方向向量格式 (vx, vy, x0, y0) 绘制直线
        drawLineFromDirectionVector(resultImage, centerLine);

        // 保存带中心线的结果图像
        cv::imwrite("E:/work/车门门环拼接/image/test/frontLight/1107/正normal5/result_with_center_line.bmp", resultImage);
    } else {
        std::cout << "未找到有效的中心线" << std::endl;
    }

    // 轮廓信息整理
    for (auto& contour : filteredContours) {
        ContourProcessor cProcessor;
        std::vector<cv::Point2f> subpixelContour = ed.getSubpixelContourZernike(grayImage, contour);
        cProcessor.processContour(subpixelContour);
        m_contourProcessor.push_back(cProcessor);
    }
}

// 新增方法：根据方向向量格式 (vx, vy, x0, y0) 绘制直线
void JointSeam::drawLineFromDirectionVector(cv::Mat& image, const cv::Vec4f& directionVector) {
    // 提取直线参数
    float vx = directionVector[0]; // 方向向量x分量
    float vy = directionVector[1]; // 方向向量y分量
    float x0 = directionVector[2]; // 直线上的点x坐标
    float y0 = directionVector[3]; // 直线上的点y坐标

    // 检查方向向量是否有效
    float length = std::sqrt(vx * vx + vy * vy);
    if (length < 1e-6) {
        std::cout << "无效的方向向量" << std::endl;
        return;
    }

    // 归一化方向向量
    float nx = vx / length;
    float ny = vy / length;

    // 计算直线与图像边界的交点
    std::vector<cv::Point2f> intersections;

    // 使用参数方程：x = x0 + t * nx, y = y0 + t * ny
    // 计算与图像边界的交点

    // 与左边界 (x=0) 的交点
    if (std::abs(nx) > 1e-6) {
        float t_left = (0 - x0) / nx;
        float y_left = y0 + t_left * ny;
        if (y_left >= 0 && y_left < image.rows) {
            intersections.push_back(cv::Point2f(0, y_left));
        }
    }

    // 与右边界 (x=image.cols-1) 的交点
    if (std::abs(nx) > 1e-6) {
        float t_right = (image.cols - 1 - x0) / nx;
        float y_right = y0 + t_right * ny;
        if (y_right >= 0 && y_right < image.rows) {
            intersections.push_back(cv::Point2f(image.cols - 1, y_right));
        }
    }

    // 与上边界 (y=0) 的交点
    if (std::abs(ny) > 1e-6) {
        float t_top = (0 - y0) / ny;
        float x_top = x0 + t_top * nx;
        if (x_top >= 0 && x_top < image.cols) {
            intersections.push_back(cv::Point2f(x_top, 0));
        }
    }

    // 与下边界 (y=image.rows-1) 的交点
    if (std::abs(ny) > 1e-6) {
        float t_bottom = (image.rows - 1 - y0) / ny;
        float x_bottom = x0 + t_bottom * nx;
        if (x_bottom >= 0 && x_bottom < image.cols) {
            intersections.push_back(cv::Point2f(x_bottom, image.rows - 1));
        }
    }

    // 去重并确保有两个不同的交点
    if (intersections.size() >= 2) {
        // 去除重复点
        std::vector<cv::Point2f> unique_intersections;
        for (const auto& point : intersections) {
            bool is_duplicate = false;
            for (const auto& existing : unique_intersections) {
                if (cv::norm(point - existing) < 1.0) {
                    is_duplicate = true;
                    break;
                }
            }
            if (!is_duplicate) {
                unique_intersections.push_back(point);
            }
        }

        if (unique_intersections.size() >= 2) {
            // 绘制直线（红色，线宽3像素）
            cv::line(image, unique_intersections[0], unique_intersections[1], cv::Scalar(0, 0, 255), 3);

            // 绘制端点（绿色圆圈）
            cv::circle(image, unique_intersections[0], 5, cv::Scalar(0, 255, 0), -1);
            cv::circle(image, unique_intersections[1], 5, cv::Scalar(0, 255, 0), -1);

            // 绘制直线上的参考点（蓝色圆圈）
            cv::Point2f referencePoint(x0, y0);
            cv::circle(image, referencePoint, 3, cv::Scalar(255, 0, 0), -1);

            return;
        }
    }

    // 如果无法找到两个边界交点，使用默认方法：在直线上取两个距离较远的点
    float half_diag = std::sqrt(image.cols * image.cols + image.rows * image.rows) / 2.0f;

    cv::Point2f p1(x0 - half_diag * nx, y0 - half_diag * ny);
    cv::Point2f p2(x0 + half_diag * nx, y0 + half_diag * ny);

    // 确保点在图像范围内
    p1.x = std::max(0.0f, std::min(static_cast<float>(image.cols - 1), p1.x));
    p1.y = std::max(0.0f, std::min(static_cast<float>(image.rows - 1), p1.y));
    p2.x = std::max(0.0f, std::min(static_cast<float>(image.cols - 1), p2.x));
    p2.y = std::max(0.0f, std::min(static_cast<float>(image.rows - 1), p2.y));

    // 绘制直线（红色，线宽3像素）
    cv::line(image, p1, p2, cv::Scalar(0, 0, 255), 3);

    // 绘制端点（绿色圆圈）
    cv::circle(image, p1, 5, cv::Scalar(0, 255, 0), -1);
    cv::circle(image, p2, 5, cv::Scalar(0, 255, 0), -1);

    // 绘制直线上的参考点（蓝色圆圈）
    cv::Point2f referencePoint(x0, y0);
    cv::circle(image, referencePoint, 3, cv::Scalar(255, 0, 0), -1);
}



























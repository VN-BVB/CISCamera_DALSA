#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <set>

#include <plog/Log.h>

#include "canny_zernike_detector.h"
#include "src/utils/image_tools.h"
#include "src/utils/geometry_utils.h"


CannyZernikeDetector::CannyZernikeDetector() {}

// // 改进Zernike亚像素偏移计算辅助函数
// cv::Point2f CannyZernikeDetector::zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius)
// {
//     // 提取边缘点邻域
//     cv::Rect roi(cv::Point(std::max(0, int(edgePoint.x - radius)), std::max(0, int(edgePoint.y - radius))),
//                  cv::Size(2 * radius + 1, 2 * radius + 1));
//     roi &= cv::Rect(0, 0, gray.cols, gray.rows);
//     cv::Mat roiImg = gray(roi);

//     // 定义Zernike矩模板（7x7）
//     cv::Mat M00 = (cv::Mat_<double>(7, 7) << 0, 0.0287, 0.0686, 0.0807, 0.0686, 0.0287, 0,
//                    0.0287, 0.0815, 0.0816, 0.0816, 0.0816, 0.0815, 0.0287,
//                    0.0686, 0.0816, 0.0816, 0.0816, 0.0816, 0.0816, 0.0686,
//                    0.0807, 0.0816, 0.0816, 0.0816, 0.0816, 0.0816, 0.0807,
//                    0.0686, 0.0816, 0.0816, 0.0816, 0.0816, 0.0816, 0.0686,
//                    0.0287, 0.0815, 0.0816, 0.0816, 0.0816, 0.0815, 0.0287,
//                    0, 0.0287, 0.0686, 0.0807, 0.0686, 0.0287, 0);

//     cv::Mat M11R = (cv::Mat_<double>(7, 7) << 0, -0.015, -0.019, 0, 0.019, 0.015, 0,
//                     -0.0224, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0224,
//                     -0.0573, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0573,
//                     -0.069, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.069,
//                     -0.0573, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0573,
//                     -0.0224, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0224,
//                     0, -0.015, -0.019, 0, 0.019, 0.015, 0);

//     cv::Mat M11I = (cv::Mat_<double>(7, 7) << 0, -0.0224, -0.0573, -0.069, -0.0573, -0.0224, 0,
//                     -0.015, -0.0466, -0.0466, -0.0466, -0.0466, -0.0466, -0.015,
//                     -0.019, -0.0233, -0.0233, -0.0233, -0.0233, -0.0233, -0.019,
//                     0, 0, 0, 0, 0, 0, 0,
//                     0.019, 0.0233, 0.0233, 0.0233, 0.0233, 0.0233, 0.019,
//                     0.015, 0.0466, 0.0466, 0.0466, 0.0466, 0.0466, 0.015,
//                     0, 0.0224, 0.0573, 0.069, 0.0573, 0.0224, 0);

//     cv::Mat M20 = (cv::Mat_<double>(7, 7) << 0, 0.0225, 0.0394, 0.0396, 0.0394, 0.0225, 0,
//                    0.0225, 0.0271, -0.0128, -0.0261, -0.0128, 0.0271, 0.0225,
//                    0.0394, -0.0128, -0.0528, -0.0661, -0.0528, -0.0128, 0.0394,
//                    0.0396, -0.0261, -0.0661, -0.0794, -0.0661, -0.0261, 0.0396,
//                    0.0394, -0.0128, -0.0528, -0.0661, -0.0528, -0.0128, 0.0394,
//                    0.0225, 0.0271, -0.0128, -0.0261, -0.0128, 0.0271, 0.0225,
//                    0, 0.0225, 0.0394, 0.0396, 0.0394, 0.0225, 0);

//     cv::Mat M31R = (cv::Mat_<double>(7, 7) << 0, -0.0103, -0.0073, 0, 0.0073, 0.0103, 0,
//                     -0.0153, -0.0018, 0.0162, 0, -0.0162, 0.0018, 0.0153,
//                     -0.0223, 0.0324, 0.0333, 0, -0.0333, -0.0324, 0.0223,
//                     -0.0190, 0.0438, 0.0390, 0, -0.0390, -0.0438, 0.0190,
//                     -0.0223, 0.0324, 0.0333, 0, -0.0333, -0.0324, 0.0223,
//                     -0.0153, -0.0018, 0.0162, 0, -0.0162, 0.0018, 0.0153,
//                     0, -0.0103, -0.0073, 0, 0.0073, 0.0103, 0);

//     cv::Mat M31I = (cv::Mat_<double>(7, 7) << 0, -0.0153, -0.0223, -0.019, -0.0223, -0.0153, 0,
//                     -0.0103, -0.0018, 0.0324, 0.0438, 0.0324, -0.0018, -0.0103,
//                     -0.0073, 0.0162, 0.0333, 0.039, 0.0333, 0.0162, -0.0073,
//                     0, 0, 0, 0, 0, 0, 0,
//                     0.0073, -0.0162, -0.0333, -0.039, -0.0333, -0.0162, 0.0073,
//                     0.0103, 0.0018, -0.0324, -0.0438, -0.0324, 0.0018, 0.0103,
//                     0, 0.0153, 0.0223, 0.0190, 0.0223, 0.0153, 0);

//     cv::Mat M40 = (cv::Mat_<double>(7, 7) << 0, 0.013, 0.0056, -0.0018, 0.0056, 0.013, 0,
//                    0.0130, -0.0186, -0.0323, -0.0239, -0.0323, -0.0186, 0.0130,
//                    0.0056, -0.0323, 0.0125, 0.0406, 0.0125, -0.0323, 0.0056,
//                    -0.0018, -0.0239, 0.0406, 0.0751, 0.0406, -0.0239, -0.0018,
//                    0.0056, -0.0323, 0.0125, 0.0406, 0.0125, -0.0323, 0.0056,
//                    0.0130, -0.0186, -0.0323, -0.0239, -0.0323, -0.0186, 0.0130,
//                    0, 0.013, 0.0056, -0.0018, 0.0056, 0.013, 0);

//     // 计算Zernike矩
//     cv::Mat roiImgFloat;
//     roiImg.convertTo(roiImgFloat, CV_64F);

//     cv::Mat ZerImgM00, ZerImgM11R, ZerImgM11I, ZerImgM20, ZerImgM31R, ZerImgM31I, ZerImgM40;
//     cv::filter2D(roiImgFloat, ZerImgM00, CV_64F, M00);
//     cv::filter2D(roiImgFloat, ZerImgM11R, CV_64F, M11R);
//     cv::filter2D(roiImgFloat, ZerImgM11I, CV_64F, M11I);
//     cv::filter2D(roiImgFloat, ZerImgM20, CV_64F, M20);
//     cv::filter2D(roiImgFloat, ZerImgM31R, CV_64F, M31R);
//     cv::filter2D(roiImgFloat, ZerImgM31I, CV_64F, M31I);
//     cv::filter2D(roiImgFloat, ZerImgM40, CV_64F, M40);

//     // 获取中心点值
//     int center_x = radius;
//     int center_y = radius;

//     double z00 = ZerImgM00.at<double>(center_y, center_x);
//     double z11r = ZerImgM11R.at<double>(center_y, center_x);
//     double z11i = ZerImgM11I.at<double>(center_y, center_x);
//     double z20 = ZerImgM20.at<double>(center_y, center_x);
//     double z31r = ZerImgM31R.at<double>(center_y, center_x);
//     double z31i = ZerImgM31I.at<double>(center_y, center_x);
//     double z40 = ZerImgM40.at<double>(center_y, center_x);

//     // 计算角度和长度参数
//     double theta = std::atan2(z31i, z31r);
//     double rotated_z11 = std::sin(theta) * z11i + std::cos(theta) * z11r;
//     double rotated_z31 = std::sin(theta) * z31i + std::cos(theta) * z31r;

//     double l_method1 = std::sqrt((5 * z40 + 3 * z20) / (8 * z20));
//     double l_method2 = std::sqrt((5 * rotated_z31 + rotated_z11) / (6 * rotated_z11));

//     double l = (l_method1 + l_method2) / 2;
//     double k = 3 * rotated_z11 / (2 * std::pow(1 - l_method2 * l_method2, 1.5));

//     // 阈值参数
//     double k_value = 20.0;
//     double l_value = std::sqrt(2.0) / 7.0; // sqrt(2)/g_N, g_N=7
//     double absl = std::abs(l_method2 - l_method1);

//     // @TODO:排查Zernike矩亚像素不计算偏移的问题
//     k_value = 0.0;
//     absl = -100000;
//     // 根据条件计算亚像素偏移
//     if (k >= k_value && absl <= l_value)
//     {
//         float dx = 7.0 * l * std::cos(theta) / 2.0; // g_N=7
//         float dy = 7.0 * l * std::sin(theta) / 2.0;
//         return cv::Point2f(edgePoint.x + dx, edgePoint.y + dy);
//     }

//     return edgePoint; // 不满足条件时返回原坐标
// }

/**
 * @brief 使用Zernike矩法计算亚像素级边缘点偏移
 * @param gray 输入灰度图像
 * @param edgePoint 原始边缘点坐标
 * @param radius Zernike矩计算半径
 * @return 亚像素级精度的边缘点坐标
 */
cv::Point2f CannyZernikeDetector::zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius) {
    // 检查边缘点是否在图像范围内
    if (edgePoint.x < 0 || edgePoint.x >= gray.cols || edgePoint.y < 0 || edgePoint.y >= gray.rows) {
        return edgePoint;
    }

    // 提取边缘点邻域
    cv::Rect roi(cv::Point(std::max(0, int(edgePoint.x - radius)), std::max(0, int(edgePoint.y - radius))),
                 cv::Size(2 * radius + 1, 2 * radius + 1));
    roi &= cv::Rect(0, 0, gray.cols, gray.rows);
    cv::Mat roiImg = gray(roi);

    // 三个矩模板
    cv::Mat M11R = (cv::Mat_<double>(7, 7) << 0, -0.0150, -0.0190, 0, 0.0190, 0.0150, 0, -0.0224, -0.0466, -0.0233, 0, 0.0233,
                    0.0466, 0.0224, -0.0573, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0573, -0.0690, -0.0466, -0.0233, 0, 0.0233,
                    0.0466, 0.0690, -0.0573, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0573, -0.0224, -0.0466, -0.0233, 0, 0.0233,
                    0.0466, 0.0224, 0, -0.0150, -0.0190, 0, 0.0190, 0.0150, 0);

    cv::Mat M11I = (cv::Mat_<double>(7, 7) << 0, -0.0224, -0.0573, -0.0690, -0.0573, -0.0224, 0, -0.0150, -0.0466, -0.0466,
                    -0.0466, -0.0466, -0.0466, -0.0150, -0.0190, -0.0233, -0.0233, -0.0233, -0.0233, -0.0233, -0.0190, 0, 0, 0, 0,
                    0, 0, 0, 0.0190, 0.0233, 0.0233, 0.0233, 0.0233, 0.0233, 0.0190, 0.0150, 0.0466, 0.0466, 0.0466, 0.0466,
                    0.0466, 0.0150, 0, 0.0224, 0.0573, 0.0690, 0.0573, 0.0224, 0);

    cv::Mat M20 = (cv::Mat_<double>(7, 7) << 0, 0.0224, 0.0394, 0.0396, 0.0394, 0.0224, 0, 0.0224, 0.0272, -0.0128, -0.0261,
                   -0.0128, 0.0272, 0.0224, 0.0394, -0.0128, -0.0528, -0.0661, -0.0528, -0.0128, 0.0394, 0.0396, -0.0261, -0.0661,
                   -0.0794, -0.0661, -0.0261, 0.0396, 0.0394, -0.0128, -0.0528, -0.0661, -0.0528, -0.0128, 0.0394, 0.0224, 0.0272,
                   -0.0128, -0.0261, -0.0128, 0.0272, 0.0224, 0, 0.0224, 0.0394, 0.0396, 0.0394, 0.0224, 0);

    // 计算Zernike矩
    cv::Mat roiImgFloat;
    roiImg.convertTo(roiImgFloat, CV_64F);

    cv::Mat ZerImgM11R, ZerImgM11I, ZerImgM20;
    cv::filter2D(roiImgFloat, ZerImgM11R, CV_64F, M11R);
    cv::filter2D(roiImgFloat, ZerImgM11I, CV_64F, M11I);
    cv::filter2D(roiImgFloat, ZerImgM20, CV_64F, M20);

    // 获取中心点值
    int center_x = radius;
    int center_y = radius;

    double z11r = ZerImgM11R.at<double>(center_y, center_x);
    double z11i = ZerImgM11I.at<double>(center_y, center_x);
    double z20 = ZerImgM20.at<double>(center_y, center_x);

    // 计算相位角
    double phi = std::atan2(z11i, z11r);

    // 计算旋转后的Z11
    double z11p = z11r * std::cos(phi) + z11i * std::sin(phi);

    // 计算L参数
    double L = z20 / z11p;

    // 计算K参数
    double K = 1.5 * z11p / std::pow(1 - L * L, 1.5);

    // 边缘检测条件
    if (std::abs(L) < 0.14 && std::abs(K) > 40) {
        // 计算亚像素偏移
        float dx = static_cast<float>((7.0 / 2.0) * L * std::cos(phi));
        float dy = static_cast<float>((7.0 / 2.0) * L * std::sin(phi));
        return cv::Point2f(edgePoint.x + dx, edgePoint.y + dy);
    }

    return edgePoint;  // 不满足条件时返回原坐标
}

/**
 * @brief 使用Zernike矩法将像素级轮廓转换为亚像素级轮廓
 * @param src 输入图像（彩色或灰度）
 * @param contour 像素级轮廓点集
 * @return 亚像素级精度的轮廓点集
 */
std::vector<cv::Point2f> CannyZernikeDetector::getSubpixelContourZernike(const cv::Mat &src, const std::vector<cv::Point> &contour) {
    cv::Mat gray;
    if (src.channels() > 1) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src.clone();
    }

    // 高斯平滑预处理
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 1.0);

    std::vector<cv::Point2f> subpixelContour;
    for (const auto &p : contour) {
        // 使用Zernike矩法计算亚像素坐标
        cv::Point2f subpixel = zernikeSubpixel(gray, p, 3);
        subpixelContour.push_back(subpixel);
    }

    // std::cout << u8"Zernike矩法提取亚像素坐标完成" << std::endl;
    return subpixelContour;
}

/**
 * @brief 使用Otsu算法自适应计算Canny边缘检测的阈值
 * @param srcImage 输入图像（彩色或灰度）
 * @return 计算得到的Canny阈值
 * @details 该方法通过计算图像的梯度幅值，应用非极大值抑制后，使用Otsu算法自动确定最佳阈值
 */
double CannyZernikeDetector::adaptiveCannyThresholdByOtsu(const cv::Mat &srcImage) {
    cv::Mat grayImage;
    if (srcImage.channels() > 1) {
        cv::cvtColor(srcImage, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = srcImage.clone();
    }

    cv::Mat gx, gy;
    cv::Mat mag, angle;

    cv::Sobel(grayImage, gx, CV_32F, 1, 0, 3);
    cv::Sobel(grayImage, gy, CV_32F, 0, 1, 3);
    // 计算梯度幅值和梯度的方向（角度）
    cv::cartToPolar(gx, gy, mag, angle, true);
    // 定义全黑非极大值抑制图像
    cv::Mat Non_maxImage = cv::Mat::zeros(grayImage.size(), CV_32FC1);
    int height = grayImage.rows;
    int width = grayImage.cols;
    // 获得非极大值抑制图像
    for (int i = 1; i < height - 1; ++i) {
        for (int j = 1; j < width - 1; ++j) {
            float g_angle = angle.at<float>(i, j);
            float K_mag = mag.at<float>(i, j);
            // 梯度方向在垂直方向
            if ((g_angle <= 112.5 && g_angle > 67.5) || (g_angle <= 292.5 && g_angle > 247.5)) {
                if (K_mag >= mag.at<float>(i - 1, j) && K_mag >= mag.at<float>(i + 1, j)) Non_maxImage.at<float>(i, j) = K_mag;
            }
            // 梯度方向在水平方向
            else if (g_angle <= 22.5 || g_angle > 337.5 || (g_angle <= 202.5 && g_angle > 157.5)) {
                if (K_mag >= mag.at<float>(i, j - 1) && K_mag >= mag.at<float>(i, j + 1)) Non_maxImage.at<float>(i, j) = K_mag;
            }
            // 梯度方向在+45方向
            else if ((g_angle <= 67.5 && g_angle > 22.5) || (g_angle <= 247.5 && g_angle > 202.5)) {
                if (K_mag >= mag.at<float>(i - 1, j - 1) && K_mag >= mag.at<float>(i + 1, j + 1))
                    Non_maxImage.at<float>(i, j) = K_mag;
            }
            // 梯度方向在-45方向
            else if ((g_angle <= 337.5 && g_angle > 292.5) || (g_angle <= 157.5 && g_angle > 112.5)) {
                if (K_mag >= mag.at<float>(i + 1, j - 1) && K_mag >= mag.at<float>(i - 1, j + 1))
                    Non_maxImage.at<float>(i, j) = K_mag;
            }
        }
    }

    cv::Mat nonMaxImage8U;
    Non_maxImage.convertTo(nonMaxImage8U, CV_8UC1);
    cv::Mat thresholdedImage;
    double TH = cv::threshold(nonMaxImage8U, thresholdedImage, 0, 255, cv::THRESH_OTSU);
    return TH;
}

/**
 * @brief 对边缘图像进行形态学处理，去除无关区域
 * @param edge 输入的边缘图像
 * @param grayImage 灰度图像，用于生成二值掩码
 * @param outputPath 输出路径前缀，用于保存中间结果
 * @return 处理后的连接边缘图像
 * @details 该方法通过二值化、闭运算、腐蚀等形态学操作，去除边缘图像中的无关区域，保留与工件相关的有效边缘
 */
cv::Mat CannyZernikeDetector::removeIrrelevantEdgeRegions(const cv::Mat& edge, const cv::Mat& grayImage) {
    // 对背光图去除工件外杂乱边缘，对正光图去除工件内杂乱边缘
    cv::Mat binaryImage;
    cv::threshold(grayImage, binaryImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    // cv::threshold(grayImage,binaryImage,30, 255, cv::THRESH_BINARY);
    cv::imwrite("E:/work/车门门环拼接/image/正面打光/9/1/binaryImage.bmp", binaryImage);
    PLOG_INFO << "baocun binaryImage";

    // 对二值图进行腐蚀，减小边缘无关区域面积，对背光和正光都有用
    cv::Mat erodedBinary;
    cv::Mat erodeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    // 进行三次腐蚀，确保缩小边缘无关区域
    cv::erode(binaryImage, erodedBinary, erodeKernel);
    cv::erode(erodedBinary, erodedBinary, erodeKernel);
    cv::erode(erodedBinary, erodedBinary, erodeKernel);

    // 进行闭运算去除腐蚀图中白色区域的空洞，可处理工件内外有少量杂物的情况
    cv::Mat closedBinary;
    cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7,7));
    cv::morphologyEx(erodedBinary, closedBinary, cv::MORPH_CLOSE, closeKernel);
    cv::imwrite("E:/work/车门门环拼接/image/正面打光/9/1/closedBinary.bmp", closedBinary);

    // 将处理后的二值图翻转，与edge相乘，保留边缘区域，去除无关区域
    cv::bitwise_not(closedBinary, closedBinary);
    cv::Mat connectedEdge;
    cv::multiply(edge, closedBinary / 255.0, connectedEdge, 1, CV_8U);

    return connectedEdge;
}

/**
 * @brief 计算中间缝隙中心线（中轴变换 + RANSAC）
 * @param image 输入图像（彩色或灰度）
 * @return 中心线直线方程参数（vx, vy, x0, y0）
 * @details 该方法通过图像二值化、中轴变换提取骨架，然后使用RANSAC算法拟合中心线
 */
cv::Vec4f CannyZernikeDetector::calculateCenterLine(const cv::Mat& image) {
    cv::Mat grayImage;
    if (image.channels() > 1) {
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = image.clone();
    }

    // 1. 图像二值化
    cv::Mat binary;
    cv::threshold(grayImage, binary, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    // 反转二值图(正光和背光不一样,因为骨架提取算法是利用腐蚀，因此背光需要反转)
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
    cv::imwrite("E:/work/车门门环拼接/image/test/1114/eroded.bmp", skel);

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
    double threshold = 3;
    int iterations = 100;
    GeometryUtils::lineRansac(centerLinePoints, centerLine, inlierPoints, threshold, iterations);

    return centerLine;
}

/**
 * @brief 根据中心线将轮廓分类到两侧（按轮廓重心分类）
 * @param contours 输入轮廓集合
 * @param centerLine 中心线直线方程参数（vx, vy, x0, y0）
 * @return 包含左右两侧轮廓的pair，first为左侧轮廓，second为右侧轮廓
 * @details 该方法通过计算每个轮廓的重心，利用中心线的法向量判断轮廓位置，将轮廓分为左侧和右侧两组
 */
std::pair<std::vector<std::vector<cv::Point>>, std::vector<std::vector<cv::Point>>>
CannyZernikeDetector::classifyContoursByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine) {
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

        float centroidX = static_cast<float>(moments.m10 / moments.m00);
        float centroidY = static_cast<float>(moments.m01 / moments.m00);

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

/**
 * @brief 根据中心线将轮廓分类到两侧（按点分类）
 * @param contours 输入轮廓集合
 * @param centerLine 中心线直线方程参数（vx, vy, x0, y0）
 * @return 包含左右两侧轮廓点的vector，第一个元素为右侧轮廓点，第二个元素为左侧轮廓点
 * @details 该方法遍历轮廓中的每个点，利用中心线的法向量判断每个点的位置，
 *          将轮廓点按位置分为左侧和右侧两组，适用于需要精细点级分类的场景
 */
std::vector<std::vector<cv::Point>>
CannyZernikeDetector::classifyContourPointsByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine)
{
    std::vector<std::vector<cv::Point>> contoursLeftAndRight;
    std::vector<cv::Point> leftContours;  // 中心线左侧的轮廓
    std::vector<cv::Point> rightContours; // 中心线右侧的轮廓

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

        // 遍历轮廓中的每个点，按点分类
        for (const auto& point : contour) {
            // 计算点到直线上点(x0, y0)的向量
            float dx = point.x - x0;
            float dy = point.y - y0;

            // 计算向量与法向量的点积
            float dotProduct = dx * nx + dy * ny;

            // 根据点积的正负判断点在直线的哪一侧
            // 点积 > 0：在法向量方向（右侧）
            // 点积 < 0：在法向量反方向（左侧）
            if (dotProduct > 0) {
                rightContours.push_back(point);
            } else {
                leftContours.push_back(point);
            }
        }
    }
    contoursLeftAndRight.push_back(rightContours);
    contoursLeftAndRight.push_back(leftContours);

    return contoursLeftAndRight;
}

/**
 * @brief 计算二值图中亮连通域（255像素）的数量
 * @param binImg 输入的二值图像（单通道，CV_8UC1，0表示黑，255表示亮）
 * @param is8Neighbor 是否使用8邻域（true=8邻域，false=4邻域）
 * @return 连通域数量
 */
int CannyZernikeDetector::countBrightConnectedComponents(const cv::Mat& grayImage, bool is8Neighbor) {
    // 检查输入图像是否有效
    cv::Mat binImg;
    cv::threshold(grayImage, binImg, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    if (binImg.empty() || binImg.channels() != 1) {
        std::cerr << "错误：输入图像为空或不是单通道二值图！" << std::endl;
        return -1;
    }

    int rows = binImg.rows;    // 图像高度（行数）
    int cols = binImg.cols;    // 图像宽度（列数）
    cv::Mat visited = cv::Mat::zeros(rows, cols, CV_8UC1);  // 标记已访问的像素（0=未访问，1=已访问）
    int componentCount = 0;    // 连通域数量

    // 定义邻域方向（4邻域：上下左右；8邻域：加对角线）
    std::vector<cv::Point> dirs;
    if (is8Neighbor) {
        dirs = {cv::Point(-1, -1), cv::Point(-1, 0), cv::Point(-1, 1),
                cv::Point(0, -1),          cv::Point(0, 1),
                cv::Point(1, -1),  cv::Point(1, 0), cv::Point(1, 1)};
    } else {
        dirs = {cv::Point(-1, 0),  // 上
                cv::Point(1, 0),   // 下
                cv::Point(0, -1),  // 左
                cv::Point(0, 1)};  // 右
    }

    // 遍历图像每个像素
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // 若当前像素是亮区（255）且未被访问，则开始BFS标记连通域
            if (binImg.at<uchar>(i, j) == 255 && visited.at<uchar>(i, j) == 0) {
                componentCount++;  // 连通域数量+1

                // BFS队列，存储待访问的像素坐标
                std::queue<cv::Point> q;
                q.push(cv::Point(j, i));  // 注意：OpenCV中Point(x,y)，x=列，y=行
                visited.at<uchar>(i, j) = 1;  // 标记当前像素为已访问

                // 遍历当前连通域的所有像素
                while (!q.empty()) {
                    cv::Point curr = q.front();
                    q.pop();

                    // 检查所有邻域像素
                    for (const cv::Point& dir : dirs) {
                        int x = curr.x + dir.x;  // 邻域列坐标
                        int y = curr.y + dir.y;  // 邻域行坐标

                        // 确保邻域像素在图像范围内，且是亮区且未访问
                        if (x >= 0 && x < cols && y >= 0 && y < rows) {
                            if (binImg.at<uchar>(y, x) == 255 && visited.at<uchar>(y, x) == 0) {
                                visited.at<uchar>(y, x) = 1;  // 标记为已访问
                                q.push(cv::Point(x, y));      // 加入队列继续遍历
                            }
                        }
                    }
                }
            }
        }
    }

    return componentCount;
}

/**
 * @brief 执行拼缝两边轮廓检测
 * @param inputImage 输入图像（彩色或灰度）
 * @return 包含左右两侧亚像素轮廓的vector，第一个元素为右侧轮廓，第二个元素为左侧轮廓
 */
std::vector<std::vector<cv::Point2f>> CannyZernikeDetector::detectContours(const cv::Mat& inputImage)
{
    ImageTools imageTools;
    cv::Mat grayImage;
    if (inputImage.channels() > 1) {
        cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = inputImage.clone();
    }
    cv::GaussianBlur(grayImage, grayImage, cv::Size(7, 7), 0, 0);

    // 连通域分析：检查工件是否发生碰撞
    // 使用BFS算法计算亮区连通域数量（使用8邻域）
    int brightComponentCount = countBrightConnectedComponents(grayImage, true);
    // 如果亮区连通域数量大于1，说明工件可能发生碰撞
    if (brightComponentCount > 1) {
        PLOG_INFO << "警告：检测到 " << brightComponentCount << " 个亮区连通域，工件可能已发生碰撞！";
    }

    // 边缘检测
    double TH = adaptiveCannyThresholdByOtsu(grayImage);
    double TL = TH * 0.5;
    cv::Mat edge;
    cv::Canny(grayImage, edge, TL, TH);
    cv::imwrite("E:/work/车门门环拼接/image/test/1114/edge.bmp", edge);
    // 形态学处理，去除无关区域的边缘
    cv::Mat connectedEdge = removeIrrelevantEdgeRegions(edge, grayImage);
    cv::imwrite("E:/work/车门门环拼接/image/test/1114/connectedEdge.bmp", connectedEdge);
    // 计算中间缝隙中心线
    cv::Vec4f centerLine = calculateCenterLine(inputImage);
    imageTools.drawLineAndSave(grayImage, centerLine, "E:/work/车门门环拼接/image/test/1114/centerLine.bmp");
    // 提取并筛选轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(connectedEdge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    imageTools.drawColorfulContoursAndSave(grayImage, contours,
                                           "E:/work/车门门环拼接/image/test/1114/allContours.bmp");
    std::vector<std::vector<cv::Point>> filteredContours = imageTools.filterContours(contours);
    imageTools.drawColorfulContoursAndSave(grayImage, filteredContours,
                                           "E:/work/车门门环拼接/image/test/1114/filterContours.bmp");
    // 根据中心线分类轮廓
    auto contoursLeftAndRight = classifyContourPointsByCenterLine(filteredContours, centerLine);
    imageTools.drawColorfulContoursAndSave(grayImage, contoursLeftAndRight,
                                           "E:/work/车门门环拼接/image/test/1114/left_contours.bmp");
    // 亚像素轮廓提取
    std::vector<std::vector<cv::Point2f>> subpixelConturs;
    for (const auto& contour : contoursLeftAndRight) {
        std::vector<cv::Point2f> c = getSubpixelContourZernike(inputImage, contour);
        subpixelConturs.push_back(c);
    }


    return subpixelConturs;
}

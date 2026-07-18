#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <set>
#include <numeric>

#include <plog/Log.h>

#include "canny_zernike_detector.h"
#include "src/utils/image_tools.h"
#include "src/utils/geometry_utils.h"
#include "src/utils/scoped_timer.h"


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
    SCOPED_TIMER("亚像素轮廓提取");
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
 * @param grayImage 输入灰度图像
 * @return 计算得到的Canny高阈值
 * @details 直接在灰度图上应用Otsu算法计算最佳阈值，简化计算流程
 */
double CannyZernikeDetector::adaptiveCannyThresholdByOtsu(const cv::Mat &grayImage) {
    SCOPED_TIMER("计算自适应canny阈值");
    cv::Mat thresholdedImage;
    double TH = cv::threshold(grayImage, thresholdedImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
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
    SCOPED_TIMER("去除无关边缘区域（Otsu二值化）");
    // 对背光图去除工件外杂乱边缘，对正光图去除工件内杂乱边缘
    cv::Mat binaryImage;
    // 直接使用Otsu二值化，替代慢速的pyrMeanShiftFiltering
    cv::threshold(grayImage, binaryImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

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

    // 将处理后的二值图翻转，与edge相乘，保留边缘区域，去除无关区域
    cv::bitwise_not(closedBinary, closedBinary);
    cv::Mat connectedEdge;
    cv::multiply(edge, closedBinary / 255.0, connectedEdge, 1, CV_8U);

    return connectedEdge;
}

/**
 * @brief 用 Otsu + minAreaRect 对 edge 做二次过滤
 * @param edge 已经过 removeIrrelevantEdgeRegions 处理的单通道边缘图
 * @param binary 预计算的 Otsu 二值图（由 detectContours 统一计算）
 * @return 只保留落在工件最小外接旋转矩形内的边缘图
 * @details cv::findNonZero 收集白点 → cv::minAreaRect 求旋转外接矩形 →
 *          fillConvexPoly 画 mask → bitwise_and 掩蔽
 */
cv::Mat CannyZernikeDetector::filterEdgesByMinAreaRect(const cv::Mat& edge, const cv::Mat& binary) {
    SCOPED_TIMER("minAreaRect二次过滤");
    // 收集所有白点
    std::vector<cv::Point2i> whitePoints;
    cv::findNonZero(binary, whitePoints);
    if (whitePoints.empty()) {
        return edge.clone();   // 无白点时退化返回原边缘，保证流水线不崩
    }

    // 最小外接旋转矩形
    cv::RotatedRect rotatedRect = cv::minAreaRect(whitePoints);
    rotatedRect.size.width *= 0.9f;
    rotatedRect.size.height *= 0.9f;

    // 4 角点 → int 多边形
    cv::Point2f corners2f[4];
    rotatedRect.points(corners2f);
    std::vector<cv::Point> polygon;
    polygon.reserve(4);
    for (const auto& p : corners2f) {
        polygon.emplace_back(cvRound(p.x), cvRound(p.y));
    }

    // 在二值图上画出缩小后的外接矩形，保存可视化
    cv::Mat binaryWithRect;
    cv::cvtColor(binary, binaryWithRect, cv::COLOR_GRAY2BGR);
    cv::polylines(binaryWithRect, polygon, true, cv::Scalar(0, 255, 0), 1);
    cv::imwrite("E:/work/Car_door_ring_splicing/image/背面打光/260716/binary_with_minAreaRect.bmp", binaryWithRect);

    // 画旋转矩形为掩码，再与 edge 按位 AND
    cv::Mat mask = cv::Mat::zeros(edge.size(), CV_8UC1);
    cv::fillConvexPoly(mask, polygon, 255);
    cv::Mat filteredEdge;
    cv::bitwise_and(edge, mask, filteredEdge);
    return filteredEdge;
}

/**
 * @brief Zhang-Suen骨架化方法计算中心线（用于对比）
 * @param grayImage 预计算的灰度图
 * @return 中心线直线方程参数（vx, vy, x0, y0）
 */
cv::Vec4f CannyZernikeDetector::calculateCenterLineWithZhangSuen(const cv::Mat& grayImage) {
    // 提取图像中间部分：以图像中心为中心，裁剪区域
    cv::Point2f imgCenter(grayImage.cols / 2.0f, grayImage.rows / 2.0f);
    int roiWidth = grayImage.cols;
    int roiHeight = grayImage.rows;
    // cv::Rect 需要左上角坐标，由中心点推算
    int roiX = static_cast<int>(imgCenter.x - roiWidth / 2.0f);
    int roiY = static_cast<int>(imgCenter.y - roiHeight / 2.0f);
    cv::Rect centerRect(roiX, roiY, roiWidth, roiHeight);
    cv::Mat centerRegion = grayImage(centerRect).clone();

    // 1. 图像二值化（只对中间区域处理）
    cv::Mat binary;
    cv::threshold(centerRegion, binary, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    cv::bitwise_not(binary, binary);

    // 2. Zhang-Suen骨架化
    cv::Mat det = binary.clone();
    det /= 255;

    static std::vector<bool> List1, List2;
    static bool initialized = false;
    if (!initialized) {
        List1.assign(256, false);
        List2.assign(256, false);

        for (int n = 0; n < 256; n++) {
            std::vector<int> p(8);
            for (int k = 0; k < 8; k++) {
                p[k] = (n >> k) & 1;
            }

            int Np = std::accumulate(p.begin(), p.end(), 0);
            int Tp = 0;
            for (int k = 0; k < 8; k++) {
                int diff = p[(k + 1) % 8] - p[k];
                if (diff == 1) Tp++;
            }

            if (Np > 1 && Np < 7 && Tp == 1) {
                if (p[0] * p[2] * p[4] == 0 && p[6] * p[2] * p[4] == 0) {
                    List1[n] = true;
                }
                if (p[0] * p[2] * p[6] == 0 && p[0] * p[4] * p[6] == 0) {
                    List2[n] = true;
                }
            }
        }
        initialized = true;
    }

    int mat[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
    bool changed = true;
    int maxIterations = 1000; // 防止无限循环
    int iteration = 0;

    while (changed && iteration < maxIterations) {
        changed = false;
        iteration++;

        // 局部变量，避免多线程竞争（不同 ROI 尺寸会导致 static 变量错乱）
        cv::Mat label1 = cv::Mat::zeros(det.size(), CV_8UC1);
        cv::Mat label2 = cv::Mat::zeros(det.size(), CV_8UC1);

        for (int y = 1; y < det.rows - 1; y++) {
            for (int x = 1; x < det.cols - 1; x++) {
                if (det.at<uchar>(y, x)) {
                    uchar p[8] = {
                        det.at<uchar>(y - 1, x),
                        det.at<uchar>(y - 1, x + 1),
                        det.at<uchar>(y, x + 1),
                        det.at<uchar>(y + 1, x + 1),
                        det.at<uchar>(y + 1, x),
                        det.at<uchar>(y + 1, x - 1),
                        det.at<uchar>(y, x - 1),
                        det.at<uchar>(y - 1, x - 1)
                    };

                    int idx = 0;
                    for (int k = 0; k < 8; k++) {
                        idx += p[k] * mat[k];
                    }
                    if (List1[idx]) {
                        label1.at<uchar>(y, x) = 1;
                        changed = true;
                    }
                }
            }
        }
        det.setTo(0, label1);

        for (int y = 1; y < det.rows - 1; y++) {
            for (int x = 1; x < det.cols - 1; x++) {
                if (det.at<uchar>(y, x)) {
                    uchar p[8] = {
                        det.at<uchar>(y - 1, x),
                        det.at<uchar>(y - 1, x + 1),
                        det.at<uchar>(y, x + 1),
                        det.at<uchar>(y + 1, x + 1),
                        det.at<uchar>(y + 1, x),
                        det.at<uchar>(y + 1, x - 1),
                        det.at<uchar>(y, x - 1),
                        det.at<uchar>(y - 1, x - 1)
                    };

                    int idx = 0;
                    for (int k = 0; k < 8; k++) {
                        idx += p[k] * mat[k];
                    }
                    if (List2[idx]) {
                        label2.at<uchar>(y, x) = 1;
                        changed = true;
                    }
                }
            }
        }
        det.setTo(0, label2);
    }

    det *= 255;
    cv::imwrite("E:/work/Car_door_ring_splicing/image/背面打光/260716/zhangSuen_skeleton.bmp", det);

    // 3. 提取点+RANSAC
    std::vector<cv::Point2f> centerLinePoints;
    for (int y = 0; y < det.rows; y++) {
        for (int x = 0; x < det.cols; x++) {
            if (det.at<uchar>(y, x) > 0) {
                centerLinePoints.push_back(cv::Point2f(x, y));
            }
        }
    }

    cv::Vec4f centerLine;
    std::vector<cv::Point2f> inlierPoints;
    GeometryUtils::lineRansac(centerLinePoints, centerLine, inlierPoints, 3.0, 100);

    return centerLine;
}

/**
 * @brief 扫描线法计算中心线
 * @param grayImage 预计算的灰度图
 * @return 中心线直线方程参数（vx, vy, x0, y0），坐标已转换为原图坐标系
 * @details 提取 ROI → 二值化 → minAreaRect → 沿长轴法线方向扫描取中点 → RANSAC 拟合
 *          内部完成可视化保存（scanline_method_N.bmp）
 */
cv::Vec4f CannyZernikeDetector::calculateCenterLineByScanline(const cv::Mat& grayImage) {
    // 1. ROI 提取
    cv::Point2f imgCenter(grayImage.cols / 2.0f, grayImage.rows / 2.0f);
    int roiWidth = grayImage.cols / 4;
    int roiHeight = grayImage.rows;
    int roiX = static_cast<int>(imgCenter.x - roiWidth / 2.0f);
    int roiY = static_cast<int>(imgCenter.y - roiHeight / 2.0f);
    cv::Rect centerRect(roiX, roiY, roiWidth, roiHeight);
    cv::Mat centerRegion = grayImage(centerRect).clone();

    // 2. 二值化（白色 = 缝隙区域）
    cv::Mat binary;
    cv::threshold(centerRegion, binary, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    cv::bitwise_not(binary, binary);

    // 3. 计算白色区域最小外接旋转矩形
    std::vector<cv::Point2i> whitePoints;
    cv::findNonZero(binary, whitePoints);
    if (whitePoints.empty()) {
        return cv::Vec4f(1, 0, imgCenter.x, imgCenter.y);
    }
    cv::RotatedRect rotatedRect = cv::minAreaRect(whitePoints);

    // 4. 确定长轴方向（OpenCV 的 angle 关联 width 边，height 更长时需 +90）
    float angleDeg = rotatedRect.angle;
    float longLen, shortLen;
    if (rotatedRect.size.width >= rotatedRect.size.height) {
        longLen = rotatedRect.size.width;
        shortLen = rotatedRect.size.height;
    } else {
        longLen = rotatedRect.size.height;
        shortLen = rotatedRect.size.width;
        angleDeg += 90.0f;
    }
    float angleRad = angleDeg * static_cast<float>(CV_PI) / 180.0f;

    // 长轴方向向量 和 法线方向向量
    cv::Point2f dir(std::cos(angleRad), std::sin(angleRad));
    cv::Point2f normVec(-std::sin(angleRad), std::cos(angleRad));
    cv::Point2f rectCenter = rotatedRect.center;

    // 5. 沿长轴等步长扫描，每条线沿法线方向找黑白跳变对
    std::vector<cv::Point2f> centerLinePoints;
    float halfLong = longLen / 2.0f;
    float halfShort = shortLen / 2.0f;
    float step = 2.0f;

    for (float t = -halfLong; t <= halfLong; t += step) {
        cv::Point2f lineCenter = rectCenter + dir * t;

        std::vector<float> transitions;
        bool prevWhite = false;

        for (float s = -halfShort; s <= halfShort; s += 1.0f) {
            cv::Point2f pt = lineCenter + normVec * s;
            int px = cvRound(pt.x);
            int py = cvRound(pt.y);

            if (px < 0 || px >= binary.cols || py < 0 || py >= binary.rows) {
                prevWhite = false;
                continue;
            }

            bool isWhite = binary.at<uchar>(py, px) > 0;
            if (isWhite != prevWhite) {
                transitions.push_back(s);
                prevWhite = isWhite;
            }
        }

        if (transitions.size() >= 2) {
            float midS = (transitions[0] + transitions[1]) / 2.0f;
            cv::Point2f midPoint = lineCenter + normVec * midS;
            centerLinePoints.push_back(midPoint);
        }
    }

    // 6. RANSAC 拟合中心线（ROI 局部坐标）
    cv::Vec4f centerLine(1, 0, rectCenter.x, rectCenter.y);
    std::vector<cv::Point2f> inlierPoints;
    if (!centerLinePoints.empty()) {
        GeometryUtils::lineRansac(centerLinePoints, centerLine, inlierPoints, 3.0, 100);
    }

    // 7. 可视化：在原图灰度图上画 minAreaRect（红）+ 中点（绿）+ 拟合直线（蓝），保存
    static int saveCounter = 0;
    saveCounter++;
    cv::Mat visImg;
    cv::cvtColor(grayImage, visImg, cv::COLOR_GRAY2BGR);

    cv::Point2f corners[4];
    rotatedRect.points(corners);
    for (int i = 0; i < 4; ++i) {
        corners[i].x += roiX;
        corners[i].y += roiY;
    }
    for (int i = 0; i < 4; ++i) {
        cv::line(visImg, corners[i], corners[(i + 1) % 4], cv::Scalar(0, 0, 255), 1);
    }
    for (const auto& pt : centerLinePoints) {
        cv::circle(visImg, cv::Point2f(pt.x + roiX, pt.y + roiY), 1, cv::Scalar(0, 255, 0), -1);
    }
    cv::Point2f p1(centerLine[2] - 1000 * centerLine[0] + roiX,
                   centerLine[3] - 1000 * centerLine[1] + roiY);
    cv::Point2f p2(centerLine[2] + 1000 * centerLine[0] + roiX,
                   centerLine[3] + 1000 * centerLine[1] + roiY);
    cv::line(visImg, p1, p2, cv::Scalar(255, 0, 0), 2);

    static std::string visDir = "E:/work/Car_door_ring_splicing/image/背面打光/260716/";
    cv::imwrite(visDir + "scanline_method_" + std::to_string(saveCounter) + ".bmp", visImg);

    // 返回原图坐标系的中心线
    centerLine[2] += roiX;
    centerLine[3] += roiY;
    return centerLine;
}

/**
 * @brief 计算中间缝隙中心线
 * @param grayImage 预计算的灰度图
 * @return 中心线直线方程参数（vx, vy, x0, y0），原图坐标系
 * @details 当前采用扫描线法，内部完成 ROI 提取、RANSAC 拟合、可视化保存
 */
cv::Vec4f CannyZernikeDetector::calculateCenterLineWithoutCollision(const cv::Mat &grayImage)
{
    SCOPED_TIMER("计算中心线");
    return calculateCenterLineByScanline(grayImage);
}

/**
 * @brief 计算中间缝隙中心线
 * @param grayImage 预计算的灰度图
 * @return 中心线直线方程参数（vx, vy, x0, y0），原图坐标系
 * @details 当前采用扫描线法，内部完成 ROI 提取、RANSAC 拟合、可视化保存
 */
cv::Vec4f CannyZernikeDetector::calculateCenterLineWithCollision(const cv::Mat &grayImage)
{
    SCOPED_TIMER("计算中心线");
    return calculateCenterLineWithZhangSuen(grayImage);
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
 * @param binary 输入的二值图像（单通道，CV_8UC1，0表示黑，255表示亮）
 * @param is8Neighbor 是否使用8邻域（true=8邻域，false=4邻域）
 * @return 连通域数量
 */
int CannyZernikeDetector::countBrightConnectedComponents(const cv::Mat& binary, bool is8Neighbor, int minArea) {
    // 检查输入图像是否有效
    if (binary.empty() || binary.channels() != 1) {
        PLOG_ERROR << "输入图像为空或不是单通道二值图！";
        return -1;
    }

    int rows = binary.rows;    // 图像高度（行数）
    int cols = binary.cols;    // 图像宽度（列数）
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
            if (binary.at<uchar>(i, j) == 255 && visited.at<uchar>(i, j) == 0) {
                // BFS队列，存储待访问的像素坐标
                std::queue<cv::Point> q;
                q.push(cv::Point(j, i));  // 注意：OpenCV中Point(x,y)，x=列，y=行
                visited.at<uchar>(i, j) = 1;  // 标记当前像素为已访问
                int componentPixelCount = 1;  // 起始点已计入

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
                            if (binary.at<uchar>(y, x) == 255 && visited.at<uchar>(y, x) == 0) {
                                visited.at<uchar>(y, x) = 1;  // 标记为已访问
                                ++componentPixelCount;
                                q.push(cv::Point(x, y));      // 加入队列继续遍历
                            }
                        }
                    }
                }

                // 只有面积达到阈值才计数（minArea <= 0 时不过滤，保持原行为）
                if (minArea <= 0 || componentPixelCount >= minArea) {
                    ++componentCount;
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
    PLOG_INFO << "开始轮廓检测";
    cv::Mat grayImage;
    if (inputImage.channels() > 1) {
        cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = inputImage.clone();
    }
    cv::GaussianBlur(grayImage, grayImage, cv::Size(7, 7), 0, 0);

    cv::Mat binaryImage;
    cv::threshold(grayImage, binaryImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 碰撞检测 + 路径分叉
    int brightComponentCount = 0;
    {
        SCOPED_TIMER("连通域分析");
        brightComponentCount = countBrightConnectedComponents(binaryImage, true, 500);
    }

    std::vector<std::vector<cv::Point2f>> subpixelConturs;
    if (brightComponentCount > 1) {
        PLOG_INFO << "检测到 " << brightComponentCount
                  << " 个亮区连通域，工件可能已碰撞，进入碰撞处理路径";
        subpixelConturs = detectContoursWithCollision(grayImage, binaryImage, inputImage);
    } else {
        subpixelConturs = detectContoursWithoutCollision(grayImage, binaryImage, inputImage);
    }

    PLOG_INFO << "轮廓检测完成";
    return subpixelConturs;
}

/**
 * @brief 无碰撞情况下的轮廓检测流水线
 * @param grayImage   已高斯模糊的灰度图（detectContours 预处理产物）
 * @param binaryImage Otsu 二值化图（detectContours 预处理产物）
 * @param inputImage  原始输入图（亚像素提取用）
 * @return 左右两侧亚像素轮廓，[0]=右、[1]=左
 */
std::vector<std::vector<cv::Point2f>> CannyZernikeDetector::detectContoursWithoutCollision(
    const cv::Mat& grayImage, const cv::Mat& binaryImage, const cv::Mat& inputImage)
{
    ImageTools imageTools;

    // 边缘检测
    double TH = adaptiveCannyThresholdByOtsu(grayImage);
    double TL = TH * 0.5;
    cv::Mat edge;
    cv::Canny(grayImage, edge, TL, TH);
    // cv::imwrite("E:/work/Car_door_ring_splicing/image/背面打光/260716/edge.bmp", edge);

    // 形态学处理，去除无关区域的边缘
    cv::Mat connectedEdge = removeIrrelevantEdgeRegions(edge, grayImage);

    // 二次过滤：Otsu + minAreaRect，只保留落在工件外接旋转矩形内的边缘
    cv::Mat filteredEdge = filterEdgesByMinAreaRect(connectedEdge, binaryImage);
    // cv::imwrite("E:/work/Car_door_ring_splicing/image/背面打光/260716/filteredEdge.bmp", filteredEdge);

    // 提取并筛选轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(filteredEdge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<std::vector<cv::Point>> filteredContours = imageTools.filterContours(contours);

    // 计算中间缝隙中心线
    cv::Vec4f centerLine = calculateCenterLineWithoutCollision(grayImage);
    // imageTools.drawLineAndSave(grayImage, centerLine, "E:/work/Car_door_ring_splicing/image/背面打光/260716/centerLine.bmp");

    // 根据中心线分类轮廓
    auto contoursLeftAndRight = classifyContourPointsByCenterLine(filteredContours, centerLine);

    // 亚像素轮廓提取
    std::vector<std::vector<cv::Point2f>> subpixelConturs;
    for (const auto& contour : contoursLeftAndRight) {
        std::vector<cv::Point2f> c = getSubpixelContourZernike(inputImage, contour);
        subpixelConturs.push_back(c);
    }

    return subpixelConturs;
}

// =============================================================================
// 碰撞路径专用子步骤：minAreaRect 长轴扫描法
// =============================================================================

cv::RotatedRect CannyZernikeDetector::computeWhiteMinAreaRect(const cv::Mat& binary) {
    std::vector<cv::Point2i> whitePoints;
    cv::findNonZero(binary, whitePoints);
    if (whitePoints.empty()) {
        PLOG_WARNING << "[碰撞路径] 白色区域为空，回退到图像中心默认矩形";
        return cv::RotatedRect(cv::Point2f(binary.cols / 2.0f, binary.rows / 2.0f),
                               cv::Size2f(static_cast<float>(binary.cols), static_cast<float>(binary.rows)),
                               0.0f);
    }
    return cv::minAreaRect(whitePoints);
}

RectFrame CannyZernikeDetector::establishRectFrame(const cv::RotatedRect& rect) {
    RectFrame frame;
    frame.center = rect.center;
    // OpenCV 的 angle 关联 width 边；height 更长时需 +90，使 longDir 指向真正的长边
    float angleDeg = rect.angle;
    if (rect.size.width >= rect.size.height) {
        frame.longLen = rect.size.width;
        frame.shortLen = rect.size.height;
    } else {
        frame.longLen = rect.size.height;
        frame.shortLen = rect.size.width;
        angleDeg += 90.0f;
    }
    float angleRad = angleDeg * static_cast<float>(CV_PI) / 180.0f;
    frame.longDir = cv::Point2f(std::cos(angleRad), std::sin(angleRad));
    frame.shortDir = cv::Point2f(-std::sin(angleRad), std::cos(angleRad));
    return frame;
}

std::vector<float> CannyZernikeDetector::collectTransitionProjections(const cv::Mat& binary, const RectFrame& frame) {
    // 沿长轴扫描：扫描线平行于 longDir（固定 shortDir 偏移 s），沿 longDir 方向 t 行走，
    // 检测白↔黑跳变。点 = center + shortDir*s + longDir*t，其到 longDir 的投影即 t。
    std::vector<float> projections;
    const float halfLong = frame.longLen / 2.0f;
    const float halfShort = frame.shortLen / 2.0f;

    for (float s = -halfShort; s <= halfShort; s += 1.0f) {
        cv::Point2f lineStart = frame.center + frame.shortDir * s;
        bool prevValid = false;
        bool prevWhite = false;
        for (float t = -halfLong; t <= halfLong; t += 1.0f) {
            cv::Point2f pt = lineStart + frame.longDir * t;
            int px = cvRound(pt.x);
            int py = cvRound(pt.y);
            if (px < 0 || px >= binary.cols || py < 0 || py >= binary.rows) {
                prevValid = false;  // 跨越越界间隙，不视为连续跳变
                continue;
            }
            bool isWhite = binary.at<uchar>(py, px) > 0;
            if (prevValid && (isWhite != prevWhite)) {
                projections.push_back(t);
            }
            prevWhite = isWhite;
            prevValid = true;
        }
    }
    return projections;
}

std::pair<FloatRange, FloatRange> CannyZernikeDetector::findTwoDenseIntervals(
    const std::vector<float>& projections, const RectFrame& frame) {
    const float halfLong = frame.longLen / 2.0f;
    if (projections.size() < 2) {
        return {FloatRange{-halfLong, 0.0f}, FloatRange{0.0f, halfLong}};
    }

    // 兜底：把投影排序后按最大间隙二分
    auto splitByMaxGap = [](std::vector<float> proj) -> std::pair<FloatRange, FloatRange> {
        std::sort(proj.begin(), proj.end());
        int gapIdx = 0;
        float maxGap = -1.0f;
        for (size_t i = 0; i + 1 < proj.size(); ++i) {
            float g = proj[i + 1] - proj[i];
            if (g > maxGap) {
                maxGap = g;
                gapIdx = static_cast<int>(i);
            }
        }
        FloatRange a{proj.front(), proj[gapIdx]};
        FloatRange b{proj[gapIdx + 1], proj.back()};
        return std::make_pair(a, b);
    };

    // 1D 直方图
    const float binWidth = 150.0f;
    int numBins = std::max(1, static_cast<int>(std::ceil(frame.longLen / binWidth)));
    std::vector<int> hist(numBins, 0);
    for (float t : projections) {
        int bin = static_cast<int>((t + halfLong) / binWidth);
        if (bin >= 0 && bin < numBins) {
            hist[bin]++;
        }
    }
    // 3-tap 平滑
    std::vector<float> smooth(numBins, 0.0f);
    for (int i = 0; i < numBins; ++i) {
        float sum = 0.0f;
        int cnt = 0;
        for (int k = -1; k <= 1; ++k) {
            int j = i + k;
            if (j >= 0 && j < numBins) {
                sum += static_cast<float>(hist[j]);
                ++cnt;
            }
        }
        smooth[i] = sum / static_cast<float>(cnt);
    }
    // 阈值 + 连续超阈值区间
    float maxVal = *std::max_element(smooth.begin(), smooth.end());
    float threshold = maxVal * 0.5f;
    struct Interval { int loBin; int hiBin; float mass; };
    std::vector<Interval> intervals;
    int i = 0;
    while (i < numBins) {
        if (threshold > 0.0f && smooth[i] >= threshold) {
            int start = i;
            float mass = 0.0f;
            while (i < numBins && smooth[i] >= threshold) {
                mass += smooth[i];
                ++i;
            }
            intervals.push_back({start, i - 1, mass});
        } else {
            ++i;
        }
    }
    auto binToRange = [&](int loBin, int hiBin) -> FloatRange {
        return FloatRange{loBin * binWidth - halfLong, (hiBin + 1) * binWidth - halfLong};
    };
    if (intervals.size() >= 2) {
        // 取质量最大的两段，再按位置（左、右）排序返回
        std::sort(intervals.begin(), intervals.end(),
                  [](const Interval& a, const Interval& b) { return a.mass > b.mass; });
        FloatRange a = binToRange(intervals[0].loBin, intervals[0].hiBin);
        FloatRange b = binToRange(intervals[1].loBin, intervals[1].hiBin);
        if (a.lo > b.lo) {
            std::swap(a, b);
        }
        return {a, b};
    }
    return splitByMaxGap(projections);
}

std::vector<cv::RotatedRect> CannyZernikeDetector::splitMinAreaRect(
    const cv::RotatedRect& parent, const RectFrame& frame,
    const std::pair<FloatRange, FloatRange>& intervals) {
    // 子矩形长轴沿 frame.longDir；据父矩形长短边归属设置 size，使子矩形长轴方向与父矩形一致
    const bool longIsWidth = parent.size.width >= parent.size.height;
    std::vector<cv::RotatedRect> subRects;
    auto build = [&](const FloatRange& r) -> cv::RotatedRect {
        float mid = (r.lo + r.hi) / 2.0f;
        float longExtent = std::max(r.hi - r.lo, 1.0f);
        cv::RotatedRect sub;
        sub.center = frame.center + frame.longDir * mid;
        sub.angle = parent.angle;
        sub.size = longIsWidth ? cv::Size2f(longExtent, frame.shortLen)
                               : cv::Size2f(frame.shortLen, longExtent);
        return sub;
    };
    subRects.push_back(build(intervals.first));
    subRects.push_back(build(intervals.second));
    return subRects;
}

void CannyZernikeDetector::collectEdgePointsInRect(const cv::Mat& edgeMap, const cv::RotatedRect& subRect,
                                                   std::vector<cv::Point2f>& outEdgePoints) {
    outEdgePoints.clear();
    std::vector<cv::Point> edgePx;
    cv::findNonZero(edgeMap, edgePx);
    outEdgePoints.reserve(edgePx.size());
    for (const auto& p : edgePx) {
        cv::Point2f pf(static_cast<float>(p.x), static_cast<float>(p.y));
        if (GeometryUtils::isPointInRotatedRect(pf, subRect)) {
            outEdgePoints.push_back(pf);
        }
    }
    if (outEdgePoints.size() < 2) {
        PLOG_WARNING << "[碰撞路径] 子矩形内边缘点不足 2 个";
    }
}

cv::Vec4f CannyZernikeDetector::centerLineFromFrame(const RectFrame& frame) {
    return cv::Vec4f(frame.longDir.x, frame.longDir.y, frame.center.x, frame.center.y);
}

cv::Vec4f CannyZernikeDetector::calculateCenterLineInGap(
    const cv::Mat& binary, const RectFrame& frame,
    const std::pair<FloatRange, FloatRange>& intervals) {
    // 扫描范围 = 两子矩形在长轴上夹着的中间区域 [first.hi, second.lo]（first/second 已按 lo 升序）
    float tLo = std::min(intervals.first.hi, intervals.second.lo);
    float tHi = std::max(intervals.first.hi, intervals.second.lo);
    const float halfShort = frame.shortLen / 2.0f;
    const float kStep = 2.0f;

    // 退化兜底：中间区域扫描中点不足时退回长轴中线
    cv::Vec4f centerLine = centerLineFromFrame(frame);

    // 沿长轴等步长扫描；每条线沿短轴(法向)找白↔黑跳变对，取前两跳变中点为缝隙中心
    std::vector<cv::Point2f> centerLinePoints;
    for (float t = tLo; t <= tHi; t += kStep) {
        cv::Point2f lineCenter = frame.center + frame.longDir * t;
        std::vector<float> transitions;
        bool prevWhite = false;
        for (float s = -halfShort; s <= halfShort; s += 1.0f) {
            cv::Point2f pt = lineCenter + frame.shortDir * s;
            int px = cvRound(pt.x);
            int py = cvRound(pt.y);
            if (px < 0 || px >= binary.cols || py < 0 || py >= binary.rows) {
                prevWhite = false;
                continue;
            }
            bool isWhite = binary.at<uchar>(py, px) > 0;
            if (isWhite != prevWhite) {
                transitions.push_back(s);
                prevWhite = isWhite;
            }
        }
        if (transitions.size() >= 2) {
            float midS = (transitions[0] + transitions[1]) / 2.0f;
            centerLinePoints.push_back(lineCenter + frame.shortDir * midS);
        }
    }

    const size_t kMinScanPoints = 50;  // 扫描中点数低于此阈值则退回最小外接矩形中心线
    if (centerLinePoints.size() >= kMinScanPoints) {
        std::vector<cv::Point2f> inliers;
        GeometryUtils::lineRansac(centerLinePoints, centerLine, inliers, 3.0, 100);
    } else {
        centerLine = centerLineFromFrame(frame);  // 显式退回最小外接矩形中心线
        PLOG_WARNING << "[碰撞路径] 中间区域扫描中点 " << centerLinePoints.size()
                     << " < " << kMinScanPoints << "，退回最小外接矩形中心线";
    }

    // [DEBUG] 可视化：扫描中点(绿) + 拟合中心线(蓝)，叠在二值化白色区域上
    {
        static int centerLineSaveCounter = 0;
        ++centerLineSaveCounter;
        cv::Mat vis;
        cv::cvtColor(binary, vis, cv::COLOR_GRAY2BGR);
        for (const auto& pt : centerLinePoints) {
            cv::circle(vis, cv::Point(cvRound(pt.x), cvRound(pt.y)), 1, cv::Scalar(0, 255, 0), -1);
        }
        cv::Point2f p1(centerLine[2] - 1000 * centerLine[0], centerLine[3] - 1000 * centerLine[1]);
        cv::Point2f p2(centerLine[2] + 1000 * centerLine[0], centerLine[3] + 1000 * centerLine[1]);
        cv::line(vis, p1, p2, cv::Scalar(255, 0, 0), 2);
        static std::string visDir = "E:/work/Car_door_ring_splicing/image/背面打光/260716/";
        cv::imwrite(visDir + "collision_centerLine_" + std::to_string(centerLineSaveCounter) + ".bmp", vis);
    }

    return centerLine;
}

std::vector<std::vector<cv::Point2f>> CannyZernikeDetector::buildRightLeftContours(
    const std::vector<std::vector<cv::Point2f>>& edgePointSets, const cv::Vec4f& centerLine,
    const std::vector<cv::Point2f>& subRectCenters, const cv::Mat& inputImage) {
    // 中心线法向，用于把边缘点分到 [右,左]
    float vx = centerLine[0];
    float vy = centerLine[1];
    float nx = -vy;
    float ny = vx;
    float nlen = std::sqrt(nx * nx + ny * ny);
    if (nlen > 1e-6f) {
        nx /= nlen;
        ny /= nlen;
    }
    float x0 = centerLine[2];
    float y0 = centerLine[3];

    std::vector<cv::Point> rightPx, leftPx;
    for (const auto& pts : edgePointSets) {
        for (const auto& p : pts) {
            float dot = (p.x - x0) * nx + (p.y - y0) * ny;
            cv::Point pi(cvRound(p.x), cvRound(p.y));
            if (dot > 0.0f) {
                rightPx.push_back(pi);
            } else {
                leftPx.push_back(pi);
            }
        }
    }
    // 与无碰撞路径一致：Zernike 亚像素化（仅边缘点）。契约 [0]=右, [1]=左
    std::vector<cv::Point2f> right = getSubpixelContourZernike(inputImage, rightPx);
    std::vector<cv::Point2f> left = getSubpixelContourZernike(inputImage, leftPx);

    // spine：两子矩形中心投影到扫描缝隙中线 centerLine 上，沿 centerLine 在两投影点之间
    // 等步长采样（含两端）。spine 是合成点、落在缝隙中线（非边缘），不走 Zernike；
    // 各追加一份到右/左，把长轴两端的两簇边缘桥成完整 C 形开口弧，供下游 SortingStrategy 正确重排。
    const float kCenterLineSampleStep = 5.0f;
    float vlen = std::sqrt(vx * vx + vy * vy);
    float uvx = (vlen > 1e-6f) ? vx / vlen : vx;  // centerLine 方向归一化（lineRansac 方向未必单位长）
    float uvy = (vlen > 1e-6f) ? vy / vlen : vy;
    std::vector<cv::Point2f> spine;
    if (subRectCenters.size() >= 2) {
        const cv::Point2f& c0 = subRectCenters[0];
        const cv::Point2f& c1 = subRectCenters[1];
        // c0/c1 在 centerLine 上的投影参数 t = (c - 线上点)·单位方向
        float t0 = (c0.x - x0) * uvx + (c0.y - y0) * uvy;
        float t1 = (c1.x - x0) * uvx + (c1.y - y0) * uvy;
        float tLo = std::min(t0, t1);
        float tHi = std::max(t0, t1);
        if (tHi - tLo < kCenterLineSampleStep) {
            float tm = (tLo + tHi) * 0.5f;  // 退化：两投影点过近，只加投影中点
            spine.emplace_back(x0 + uvx * tm, y0 + uvy * tm);
        } else {
            int n = static_cast<int>(std::ceil((tHi - tLo) / kCenterLineSampleStep));
            for (int i = 0; i <= n; ++i) {
                float t = tLo + (tHi - tLo) * static_cast<float>(i) / static_cast<float>(n);
                spine.emplace_back(x0 + uvx * t, y0 + uvy * t);
            }
        }
    }
    right.insert(right.end(), spine.begin(), spine.end());
    left.insert(left.end(), spine.begin(), spine.end());
    return {right, left};
}

/**
 * @brief 碰撞情况下的轮廓检测流水线（minAreaRect 长轴扫描法）
 * @details 取白色区域最小外接矩形 → 沿长轴扫描白↔黑跳变并投影到长轴 → 找两个密集区间
 *          → 切成两个子矩形 → 整图一次 Canny 后按子矩形取边缘并拟合直线 → 与长轴中线求交
 *          → 边缘点按中心线法向分成 [右,左] 并亚像素化。输出契约与无碰撞路径一致。
 */
std::vector<std::vector<cv::Point2f>> CannyZernikeDetector::detectContoursWithCollision(
    const cv::Mat& grayImage, const cv::Mat& binaryImage, const cv::Mat& inputImage)
{
    SCOPED_TIMER("碰撞路径轮廓检测");

    // 1-2. 白色区域最小外接矩形 + 长/短轴坐标系
    cv::RotatedRect rect = computeWhiteMinAreaRect(binaryImage);
    // 将最小外接矩形整体缩小到 0.8 倍（中心与角度不变，仅缩放宽高），收束到白色区域核心后再做后续切分
    const float kRectShrinkScale = 0.8f;
    rect.size.width *= kRectShrinkScale;
    rect.size.height *= kRectShrinkScale;
    RectFrame frame = establishRectFrame(rect);

    // 3-4. 沿长轴扫描跳变投影 + 找两个密集区间
    std::vector<float> projections = collectTransitionProjections(binaryImage, frame);
    std::pair<FloatRange, FloatRange> intervals = findTwoDenseIntervals(projections, frame);

    // 5. 切两个子矩形
    std::vector<cv::RotatedRect> subRects = splitMinAreaRect(rect, frame, intervals);

    // [DEBUG] 可视化切分结果：父矩形(黄) + 两个子矩形(红/蓝)，叠在二值化白色区域上
    {
        static int subRectSaveCounter = 0;
        ++subRectSaveCounter;
        cv::Mat subRectVis;
        cv::cvtColor(binaryImage, subRectVis, cv::COLOR_GRAY2BGR);
        auto drawRotatedRect = [](cv::Mat& img, const cv::RotatedRect& r,
                                  const cv::Scalar& color, const std::string& tag) {
            cv::Point2f pts2f[4];
            r.points(pts2f);
            std::vector<cv::Point> poly(4);
            for (int i = 0; i < 4; ++i) {
                poly[i] = cv::Point(cvRound(pts2f[i].x), cvRound(pts2f[i].y));
            }
            cv::polylines(img, poly, true, color, 2);
            cv::putText(img, tag, cv::Point(cvRound(r.center.x), cvRound(r.center.y)),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
        };
        drawRotatedRect(subRectVis, rect, cv::Scalar(0, 255, 255), "parent");    // 黄色：父最小外接矩形
        drawRotatedRect(subRectVis, subRects[0], cv::Scalar(0, 0, 255), "sub0"); // 红色：子矩形0
        drawRotatedRect(subRectVis, subRects[1], cv::Scalar(255, 0, 0), "sub1"); // 蓝色：子矩形1
        static std::string visDir = "E:/work/Car_door_ring_splicing/image/背面打光/260716/";
        cv::imwrite(visDir + "collision_subRects_" + std::to_string(subRectSaveCounter) + ".bmp", subRectVis);
    }

    // 整图一次 Canny + 复用无碰撞路径的两步预过滤
    double TH = adaptiveCannyThresholdByOtsu(grayImage);
    double TL = TH * 0.5;
    cv::Mat edge;
    cv::Canny(grayImage, edge, TL, TH);
    edge = removeIrrelevantEdgeRegions(edge, grayImage);
    edge = filterEdgesByMinAreaRect(edge, binaryImage);

    // 6. 每个子矩形内收集边缘点
    std::vector<std::vector<cv::Point2f>> edgePointSets;
    edgePointSets.reserve(subRects.size());
    for (const auto& subRect : subRects) {
        std::vector<cv::Point2f> pts;
        collectEdgePointsInRect(edge, subRect, pts);
        edgePointSets.push_back(std::move(pts));
    }

    // 7. 中心线：在两子矩形中间区域沿长轴扫描法线方向取缝隙中点 + RANSAC（限定中间区域的扫描法）
    cv::Vec4f centerLine = calculateCenterLineInGap(binaryImage, frame, intervals);

    // 9. 边缘点按中心线法向分成 [右,左] 并亚像素化；中心线 spine 桥接长轴两端成完整 C 形
    std::vector<cv::Point2f> subRectCenters = {subRects[0].center, subRects[1].center};
    auto contours = buildRightLeftContours(edgePointSets, centerLine, subRectCenters, inputImage);

    // [DEBUG] 可视化右(红)/左(蓝)轮廓点集（端部两簇 + 中心线 spine），确认各构成完整 C 形开口弧
    {
        static int contourSaveCounter = 0;
        ++contourSaveCounter;
        cv::Mat contourVis;
        cv::cvtColor(binaryImage, contourVis, cv::COLOR_GRAY2BGR);
        auto drawPts = [](cv::Mat& img, const std::vector<cv::Point2f>& pts, const cv::Scalar& color) {
            for (const auto& p : pts) {
                cv::circle(img, cv::Point(cvRound(p.x), cvRound(p.y)), 1, color, -1);
            }
        };
        if (contours.size() >= 1) {
            drawPts(contourVis, contours[0], cv::Scalar(0, 0, 255));  // 右：红
        }
        if (contours.size() >= 2) {
            drawPts(contourVis, contours[1], cv::Scalar(255, 0, 0));  // 左：蓝
        }
        static std::string visDir = "E:/work/Car_door_ring_splicing/image/背面打光/260716/";
        cv::imwrite(visDir + "collision_contours_" + std::to_string(contourSaveCounter) + ".bmp", contourVis);
    }

    return contours;
}

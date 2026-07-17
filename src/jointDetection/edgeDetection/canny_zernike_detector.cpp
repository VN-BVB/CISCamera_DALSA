#include "canny_zernike_detector.h"

#include <plog/Log.h>

#include <algorithm>
#include <cmath>
#include <queue>
#include <set>
#include <numeric>

#include <plog/Log.h>

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
cv::Point2f CannyZernikeDetector::zernikeSubpixel(const cv::Mat& gray, const cv::Point2f& edgePoint, int radius) {
    // 检查边缘点是否在图像范围内
    if (edgePoint.x < 0 || edgePoint.x >= gray.cols || edgePoint.y < 0 || edgePoint.y >= gray.rows) {
        return edgePoint;
    }

    // 提取边缘点邻域
    cv::Rect roi(cv::Point(std::max(0, int(edgePoint.x - radius)), std::max(0, int(edgePoint.y - radius))), cv::Size(2 * radius + 1, 2 * radius + 1));
    roi &= cv::Rect(0, 0, gray.cols, gray.rows);
    cv::Mat roiImg = gray(roi);

    // 三个矩模板
    cv::Mat M11R =
        (cv::Mat_<double>(7, 7) << 0, -0.0150, -0.0190, 0, 0.0190, 0.0150, 0, -0.0224, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0224, -0.0573, -0.0466,
         -0.0233, 0, 0.0233, 0.0466, 0.0573, -0.0690, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0690, -0.0573, -0.0466, -0.0233, 0, 0.0233, 0.0466,
         0.0573, -0.0224, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0224, 0, -0.0150, -0.0190, 0, 0.0190, 0.0150, 0);

    cv::Mat M11I = (cv::Mat_<double>(7, 7) << 0, -0.0224, -0.0573, -0.0690, -0.0573, -0.0224, 0, -0.0150, -0.0466, -0.0466, -0.0466, -0.0466, -0.0466,
                    -0.0150, -0.0190, -0.0233, -0.0233, -0.0233, -0.0233, -0.0233, -0.0190, 0, 0, 0, 0, 0, 0, 0, 0.0190, 0.0233, 0.0233, 0.0233,
                    0.0233, 0.0233, 0.0190, 0.0150, 0.0466, 0.0466, 0.0466, 0.0466, 0.0466, 0.0150, 0, 0.0224, 0.0573, 0.0690, 0.0573, 0.0224, 0);

    cv::Mat M20 =
        (cv::Mat_<double>(7, 7) << 0, 0.0224, 0.0394, 0.0396, 0.0394, 0.0224, 0, 0.0224, 0.0272, -0.0128, -0.0261, -0.0128, 0.0272, 0.0224, 0.0394,
         -0.0128, -0.0528, -0.0661, -0.0528, -0.0128, 0.0394, 0.0396, -0.0261, -0.0661, -0.0794, -0.0661, -0.0261, 0.0396, 0.0394, -0.0128, -0.0528,
         -0.0661, -0.0528, -0.0128, 0.0394, 0.0224, 0.0272, -0.0128, -0.0261, -0.0128, 0.0272, 0.0224, 0, 0.0224, 0.0394, 0.0396, 0.0394, 0.0224, 0);

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
    for (const auto& p : contour) {
        // 使用Zernike矩法计算亚像素坐标
        cv::Point2f subpixel = zernikeSubpixel(gray, p, 3);
        subpixelContour.push_back(subpixel);
    }

    // PLOG_INFO << u8"Zernike矩法提取亚像素坐标完成" << std::endl;
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
    cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
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
    cv::imwrite("E:/work/Car_door_ring_splicing/image/背面打光/260714/binary_with_minAreaRect.bmp", binaryWithRect);

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
 * @param debugOutput 输出骨架图像用于可视化
 * @return 中心线直线方程参数（vx, vy, x0, y0）
 */
cv::Vec4f CannyZernikeDetector::calculateCenterLineWithZhangSuen(const cv::Mat& grayImage, cv::Mat& debugOutput) {
    // 提取图像中间部分：以图像中心为中心，裁剪出宽高各1/4的区域（与 calculateCenterLine 保持一致）
    cv::Point2f imgCenter(grayImage.cols / 2.0f, grayImage.rows / 2.0f);
    int roiWidth = grayImage.cols / 4;
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
    debugOutput = det.clone();

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

    static std::string visDir = "E:/work/Car_door_ring_splicing/image/背面打光/260714/";
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
cv::Vec4f CannyZernikeDetector::calculateCenterLine(const cv::Mat& grayImage) {
    SCOPED_TIMER("计算中心线");
    return calculateCenterLineByScanline(grayImage);
}

/**
 * @brief 根据中心线将轮廓分类到两侧（按轮廓重心分类）
 * @param contours 输入轮廓集合
 * @param centerLine 中心线直线方程参数（vx, vy, x0, y0）
 * @return 包含左右两侧轮廓的pair，first为左侧轮廓，second为右侧轮廓
 * @details 该方法通过计算每个轮廓的重心，利用中心线的法向量判断轮廓位置，将轮廓分为左侧和右侧两组
 */
std::pair<std::vector<std::vector<cv::Point>>, std::vector<std::vector<cv::Point>>> CannyZernikeDetector::classifyContoursByCenterLine(
    const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine) {
    std::vector<std::vector<cv::Point>> leftContours;   // 中心线左侧的轮廓
    std::vector<std::vector<cv::Point>> rightContours;  // 中心线右侧的轮廓

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
std::vector<std::vector<cv::Point>> CannyZernikeDetector::classifyContourPointsByCenterLine(const std::vector<std::vector<cv::Point>>& contours,
                                                                                            const cv::Vec4f& centerLine) {
    std::vector<std::vector<cv::Point>> contoursLeftAndRight;
    std::vector<cv::Point> leftContours;   // 中心线左侧的轮廓
    std::vector<cv::Point> rightContours;  // 中心线右侧的轮廓

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
    int componentCount = 0;                                 // 连通域数量

    // 定义邻域方向（4邻域：上下左右；8邻域：加对角线）
    std::vector<cv::Point> dirs;
    if (is8Neighbor) {
        dirs = {cv::Point(-1, -1), cv::Point(-1, 0), cv::Point(-1, 1), cv::Point(0, -1),
                cv::Point(0, 1),   cv::Point(1, -1), cv::Point(1, 0),  cv::Point(1, 1)};
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
                q.push(cv::Point(j, i));      // 注意：OpenCV中Point(x,y)，x=列，y=行
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
    ImageTools imageTools;
    cv::Mat grayImage;
    if (inputImage.channels() > 1) {
        cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = inputImage.clone();
    }
    cv::GaussianBlur(grayImage, grayImage, cv::Size(7, 7), 0, 0);

    cv::Mat binaryImage;
    cv::threshold(grayImage, binaryImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    // cv::imwrite("E:/work/Car_door_ring_splicing/image/背面打光/260714/binaryImage.bmp", binaryImage);

    {
        SCOPED_TIMER("连通域分析");
        // 连通域分析：检查工件是否发生碰撞
        int brightComponentCount = countBrightConnectedComponents(binaryImage, true, 500);
        // 如果亮区连通域数量大于1，说明工件可能发生碰撞
        if (brightComponentCount > 1) {
            PLOG_INFO << "警告：检测到 " << brightComponentCount << " 个亮区连通域，工件可能已发生碰撞！";
        }
    }

    // 边缘检测
    double TH = adaptiveCannyThresholdByOtsu(grayImage);
    double TL = TH * 0.5;
    cv::Mat edge;
    cv::Canny(grayImage, edge, TL, TH);
    // cv::imwrite("E:/work/Car_door_ring_splicing/image/背面打光/260714/edge.bmp", edge);

    // 形态学处理，去除无关区域的边缘
    cv::Mat connectedEdge = removeIrrelevantEdgeRegions(edge, grayImage);
    // cv::imwrite("E:/work/Car_door_ring_splicing/image/背面打光/260714/connectedEdge.bmp", connectedEdge);

    // 二次过滤：Otsu + minAreaRect，只保留落在工件外接旋转矩形内的边缘
    cv::Mat filteredEdge = filterEdgesByMinAreaRect(connectedEdge, binaryImage);
    cv::imwrite("E:/work/Car_door_ring_splicing/image/背面打光/260714/filteredEdge.bmp", filteredEdge);


    // 提取并筛选轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(filteredEdge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    // imageTools.drawColorfulContoursAndSave(grayImage, contours,
    // "E:/work/Car_door_ring_splicing/image/背面打光/260714/allContours.bmp");
    std::vector<std::vector<cv::Point>> filteredContours = imageTools.filterContours(contours);
    // imageTools.drawColorfulContoursAndSave(grayImage, filteredContours,
    // "E:/work/Car_door_ring_splicing/image/背面打光/260714/filterContours.bmp");

    // 计算中间缝隙中心线
    cv::Vec4f centerLine = calculateCenterLine(grayImage);
    imageTools.drawLineAndSave(grayImage, centerLine, "E:/work/Car_door_ring_splicing/image/背面打光/260714/centerLine.bmp");

    // 根据中心线分类轮廓
    auto contoursLeftAndRight = classifyContourPointsByCenterLine(filteredContours, centerLine);
    std::vector<std::vector<cv::Point>> rightOnly = {contoursLeftAndRight[0]};
    // imageTools.drawColorfulContoursAndSave(grayImage, rightOnly, "E:/work/Car_door_ring_splicing/image/背面打光/260714/right_contours.bmp");
    std::vector<std::vector<cv::Point>>  leftOnly= {contoursLeftAndRight[1]};
    // imageTools.drawColorfulContoursAndSave(grayImage, leftOnly, "E:/work/Car_door_ring_splicing/image/背面打光/260714/left_contours.bmp");

    // 亚像素轮廓提取
    std::vector<std::vector<cv::Point2f>> subpixelConturs;
    for (const auto& contour : contoursLeftAndRight) {
        std::vector<cv::Point2f> c = getSubpixelContourZernike(inputImage, contour);
        subpixelConturs.push_back(c);
    }
    // 亚像素轮廓可视化
    std::vector<std::vector<cv::Point>> subpixelContursInt;
    subpixelContursInt.reserve(subpixelConturs.size());
    for (const auto& contour : subpixelConturs) {
        std::vector<cv::Point> intContour;
        intContour.reserve(contour.size());
        for (const auto& p : contour) {
            intContour.emplace_back(cvRound(p.x), cvRound(p.y));
        }
        subpixelContursInt.push_back(std::move(intContour));
    }
    std::vector<std::vector<cv::Point>> subrightOnly = {subpixelContursInt[0]};
    // imageTools.drawColorfulContoursAndSave(grayImage, subrightOnly, "E:/work/Car_door_ring_splicing/image/背面打光/260714/subpixel_contours_right.bmp");
    std::vector<std::vector<cv::Point>> subleftOnly = {subpixelContursInt[1]};
    // imageTools.drawColorfulContoursAndSave(grayImage, subleftOnly, "E:/work/Car_door_ring_splicing/image/背面打光/260714/subpixel_contours_left.bmp");

    PLOG_INFO << "轮廓检测完成";
    return subpixelConturs;
}

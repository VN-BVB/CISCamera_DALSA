#include "edge_detector.h"

EdgeDetector::EdgeDetector() {}


// // 改进Zernike亚像素偏移计算辅助函数
// cv::Point2f EdgeDetector::zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius)
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

// 原始Zernike亚像素偏移计算辅助函数
cv::Point2f EdgeDetector::zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius) {
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

std::vector<cv::Point2f> EdgeDetector::getSubpixelContourZernike(const cv::Mat &src, const std::vector<cv::Point> &contour) {
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

    std::cout << u8"Zernike矩法提取亚像素坐标完成" << std::endl;
    return subpixelContour;
}

// Otsu算法自适应计算Canny阈值
double EdgeDetector::adaptiveCannyThresholdByOtsu(const cv::Mat &srcImage) {
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
                if (K_mag >= mag.at<float>(i - 1, j - 1) && K_mag >= mag.at<float>(i + 1, j + 1)) Non_maxImage.at<float>(i, j) = K_mag;
            }
            // 梯度方向在-45方向
            else if ((g_angle <= 337.5 && g_angle > 292.5) || (g_angle <= 157.5 && g_angle > 112.5)) {
                if (K_mag >= mag.at<float>(i + 1, j - 1) && K_mag >= mag.at<float>(i - 1, j + 1)) Non_maxImage.at<float>(i, j) = K_mag;
            }
        }
    }

    cv::Mat nonMaxImage8U;
    Non_maxImage.convertTo(nonMaxImage8U, CV_8UC1);
    cv::Mat thresholdedImage;
    double TH = cv::threshold(nonMaxImage8U, thresholdedImage, 0, 255, cv::THRESH_OTSU);
    return TH;
}

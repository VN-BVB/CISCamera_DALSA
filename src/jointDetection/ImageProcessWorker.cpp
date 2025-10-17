#include "ImageProcessWorker.h"
#include "contourSegment/contourSegment.h"
#include "imageTools.h"
#include <QDebug>

ImageProcessWorker::ImageProcessWorker(QObject *parent)
    : QObject{parent}
{}

// // 改进Zernike亚像素偏移计算辅助函数
// cv::Point2f ImageProcessWorker::zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius)
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
cv::Point2f ImageProcessWorker::zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius)
{
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
    cv::Mat M11R = (cv::Mat_<double>(7, 7) << 0, -0.0150, -0.0190, 0, 0.0190, 0.0150, 0,
                    -0.0224, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0224,
                    -0.0573, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0573,
                    -0.0690, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0690,
                    -0.0573, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0573,
                    -0.0224, -0.0466, -0.0233, 0, 0.0233, 0.0466, 0.0224,
                    0, -0.0150, -0.0190, 0, 0.0190, 0.0150, 0);

    cv::Mat M11I = (cv::Mat_<double>(7, 7) << 0, -0.0224, -0.0573, -0.0690, -0.0573, -0.0224, 0,
                    -0.0150, -0.0466, -0.0466, -0.0466, -0.0466, -0.0466, -0.0150,
                    -0.0190, -0.0233, -0.0233, -0.0233, -0.0233, -0.0233, -0.0190,
                    0, 0, 0, 0, 0, 0, 0,
                    0.0190, 0.0233, 0.0233, 0.0233, 0.0233, 0.0233, 0.0190,
                    0.0150, 0.0466, 0.0466, 0.0466, 0.0466, 0.0466, 0.0150,
                    0, 0.0224, 0.0573, 0.0690, 0.0573, 0.0224, 0);

    cv::Mat M20 = (cv::Mat_<double>(7, 7) << 0, 0.0224, 0.0394, 0.0396, 0.0394, 0.0224, 0,
                   0.0224, 0.0272, -0.0128, -0.0261, -0.0128, 0.0272, 0.0224,
                   0.0394, -0.0128, -0.0528, -0.0661, -0.0528, -0.0128, 0.0394,
                   0.0396, -0.0261, -0.0661, -0.0794, -0.0661, -0.0261, 0.0396,
                   0.0394, -0.0128, -0.0528, -0.0661, -0.0528, -0.0128, 0.0394,
                   0.0224, 0.0272, -0.0128, -0.0261, -0.0128, 0.0272, 0.0224,
                   0, 0.0224, 0.0394, 0.0396, 0.0394, 0.0224, 0);

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
    if (std::abs(L) < 0.14 && std::abs(K) > 40)
    {
        // 计算亚像素偏移
        float dx = (7.0 / 2.0) * L * std::cos(phi);
        float dy = (7.0 / 2.0) * L * std::sin(phi);
        return cv::Point2f(edgePoint.x + dx, edgePoint.y + dy);
    }

    return edgePoint; // 不满足条件时返回原坐标
}

std::vector<cv::Point2f> ImageProcessWorker::getSubpixelContourZernike(const cv::Mat &src,
                                                   const std::vector<cv::Point> &contour)
{
    cv::Mat gray;
    if (src.channels() > 1)
    {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = src.clone();
    }

    // 高斯平滑预处理
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 1.0);

    std::vector<cv::Point2f> subpixelContour;
    for (const auto &p : contour)
    {
        // 使用Zernike矩法计算亚像素坐标
        cv::Point2f subpixel = zernikeSubpixel(gray, p, 3);
        subpixelContour.push_back(subpixel);
    }

    std::cout << "Zernike矩法提取亚像素坐标完成" << std::endl;
    return subpixelContour;
}

// Otsu算法自适应计算Canny阈值
double ImageProcessWorker::adaptiveCannyThresholdByOtsu(const cv::Mat &srcImage) {
    cv::Mat grayImage;
    if (srcImage.channels() > 1){
        cv::cvtColor(srcImage, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = srcImage.clone();
    }

    cv::Mat gx, gy;
    cv::Mat mag, angle;

    cv::Sobel(grayImage, gx, CV_32F, 1, 0, 3);
    cv::Sobel(grayImage, gy, CV_32F, 0, 1, 3);
    //计算梯度幅值和梯度的方向（角度）
    cv::cartToPolar(gx, gy, mag, angle, true);
    //定义全黑非极大值抑制图像
    cv::Mat Non_maxImage = cv::Mat::zeros(grayImage.size(), CV_32FC1);
    int height = grayImage.rows;
    int width = grayImage.cols;
    //获得非极大值抑制图像
    for (int i = 1; i < height - 1; ++i)
    {
        for (int j = 1; j < width - 1; ++j)
        {
            float g_angle = angle.at<float>(i, j);
            float K_mag = mag.at<float>(i, j);
            //梯度方向在垂直方向
            if ((g_angle <= 112.5 && g_angle > 67.5) || (g_angle <= 292.5 && g_angle > 247.5))
            {
                if (K_mag >= mag.at<float>(i - 1, j) && K_mag >= mag.at<float>(i + 1, j))
                    Non_maxImage.at<float>(i, j) = K_mag;
            }
            //梯度方向在水平方向
            else if (g_angle <= 22.5 || g_angle > 337.5 || (g_angle <= 202.5 && g_angle > 157.5))
            {
                if (K_mag >= mag.at<float>(i, j - 1) && K_mag >= mag.at<float>(i, j + 1))
                    Non_maxImage.at<float>(i, j) = K_mag;
            }
            //梯度方向在+45方向
            else if ((g_angle <= 67.5 && g_angle > 22.5) || (g_angle <= 247.5 && g_angle > 202.5))
            {
                if (K_mag >= mag.at<float>(i - 1, j - 1) && K_mag >= mag.at<float>(i + 1, j + 1))
                    Non_maxImage.at<float>(i, j) = K_mag;
            }
            //梯度方向在-45方向
            else if ((g_angle <= 337.5 && g_angle > 292.5) || (g_angle <= 157.5 && g_angle > 112.5))
            {
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

void ImageProcessWorker::processImage(cv::Mat image) {
    try {

        ImageTools imageTools;
        // cv::Mat croppedImg = image(cv::Rect(16000, 16000, 1900, 1900));
        // cv::imwrite("D:/Cpp_Project/WeldseamMeasurement/tests/image/cropped_img.bmp", croppedImg);
        cv::Mat croppedImg = image; // 直接读裁剪后的图，不用再裁剪

        cv::Mat grayImage;
        if (croppedImg.channels() > 1){
            cv::cvtColor(croppedImg, grayImage, cv::COLOR_BGR2GRAY);
        } else {
            grayImage = croppedImg.clone();
        }
        cv::GaussianBlur(grayImage, grayImage, cv::Size(7, 7), 0, 0);

        // 双阈值处理--根据Otsu算出的阈值确定为高阈值，取高阈值的一半记为低阈值
        double TH = this->adaptiveCannyThresholdByOtsu(croppedImg);
        unsigned  TL = TH * 0.5;

        cv::Mat edge;
        cv::Canny(grayImage, edge, TL, TH);
        cv::imwrite("E:/work/车门门环拼接/image/test/cropped_img_edge.bmp",  edge);

        // 提取轮廓
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(edge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE); // 轮廓近似方法设为保存所有点，也可以选择只保存端点，具体见源码注释
        std::vector<std::vector<cv::Point>> filteredContours;
        filteredContours = imageTools.filterContours(contours);
        // 轮廓点去重
        filteredContours = imageTools.removeDuplicateContourPoints(filteredContours);

        // 判断轮廓开口方向
        ContourSegment cs{filteredContours[0]};
        std::string direction = cs.getOpeningDirection();

        // 轮廓点分类
        std::vector<std::vector<cv::Point>> segments;
        std::vector<cv::Vec4f> lines;
        double threshold =8;
        int maxIterations = 100;
        cs.sequentialRansac3Times(filteredContours[0], segments, lines, threshold, maxIterations);
        imageTools.drawColorfulContoursAndSave(edge, segments, "E:/work/车门门环拼接/image/test/coloredSegmentsImage.bmp");

        // 提取亚像素轮廓,zernike矩法
        for (auto &contour : filteredContours) {
            std::vector<cv::Point2f> subpixelContour = getSubpixelContourZernike(grayImage, contour);
            m_subpixelContours.push_back(subpixelContour);
        }

        // std::vector<std::vector<cv::Point2f>> approxContours(m_subpixelContours.size());
        // double perimeter = cv::arcLength(m_subpixelContours[1], true);
        // double epsilon = perimeter * 0.02;
        // cv::approxPolyDP(m_subpixelContours[1], approxContours[1], epsilon, false);

        // // 根据多边形拟合得到的点将轮廓分为多段
        // std::vector<std::vector<cv::Point2f>> segmentedContours =
        //     segmentContourByApproxPoints(m_subpixelContours[1], approxContours[1]);
        // m_segmentedContours.push_back(segmentedContours);
        // // 对前两段轮廓进行直线拟合并计算交点
        // if (m_segmentedContours.size() > 0 && m_segmentedContours[0].size() >= 2) {
        //     // 获取前两段轮廓
        //     std::vector<cv::Point2f> segment1 = m_segmentedContours[0][0];
        //     std::vector<cv::Point2f> segment2 = m_segmentedContours[0][1];

        //     // 对两段轮廓分别进行直线拟合
        //     cv::Vec4f line1 = fitLineToPoints(segment1);
        //     cv::Vec4f line2 = fitLineToPoints(segment2);

        //     // 计算两条直线的交点
        //     m_intersectionPoint = calculateLineIntersection(line1, line2);

        //     qDebug() << "直线1参数: vx=" << line1[0] << ", vy=" << line1[1]
        //              << ", x0=" << line1[2] << ", y0=" << line1[3];
        //     qDebug() << "直线2参数: vx=" << line2[0] << ", vy=" << line2[1]
        //              << ", x0=" << line2[2] << ", y0=" << line2[3];
        //     qDebug() << "交点坐标: (" << m_intersectionPoint.x << ", " << m_intersectionPoint.y << ")";
        // } else {
        //     qDebug() << "轮廓段数量不足，无法进行直线拟合和交点计算";
        // }
        // @TODO:现在通过肥肠粗暴的手法求出交点，存在一下问题需要解决：
        // 1、find_contours函数会进行来回扫描，或许可能需要重新设计轮廓连接和查找方法
        // 2、多边形拟合只进行线段拟合，没有进行圆或椭圆拟合
        // 3、轮廓分割时，存在会将端点单拎出来形成线段的小bug
        // 4、直线拟合的精度问题，要考虑是不是直接进行样条曲线拟合


        emit imageProcessed(croppedImg, m_subpixelContours, filteredContours);
        // emit imageProcessedCannyDevenay(cropped_img, edgeCurves);
    }
    catch (const cv::Exception& e) {
        emit errorOccurred(QString("处理图像时出错: ") + e.what());
    }
}

// 直线拟合函数
cv::Vec4f ImageProcessWorker::fitLineToPoints(const std::vector<cv::Point2f>& points)
{
    if (points.empty()) {
        return cv::Vec4f(0, 0, 0, 0);
    }

    // 使用OpenCV的fitLine函数进行直线拟合
    cv::Vec4f lineParams;
    cv::fitLine(points, lineParams, cv::DIST_L2, 0, 0.01, 0.01);

    // lineParams格式: [vx, vy, x0, y0]
    // 其中(vx, vy)是单位方向向量，(x0, y0)是直线上的一个点
    return lineParams;
}

// 计算两条直线的交点
cv::Point2f ImageProcessWorker::calculateLineIntersection(const cv::Vec4f& line1, const cv::Vec4f& line2)
{
    // 提取直线参数
    float vx1 = line1[0], vy1 = line1[1], x01 = line1[2], y01 = line1[3];
    float vx2 = line2[0], vy2 = line2[1], x02 = line2[2], y02 = line2[3];

    // 检查两条直线是否平行
    float cross = vx1 * vy2 - vy1 * vx2;
    if (std::abs(cross) < 1e-10) {
        // 直线平行或重合，返回无效点
        qDebug() << "警告：两条直线平行或重合，无法计算交点";
        return cv::Point2f(-1, -1);
    }

    // 使用参数方程求解交点
    // 直线1: (x, y) = (x01, y01) + t1 * (vx1, vy1)
    // 直线2: (x, y) = (x02, y02) + t2 * (vx2, vy2)

    // 解方程组:
    // x01 + t1 * vx1 = x02 + t2 * vx2
    // y01 + t1 * vy1 = y02 + t2 * vy2

    // 整理得:
    // t1 * vx1 - t2 * vx2 = x02 - x01
    // t1 * vy1 - t2 * vy2 = y02 - y01

    float dx = x02 - x01;
    float dy = y02 - y01;

    // 使用克莱姆法则求解t1
    float t1 = (dx * vy2 - dy * vx2) / cross;

    // 计算交点坐标
    float intersectX = x01 + t1 * vx1;
    float intersectY = y01 + t1 * vy1;

    return cv::Point2f(intersectX, intersectY);
}




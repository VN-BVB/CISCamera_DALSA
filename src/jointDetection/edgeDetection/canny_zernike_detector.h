#ifndef CANNY_ZERNIKE_DETECTOR_H
#define CANNY_ZERNIKE_DETECTOR_H

#include "abstract_contour_detector.h"

/**
 * @brief Canny-Zernike轮廓检测器，实现基于Canny和Zernike矩的亚像素边缘检测
 * @details 该类继承自AbstractContourDetector，结合Canny边缘检测和Zernike矩方法
 */
class CannyZernikeDetector : public AbstractContourDetector
{
public:
    CannyZernikeDetector();
    virtual ~CannyZernikeDetector() override = default;

    virtual std::vector<std::vector<cv::Point2f>> detectContours(const cv::Mat& inputImage) override;
    virtual std::string getDescription() const override {return "基于Canny_Zernike矩的亚像素边缘检测算法";}
private:
    // Zernike矩法辅助函数
    cv::Point2f zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius);
    // Zernike矩法获取亚像素点
    std::vector<cv::Point2f> getSubpixelContourZernike(const cv::Mat &src, const std::vector<cv::Point> &contour);
    // 获取Canny自适应阈值
    double adaptiveCannyThresholdByOtsu(const cv::Mat &srcImage);
    // 去除边缘图中无关区域的边缘
    cv::Mat removeIrrelevantEdgeRegions(const cv::Mat& edge, const cv::Mat& grayImage);
    // 用 Otsu + minAreaRect 生成工件外接旋转矩形掩码，对 edge 做像素级二次过滤
    cv::Mat filterEdgesByMinAreaRect(const cv::Mat& edge, const cv::Mat& binary);
    // 计算中间缝隙中心线（中轴变换 + RANSAC）
    cv::Vec4f calculateCenterLine(const cv::Mat& rawGray);
    // 根据中心线将轮廓分类到两侧
    std::pair<std::vector<std::vector<cv::Point>>, std::vector<std::vector<cv::Point>>>
    classifyContoursByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine);
    std::vector<std::vector<cv::Point>>
    classifyContourPointsByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine);
    // 检测亮连通域数量
    int countBrightConnectedComponents(const cv::Mat& binary, bool is8Neighbor = false);
};

#endif // CANNY_ZERNIKE_DETECTOR_H

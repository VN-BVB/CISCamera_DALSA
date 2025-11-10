#ifndef CANNY_ZERNIKE_DETECTOR_H
#define CANNY_ZERNIKE_DETECTOR_H

#include "abstract_contour_detector.h"

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
    // 计算中间缝隙中心线（中轴变换 + RANSAC）
    cv::Vec4f calculateCenterLineBySkeletonAndRANSAC(const cv::Mat& image);
    // 根据中心线将轮廓分类到两侧
    std::pair<std::vector<std::vector<cv::Point>>, std::vector<std::vector<cv::Point>>>
    classifyContoursByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine);
    std::vector<std::vector<cv::Point>>
    classifyContourPointsByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine);
};

#endif // CANNY_ZERNIKE_DETECTOR_H

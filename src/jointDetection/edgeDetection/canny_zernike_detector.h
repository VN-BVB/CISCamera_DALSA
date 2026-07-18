#ifndef CANNY_ZERNIKE_DETECTOR_H
#define CANNY_ZERNIKE_DETECTOR_H

#include "abstract_contour_detector.h"

// 旋转矩形的长/短轴局部坐标系（碰撞路径 minAreaRect 长轴扫描法用）
struct RectFrame {
    cv::Point2f center;     // 矩形中心
    cv::Point2f longDir;    // 长轴单位向量
    cv::Point2f shortDir;   // 短轴单位向量（长轴法向）
    float longLen;          // 长轴方向跨度
    float shortLen;         // 短轴方向跨度
};

// 长轴上的 1D 区间
struct FloatRange {
    float lo;
    float hi;
};

/**
 * @brief Canny-Zernike轮廓检测器，实现基于Canny和Zernike矩的亚像素边缘检测
 * @details 该类继承自AbstractContourDetector，结合Canny边缘检测和Zernike矩方法
 */
class CannyZernikeDetector : public AbstractContourDetector
{
public:
    CannyZernikeDetector();
    virtual ~CannyZernikeDetector() override = default;

    virtual ContourDetectionResult detectContours(const cv::Mat& inputImage) override;
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
    // 计算中间缝隙中心线
    cv::Vec4f calculateCenterLineWithoutCollision(const cv::Mat &rawGray);
    cv::Vec4f calculateCenterLineWithCollision(const cv::Mat &grayImage);
    // Zhang-Suen骨架化方法（用于对比）
    cv::Vec4f calculateCenterLineWithZhangSuen(const cv::Mat &grayImage);
    // 扫描线法计算中心线：minAreaRect + 法线方向扫描取中点 + RANSAC，内部完成可视化保存
    cv::Vec4f calculateCenterLineByScanline(const cv::Mat& grayImage);
    // 根据中心线将轮廓分类到两侧
    std::pair<std::vector<std::vector<cv::Point>>, std::vector<std::vector<cv::Point>>>
    classifyContoursByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine);
    std::vector<std::vector<cv::Point>>
    classifyContourPointsByCenterLine(const std::vector<std::vector<cv::Point>>& contours, const cv::Vec4f& centerLine);
    // 检测亮连通域数量（minArea <= 0 时不过滤）
    int countBrightConnectedComponents(const cv::Mat& binary, bool is8Neighbor = false, int minArea = 0);
    // 正常情况（无碰撞）下的轮廓检测流水线：Canny → 形态学 → 轮廓提取 → 中心线分类 → 亚像素
    ContourDetectionResult detectContoursWithoutCollision(
        const cv::Mat& grayImage, const cv::Mat& binaryImage, const cv::Mat& inputImage);
    // 碰撞情况下的轮廓检测流水线：minAreaRect 长轴扫描法（详见各子步骤函数）
    ContourDetectionResult detectContoursWithCollision(
        const cv::Mat& grayImage, const cv::Mat& binaryImage, const cv::Mat& inputImage);

    // ===== 碰撞路径专用子步骤 =====
    // 取二值图中白色(255)区域的最小外接旋转矩形
    cv::RotatedRect computeWhiteMinAreaRect(const cv::Mat& binary);
    // 由旋转矩形建立长/短轴局部坐标系
    RectFrame establishRectFrame(const cv::RotatedRect& rect);
    // 沿长轴扫描，收集白↔黑跳变点投影到长轴的坐标
    std::vector<float> collectTransitionProjections(const cv::Mat& binary, const RectFrame& frame);
    // 由跳变投影找出长轴上两个最密集的区间
    std::pair<FloatRange, FloatRange> findTwoDenseIntervals(const std::vector<float>& projections,
                                                            const RectFrame& frame);
    // 按两个密集区间把父矩形切成长轴方向上的两个子矩形
    std::vector<cv::RotatedRect> splitMinAreaRect(const cv::RotatedRect& parent, const RectFrame& frame,
                                                  const std::pair<FloatRange, FloatRange>& intervals);
    // 在子矩形内收集整图 Canny 边缘点（outEdgePoints）
    void collectEdgePointsInRect(const cv::Mat& edgeMap, const cv::RotatedRect& subRect,
                                 std::vector<cv::Point2f>& outEdgePoints);
    // 中心线 = 矩形长轴中线
    cv::Vec4f centerLineFromFrame(const RectFrame& frame);
    // 在两子矩形中间区域沿长轴扫描法线方向取缝隙中点 + RANSAC 求中心线（碰撞路径用，退化时退回长轴中线）
    cv::Vec4f calculateCenterLineInGap(const cv::Mat& binary, const RectFrame& frame,
                                       const std::pair<FloatRange, FloatRange>& intervals);
    // 把各子矩形边缘点按中心线法向分成 [右,左] 并做 Zernike 亚像素化；
    // 再把两子矩形中心之间的中心线等步长采样出 spine，各追加一份到右/左，使每条轮廓构成完整 C 形
    std::vector<std::vector<cv::Point2f>> buildRightLeftContours(const std::vector<std::vector<cv::Point2f>>& edgePointSets,
                                                                 const cv::Vec4f& centerLine,
                                                                 const std::vector<cv::Point2f>& subRectCenters,
                                                                 const cv::Mat& inputImage);
};

#endif // CANNY_ZERNIKE_DETECTOR_H

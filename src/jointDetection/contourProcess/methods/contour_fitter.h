#ifndef CONTOUR_FITTER_H
#define CONTOUR_FITTER_H

#include <opencv2/opencv.hpp>
#include "curve_seg.h"
#include "line_seg.h"

/**
 * @brief 轮廓拟合器 - 负责直线和曲线拟合
 */
class ContourFitter {
public:
    static std::map<int, LineSeg> fitLinesToSegments(const std::map<int, std::vector<cv::Point2f>>& segments);

    static std::map<int, CurveSeg> fitCurvesToSegments(const std::map<int, std::vector<cv::Point2f>>& segments);

    static void calculateEndPoints(const std::map<int, CurveSeg>& curveSegments,
                                   const cv::Point2f centroid,
                                   std::vector<cv::Point2f>& endPoints,
                                   std::vector<cv::Vec4f>& lines);

    // 碰撞情况：跳过曲线拟合，对原始轮廓段做顺序RANSAC，选与缝隙中心线夹角最大的拟合直线求交得端点
    static void calculateEndPointsFromCenterLine(const std::map<int, std::vector<cv::Point2f>>& segments,
                                                 std::vector<cv::Point2f>& endPoints,
                                                 std::vector<cv::Vec4f>& lines,
                                                 const cv::Vec4f& centerLine);

    static std::vector<cv::Point2f> calculateEndPoints(const std::map<int, LineSeg>& lineSegments);

private:
    // 顺序 RANSAC 最多拟合 maxLines 条直线（剥洋葱：每次拟合后剔除内点），剩余点不足则提前停止
    static std::vector<cv::Vec4f> ransacFitMaxLines(const std::vector<cv::Point2f>& points,
                                                    int maxLines,
                                                    double threshold = 8.0,
                                                    int maxIterations = 100);
    // 在一段点的 RANSAC 候选直线中，选与 centerLine 夹角最大者（方向点积绝对值最小）
    static cv::Vec4f selectLineWithMaxAngle(const std::vector<cv::Point2f>& points,
                                            const cv::Vec4f& centerLine);
};


#endif // CONTOUR_FITTER_H

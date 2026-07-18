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

    // 碰撞情况：跳过曲线拟合，直接对原始轮廓段做最小二乘拟合直线，与缝隙中心线求交得端点
    static void calculateEndPointsFromCenterLine(const std::map<int, std::vector<cv::Point2f>>& segments,
                                                 std::vector<cv::Point2f>& endPoints,
                                                 std::vector<cv::Vec4f>& lines,
                                                 const cv::Vec4f& centerLine);

    static std::vector<cv::Point2f> calculateEndPoints(const std::map<int, LineSeg>& lineSegments);
};


#endif // CONTOUR_FITTER_H

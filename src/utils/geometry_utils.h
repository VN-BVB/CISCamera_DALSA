#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include <opencv2/core.hpp>

namespace GeometryUtils {

// 计算两个向量的叉积
float crossProduct(const cv::Point2f& a, const cv::Point2f& b);

// 判断点是否在线段上
bool isPointOnSegment(const cv::Point2f& p, const cv::Point2f& a, const cv::Point2f& b);

// 判断两条线段是否相交
bool doSegmentsIntersect(const cv::Point2f& p1, const cv::Point2f& p2,
                         const cv::Point2f& q1, const cv::Point2f& q2);

// 判断点是否在旋转矩形内
bool isPointInRotatedRect(const cv::Point2f& point, const cv::RotatedRect& rotatedRect);

// Ransac直线拟合
void lineRansac(const std::vector<cv::Point2f> &points,
                cv::Vec4f &line,
                std::vector<cv::Point2f> &inlierPoints,
                const double &threshold,
                const int &iterations);

cv::Point2f calculateLineIntersection(const cv::Vec4f& line1, const cv::Vec4f& line2);

bool isPointClockwiseTo(const cv::Point2f& pointA, const cv::Point2f& pointB, const cv::Point2f& referencePoint);
} // namespace GeometryUtils

#endif // GEOMETRY_UTILS_H

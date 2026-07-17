#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>


namespace GeometryUtils {

// 计算两个向量的叉积
float crossProduct(const cv::Point2f& a, const cv::Point2f& b);

// 判断点是否在线段上
bool isPointOnSegment(const cv::Point2f& p, const cv::Point2f& a, const cv::Point2f& b);

// 判断两条线段是否相交
bool doSegmentsIntersect(const cv::Point2f& p1, const cv::Point2f& p2,
                         const cv::Point2f& q1, const cv::Point2f& q2);

// 判断闭合多边形是否自交（假定顶点数 >= 3）
bool isPolygonSelfIntersecting(const std::vector<cv::Point2f>& polygon);

// 判断点是否在旋转矩形内
bool isPointInRotatedRect(const cv::Point2f& point, const cv::RotatedRect& rotatedRect);

// Ransac直线拟合
void lineRansac(const std::vector<cv::Point2f> &points,
                cv::Vec4f &line,
                std::vector<cv::Point2f> &inlierPoints,
                const double &threshold,
                const int &iterations);

// 计算两条直线的交点
cv::Point2f calculateLineIntersection(const cv::Vec4f& line1, const cv::Vec4f& line2);

// 相对于参考点，A点是否在B点的顺时针方向
bool isPointClockwiseTo(const cv::Point2f& pointA, const cv::Point2f& pointB, const cv::Point2f& referencePoint);

// 最小二乘法拟合直线
cv::Vec4f fitLine(const std::vector<cv::Point> &points);
cv::Vec4f fitLine(const std::vector<cv::Point2f> &points);

// 将像素坐标转成世界坐标
std::vector<Eigen::Vector2d> pixel2World(const std::vector<cv::Point2f>& pix_pts);

} // namespace GeometryUtils

#endif // GEOMETRY_UTILS_H

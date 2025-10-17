#ifndef CONTOURCURVE_H
#define CONTOURCURVE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

/**
 * @brief 开口方向枚举
 */
enum class OpeningDirection : int { // enum class:CXX11的强类型枚举，更安全，使用时必须显示指定作用域
    UNKNOWN = 0,    // 未知方向
    UP = 1,         // 向上开口
    DOWN = 2,       // 向下开口
    LEFT = 3,       // 向左开口
    RIGHT = 4       // 向右开口
};

/**
 * @brief 线段特征结构体，存储一条线段的所有特征
 */
class Line
{
public:
    Line();
    void initializeFromPoints(const std::vector<cv::Point>& points);
    void calculateBasicFeatures();
    void clear();
    bool isValid() const;
    std::string getSummary()const;

private:
    // 线段上的点集
    std::vector<cv::Point> pixelPoints;           // 像素级点集
    std::vector<cv::Point2f> subpixelPoints;      // 亚像素级点集

    // 直线拟合相关特征
    cv::Vec4f lineEquation;                      // 直线方程 (vx, vy, x0, y0)
    std::vector<cv::Point> inlierPoints;         // 直线拟合的内点
    double fitError;                             // 拟合误差

    // 几何特征
    cv::Point startPoint;                        // 线段起点
    cv::Point endPoint;                          // 线段终点
    double length;                               // 线段长度
    double angle;                                // 线段角度（弧度）
    cv::Point midpoint;                          // 线段中点
};

/**
 * @brief 轮廓曲线结构体，存储一条轮廓的所有特征
 */
 class ContourCurve
{
 public:
    ContourCurve();
    void initializeFromContour(const std::vector<cv::Point>& contour);
    void calculateBasicFeatures();
    void clear();
    bool isValid() const;
    std::string getSummary()const;
    std::string openingDirectionToString(OpeningDirection direction)const;

 private:
    // 基本轮廓信息
    std::vector<cv::Point> pixelContour;           // 轮廓像素级点集
    std::vector<cv::Point2f> subpixelContour;  // 亚像素级点集
    OpeningDirection openingDirection;                     // 开口方向

    // 轮廓分割相关特征
    std::vector<std::vector<cv::Point>> segmentedPixelContours;  // 分割后的轮廓段-像素级
    std::vector<std::vector<cv::Point2f>> segmentedSubpixelContours; // 分割后的轮廓段-亚像素级
    std::vector<cv::Point2f> cornerPoints;            // 角点位置

    // 直线拟合相关特征
    std::vector<cv::Vec4f> fittedLines;               // 拟合的直线方程 (vx, vy, x0, y0)
    std::vector<std::vector<cv::Point>> lineInliers; // 每条直线对应的内点
    std::vector<double> lineFitErrors;               // 直线拟合误差

    // 几何特征
    cv::Rect boundingRect;                           // 轮廓外接矩形
    double area;                                     // 轮廓面积
    double perimeter;                                // 轮廓周长
    double aspectRatio;                              // 长宽比
    cv::Point centroid;                              // 轮廓质心

    // 多边形逼近特征
    std::vector<cv::Point> approxPolygon;            // 多边形逼近结果
    double approxError;                              // 逼近误差
};

#endif // CONTOURCURVE_H

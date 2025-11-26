#ifndef CURVE_SEG_H
#define CURVE_SEG_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include "tinysplinecxx.h"

const float MIN_DOMAIN = 0.005f;     // 样条曲线参数U的最小值
const float MAX_DOMAIN = 0.995f;     // 样条曲线参数U的最大值

/**
 * @brief 端点信息结构体，包含点的坐标和对应的u值
 */
struct EndpointInfo {
    cv::Point2f point;  // 端点坐标
    float u;            // 对应的参数u值

    EndpointInfo() : point(0, 0), u(0.0f) {}
    EndpointInfo(const cv::Point2f& p, float u_val) : point(p), u(u_val) {}
};

/**
 * @brief 曲线，存储一条线段的所有特征
 */
class CurveSeg
{
public:
    CurveSeg();
    void initializeFromPoints(const std::vector<cv::Point2f>& points);
    void fitSplineCurve();
    tinyspline::BSpline getSpline() const {return m_spline;}

    // 获取拟合后的样条曲线点
    std::vector<cv::Point2f> getFittedPoints(int numSamples = 100) const;

    // 获取控制点
    std::vector<cv::Point2f> getControlPoints() const;

    // 评估样条曲线在参数u处的点
    cv::Point2f evaluate(float u) const;

    // 获取样条曲线的导数（切线）
    cv::Vec4f getTangent(float u) const;

    // 绘制曲线到图像
    void drawCurve(cv::Mat &image, const cv::Scalar &color = cv::Scalar(0, 255, 0),
                   int thickness = 2) const;

    // 绘制控制点和控制多边形
    void drawControlPoints(cv::Mat &image, const cv::Scalar &pointColor = cv::Scalar(0, 0, 255),
                           const cv::Scalar &polygonColor = cv::Scalar(255, 0, 0)) const;

    std::pair<cv::Point2f, cv::Point2f> sortPointsCounterClockwise(const cv::Point2f& point1,
                                                                   const cv::Point2f& point2,
                                                                   const cv::Point2f& referencePoint);
    // 按照参考点的逆时针方向排序其两端点
    std::pair<EndpointInfo, EndpointInfo> sortEndpoints(const cv::Point2f& referencePoint);
    // 获取端点附近区域的平均直线
    cv::Vec4f getAverageLineNearEndpoint(float endpointU, float regionSize = 0.1f, int numSamples = 10) const;


    // 获取曲线长度
    float getCurveLength() const { return m_curveLength; }

    // 计算曲线长度
    void calculateCurveLength();

private:
    std::vector<cv::Point2f> m_points;
    tinyspline::BSpline m_spline;               // 拟合的样条曲线
    bool m_isFitted;                            // 是否已经拟合
    std::vector<cv::Point2f> m_controlPoints;   // 控制点（用于显示）
    float m_minDomain;                          // 样条曲线参数U的最小值
    float m_maxDomain;                          // 样条曲线参数U的最大值
    float m_curveLength;                        // 曲线长度
};

#endif // CURVE_SEG_H

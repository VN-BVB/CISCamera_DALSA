#ifndef CURVE_SEG_H
#define CURVE_SEG_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "tinysplinecxx.h"
/**
 * @brief 曲线，存储一条线段的所有特征
 */
class CurveSeg
{
public:
    CurveSeg();
    void initializeFromPoints(const std::vector<cv::Point2f>& points);

    void fitSplineCurve();

    // 获取拟合后的样条曲线点
    std::vector<cv::Point2f> getFittedPoints(int numSamples = 100) const;

    // 获取控制点
    std::vector<cv::Point2f> getControlPoints() const;

    // 评估样条曲线在参数u处的点
    cv::Point2f evaluate(float u) const;

    // 获取样条曲线的导数（切线）
    cv::Point2f getTangent(float u) const;

    // 绘制曲线到图像
    void drawCurve(cv::Mat &image, const cv::Scalar &color = cv::Scalar(0, 255, 0),
                   int thickness = 2) const;

    // 绘制控制点和控制多边形
    void drawControlPoints(cv::Mat &image, const cv::Scalar &pointColor = cv::Scalar(0, 0, 255),
                           const cv::Scalar &polygonColor = cv::Scalar(255, 0, 0)) const;

private:
    std::vector<cv::Point2f> m_points;
    tinyspline::BSpline m_spline;             // 拟合的样条曲线
    bool m_isFitted;                          // 是否已经拟合
    std::vector<cv::Point2f> m_controlPoints; // 控制点（用于显示）
};

#endif // CURVE_SEG_H

#include "src/jointDetection/contourProcess/curve_seg.h"
#include <opencv2/opencv.hpp>
#include <iostream>

void testCurveSeg(const std::vector<cv::Point2f> points) {
    // 创建测试点
    std::vector<cv::Point2f> testPoints;
    // testPoints.push_back(cv::Point2f(100, 100));
    // testPoints.push_back(cv::Point2f(150, 200));
    // testPoints.push_back(cv::Point2f(200, 150));
    // testPoints.push_back(cv::Point2f(250, 250));
    // testPoints.push_back(cv::Point2f(300, 180));
    // testPoints.push_back(cv::Point2f(350, 220));
    // testPoints.push_back(cv::Point2f(400, 120));
    testPoints = points;

    // 创建曲线对象
    CurveSeg curve;
    curve.initializeFromPoints(testPoints);

    // 拟合样条曲线
    curve.fitSplineCurve();

    // 创建图像
    cv::Mat image = cv::Mat::zeros(2000, 3800, CV_8UC3);
    image.setTo(cv::Scalar(255, 255, 255));

    // // 绘制原始点（蓝色）
    // for (const auto& pt : testPoints) {
    //     cv::circle(image, cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)),
    //                3, cv::Scalar(255, 0, 0), -1);
    // }

    // 绘制控制点和控制多边形（红色）
    // curve.drawControlPoints(image);

    // 绘制拟合的样条曲线（绿色）
    curve.drawCurve(image);

    // 显示图像
    cv::imshow("样条曲线拟合结果", image);
    cv::waitKey(0);
    cv::imwrite("spline_fitting_result.png", image);

    // 测试评估功能
    std::cout << "样条曲线评估测试:" << std::endl;
    for (float u = 0.1f; u <= 0.9f; u += 0.2f) {
        cv::Point2f point = curve.evaluate(u);
        cv::Point2f tangent = curve.getTangent(u);
        std::cout << "u = " << u << ": 点 = (" << point.x << ", " << point.y
                  << "), 切线 = (" << tangent.x << ", " << tangent.y << ")" << std::endl;
    }
}

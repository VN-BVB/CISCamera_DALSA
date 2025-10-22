#include "test_tiny_spline.h"


TinySplineqqq::TinySplineqqq() {}

void TinySplineqqq::run()
{
    // Create a cubic spline with 7 control points in 2D using
    // a clamped knot vector. This call is equivalent to:
    // tinyspline::BSpline spline(7, 2, 3, TS_CLAMPED);
    tinyspline::BSpline spline(7);

    // Setup control points.
    std::vector<tinyspline::real> ctrlp = spline.controlPoints();
    ctrlp[0]  = (tsReal) -1.75; // x0
    ctrlp[1]  = (tsReal) -1.0;  // y0
    ctrlp[2]  = (tsReal) -1.5;  // x1
    ctrlp[3]  = (tsReal) -0.5;  // y1
    ctrlp[4]  = (tsReal) -1.5;  // x2
    ctrlp[5]  = (tsReal)  0.0;  // y2
    ctrlp[6]  = (tsReal) -1.25; // x3
    ctrlp[7]  = (tsReal)  0.5;  // y3
    ctrlp[8]  = (tsReal) -0.75; // x4
    ctrlp[9]  = (tsReal)  0.75; // y4
    ctrlp[10] = (tsReal)  0.0;  // x5
    ctrlp[11] = (tsReal)  0.5;  // y5
    ctrlp[12] = (tsReal)  0.5;  // x6
    ctrlp[13] = (tsReal)  0.0;  // y6
    spline.setControlPoints(ctrlp);

    // Evaluate `spline` at u = 0.4 using 'eval'.
    std::vector<tinyspline::real> result = spline.eval(0.4f).result();
    std::cout << "x = " << result[0] << ", y = " << result[1] << std::endl;

    // Derive `spline` and subdivide it into a sequence of Bezier curves.
    tinyspline::BSpline beziers = spline.derive().toBeziers();

    // Evaluate `beziers` at u = 0.3 using '()' instead of 'eval'.
    result = beziers((tsReal) 0.3).result();
    std::cout << "x = " << result[0] << ", y = " << result[1] << std::endl;
}

void TinySplineqqq::runcv()
{
    // 创建OpenCV Point2f格式的控制点
    std::vector<cv::Point2f> controlPoints;
    controlPoints.push_back(cv::Point2f(100, 100));
    controlPoints.push_back(cv::Point2f(150, 200));
    controlPoints.push_back(cv::Point2f(200, 150));
    controlPoints.push_back(cv::Point2f(250, 250));
    controlPoints.push_back(cv::Point2f(300, 180));
    controlPoints.push_back(cv::Point2f(350, 220));
    controlPoints.push_back(cv::Point2f(400, 120));

    std::cout << "原始控制点 (OpenCV Point2f格式):" << std::endl;
    for (size_t i = 0; i < controlPoints.size(); ++i) {
        std::cout << "点" << i << ": (" << controlPoints[i].x << ", " << controlPoints[i].y << ")" << std::endl;
    }
    std::cout << std::endl;

    // 创建tinyspline样条曲线 (7个控制点，2维，3次样条)
    tinyspline::BSpline spline(7, 2, 3);

    // 将OpenCV Point2f点转换为tinyspline控制点格式
    std::vector<tinyspline::real> ctrlp = spline.controlPoints();
    for (size_t i = 0; i < controlPoints.size(); ++i) {
        ctrlp[i * 2] = controlPoints[i].x;     // x坐标
        ctrlp[i * 2 + 1] = controlPoints[i].y; // y坐标
    }
    spline.setControlPoints(ctrlp);

    // 创建图像用于绘制
    int image_width = 800;
    int image_height = 600;
    cv::Mat image = cv::Mat::zeros(image_height, image_width, CV_8UC3);
    image.setTo(cv::Scalar(255, 255, 255)); // 白色背景

    // 坐标变换：将数学坐标转换为图像坐标
    // 控制点范围大约为 [100, 400] x [100, 250]
    // 图像坐标范围 [0, image_width-1] x [0, image_height-1]
    auto mathToImage = [&](float x, float y) -> cv::Point {
        // 计算控制点的实际范围
        float min_x = 100, max_x = 400;
        float min_y = 100, max_y = 250;

        // 添加一些边距
        float margin_x = 50;
        float margin_y = 50;

        // 转换为图像坐标
        int img_x = static_cast<int>((x - min_x) / (max_x - min_x) * (image_width - 2 * margin_x) + margin_x);
        int img_y = static_cast<int>((max_y - y) / (max_y - min_y) * (image_height - 2 * margin_y) + margin_y);

        return cv::Point(img_x, img_y);
    };

    // 绘制坐标轴
    cv::Point origin = mathToImage(0, 0);
    cv::line(image, cv::Point(50, origin.y), cv::Point(image_width-50, origin.y), cv::Scalar(200, 200, 200), 1);
    cv::line(image, cv::Point(origin.x, 50), cv::Point(origin.x, image_height-50), cv::Scalar(200, 200, 200), 1);

    // 绘制控制点（红色）
    std::vector<cv::Point> imageControlPoints;
    for (const auto& pt : controlPoints) {
        cv::Point img_pt = mathToImage(pt.x, pt.y);
        imageControlPoints.push_back(img_pt);
        cv::circle(image, img_pt, 5, cv::Scalar(0, 0, 255), -1); // 红色实心圆
        cv::circle(image, img_pt, 5, cv::Scalar(0, 0, 0), 2);    // 黑色边框
    }

    // 绘制控制多边形（蓝色虚线）
    for (size_t i = 0; i < imageControlPoints.size() - 1; ++i) {
        cv::line(image, imageControlPoints[i], imageControlPoints[i+1],
                 cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
    }

    // 评估样条曲线并绘制（绿色实线）
    std::vector<cv::Point> splinePoints;
    int num_samples = 100;
    for (int i = 0; i <= num_samples; ++i) {
        float u = static_cast<float>(i) / num_samples;
        std::vector<tinyspline::real> result = spline.eval(u).result();
        cv::Point img_pt = mathToImage(result[0], result[1]);
        splinePoints.push_back(img_pt);
    }

    // 绘制样条曲线
    for (size_t i = 0; i < splinePoints.size() - 1; ++i) {
        cv::line(image, splinePoints[i], splinePoints[i+1],
                 cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
    }

    // 计算并绘制切线（紫色）
    tinyspline::BSpline derivative = spline.derive();
    for (float u = 0.0f; u <= 1.0f; u += 0.2f) {
        std::vector<tinyspline::real> point = spline.eval(u).result();
        std::vector<tinyspline::real> tangent = derivative.eval(u).result();

        cv::Point img_point = mathToImage(point[0], point[1]);
        cv::Point img_tangent_end = mathToImage(
            point[0] + tangent[0] * 20.0f,  // 放大切线长度以便显示
            point[1] + tangent[1] * 20.0f
            );

        cv::arrowedLine(image, img_point, img_tangent_end,
                        cv::Scalar(255, 0, 255), 2, cv::LINE_AA);
    }

    // 添加图例和标签
    cv::putText(image, "控制点 (红色)", cv::Point(20, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
    cv::putText(image, "控制多边形 (蓝色)", cv::Point(20, 60),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 0, 0), 2);
    cv::putText(image, "样条曲线 (绿色)", cv::Point(20, 90),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    cv::putText(image, "切线 (紫色)", cv::Point(20, 120),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 0, 255), 2);

    // 显示图像
    cv::imshow("样条曲线拟合结果", image);
    cv::waitKey(0); // 等待按键

    // 保存图像
    cv::imwrite("spline_fitting_result.png", image);
    std::cout << "图像已保存为 spline_fitting_result.png" << std::endl;

    // 控制台输出评估结果
    std::cout << "样条曲线评估结果:" << std::endl;
    for (float u = 0.0f; u <= 1.0f; u += 0.1f) {
        std::vector<tinyspline::real> result = spline.eval(u).result();
        std::cout << "u = " << u << ": x = " << result[0] << ", y = " << result[1] << std::endl;
    }
    std::cout << std::endl;

    // 将样条曲线转换为贝塞尔曲线序列
    tinyspline::BSpline beziers = spline.toBeziers();

    // 评估贝塞尔曲线
    std::cout << "贝塞尔曲线评估结果:" << std::endl;
    for (float u = 0.0f; u <= 1.0f; u += 0.1f) {
        std::vector<tinyspline::real> result = beziers(u).result();
        std::cout << "u = " << u << ": x = " << result[0] << ", y = " << result[1] << std::endl;
    }
    std::cout << std::endl;

    // 计算样条曲线的导数（切线向量）
    std::cout << "样条曲线导数（切线向量）:" << std::endl;
    for (float u = 0.0f; u <= 1.0f; u += 0.2f) {
        std::vector<tinyspline::real> tangent = derivative.eval(u).result();
        std::cout << "u = " << u << ": 切线 = (" << tangent[0] << ", " << tangent[1] << ")" << std::endl;
    }
}

#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>

#include <QApplication>
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include "src/crashHandler/CrashHandler.h"
#include "src/test/test_tiny_spline.h"
#include "src/ui/CISCamera_imageGrab/cis_camera_image.h"

void initPlog();  // 初始化日志类
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CrashHandler::Init(L"data/debug");  // 初始化Mini转储
    initPlog();                         // 初始化日志类
    CISWidget w;
    w.show();

    return a.exec();
}
// 初始化日志类
void initPlog() {
    // 日志信息分类等级: none = 0, fatal = 1, error = 2, warning = 3, info = 4, debug = 5, verbose = 6
    // 设置初始化等级后, 等级『数值大于』设置值的日志信息就会被『忽略』
    plog::init(plog::debug, "./data/log/log.csv", 1000000000, 100);
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::get()->addAppender(&consoleAppender);  // Also add logging to the console.
}
// #include <Eigen/Dense>
// #include <cmath>
// #include <iostream>
// #include <opencv2/opencv.hpp>
// #include <vector>

// #include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"

// // -------------------- 主函数测试 --------------------
// int main() {
//     CalibrationData calibe;
//     if (!calibe.load("./data/calibration_config/optimized_calib_data.json")) {
//         throw std::runtime_error("无法加载标定文件");
//     }

//     // 从标定数据中读取参数
//     Eigen::Matrix3d K = calibe.K;
//     Eigen::Matrix<double, 1, 5> D = calibe.coff_dis;
//     double m = calibe.m;

//     // 理想归一化坐标（齐次坐标）
//     Eigen::MatrixXd ideal(3, 3);
//     ideal << 650.1, 400.1, 1.0, 12.05, -23.05, 1.0, 3.0, 3.0, 1.0;

//     TelecentricLineCalibrator calib;

//     // 正向畸变
//     Eigen::MatrixXd distorted = calib.distort(D, ideal);

//     // 转为像素坐标
//     Eigen::MatrixXd points_px = (K * distorted.transpose()).transpose();
//     points_px = points_px.leftCols(2);  // 提取 u, v

//     // 去畸变
//     Eigen::MatrixXd undistorted_px = calib.undistortPointsIter(points_px, K, D);

//     // 输出结果
//     std::cout << "原始像素:\n" << (K * ideal.transpose()).transpose().leftCols(2) << std::endl;
//     std::cout << "畸变后:\n" << points_px << std::endl;
//     std::cout << "去畸变后:\n" << undistorted_px << std::endl;

//     // 归一化坐标
//     Eigen::MatrixXd normalized_2d = calib.pixelToCameraCoordinates(points_px, K, D);
//     Eigen::MatrixXd original_2d = ideal.leftCols(2);

//     std::cout << "\n去畸变后归一化坐标:\n" << normalized_2d << std::endl;

//     // 误差计算
//     Eigen::VectorXd errors(ideal.rows());
//     for (int i = 0; i < ideal.rows(); i++) {
//         errors(i) = std::sqrt(std::pow(normalized_2d(i, 0) - original_2d(i, 0), 2) +
//                               std::pow(normalized_2d(i, 1) - original_2d(i, 1), 2));
//     }

//     std::cout << "\n每个点误差:\n" << errors.transpose() << std::endl;
//     std::cout << "平均误差: " << errors.mean() << std::endl;

//     return 0;
// }

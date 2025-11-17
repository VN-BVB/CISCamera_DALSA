// #include <plog/Init.h>
// #include <plog/Initializers/ConsoleInitializer.h>
// #include <plog/Initializers/RollingFileInitializer.h>
// #include <plog/Log.h>

// #include <QApplication>
// // clang-format off
// #include <winsock2.h>
// #include <windows.h>
// // clang-format on
// #include "src/crashHandler/CrashHandler.h"
// #include "src/test/test_tiny_spline.h"
// #include "src/ui/CISCamera_imageGrab/cis_camera_image.h"

// void initPlog();  // 初始化日志类
// int main(int argc, char *argv[]) {
//     QApplication a(argc, argv);
//     CrashHandler::Init(L"data/debug");  // 初始化Mini转储
//     initPlog();                         // 初始化日志类
//     CISWidget w;
//     w.show();

//     return a.exec();
// }
// // 初始化日志类
// void initPlog() {
//     // 日志信息分类等级: none = 0, fatal = 1, error = 2, warning = 3, info = 4, debug = 5, verbose = 6
//     // 设置初始化等级后, 等级『数值大于』设置值的日志信息就会被『忽略』
//     plog::init(plog::debug, "./data/log/log.csv", 1000000000, 100);
//     static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
//     plog::get()->addAppender(&consoleAppender);  // Also add logging to the console.
// }
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "src/telecentricLineCalibrator/py_telecentric_optimizer.h"
#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"
#include "src/ui/CISCamera_imageGrab/cameraImage_processor.h"
int main() {
    // ------------------ 加载标定 ------------------
    CalibrationData calib, calib2;
    CameraImageProcessor reader;
    TelecentricPYOptimizer calib3;

    if (!calib.load("./data/calibration_config/optimized_calib_data.json")) {
        std::cerr << "无法加载 optimized_calib_data.json\n";
        return -1;
    }
    if (!calib2.load("./data/calibration_config/before_optimization_calib_data.json")) {
        std::cerr << "无法加载 before_optimization_calib_data.json\n";
        return -1;
    }

    Eigen::Matrix3d K = calib.K;
    Eigen::Matrix<double, 1, 5> coff_dis = calib.coff_dis;

    Eigen::Vector3d v_rot = calib2.v_rot[3];
    Eigen::Vector3d v_trans = calib2.v_trans[3];

    // ------------------ 读取像素点 ------------------
    std::vector<Eigen::Vector2d> imgPts;
    if (!reader.readPointsFromTxt("./data/CISCamera_Image/test/Board1_Points_20251111_170637.txt", imgPts)) {
        std::cerr << "读取像素点失败\n";
        return -1;
    }

    // ------------------ 构造世界点 ------------------
    const int W = 8, H = 11;
    const double spacingMM = 10.0;

    bool a = calib3.optTelecentricExtrinsicParameters(K, coff_dis, imgPts, v_rot, v_trans, W, H, spacingMM);
    return 0;
}

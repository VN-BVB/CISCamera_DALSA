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
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <direct.h>

#include <Eigen/Dense>
#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QObject>
#include <QString>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>

#include "src/telecentricLineCalibrator/telecentricplatform_calib.h"
int main() {
    Eigen::Matrix3d K;
    K << 47.283237490301396, -0.657929607742621, 15551.964431991371, 0.0, 47.05230788272559, 8043.186819107249, 0.0, 0.0, 1.0;

    Eigen::Matrix<double, 1, 5> coff_dis;
    coff_dis << -5.363602460785097e-10, -6.586873205793823e-07, -3.9297031624526706e-07, 2.075287196873092e-06,
        -2.0074419972225162e-06;

    Eigen::Vector3d v_rot(2.074776703520814, 2.0584407379860554, -0.21906585124567024);
    Eigen::Vector3d v_trans(-249.01625128531074, -134.99191717289557, 0.0);
    TelecentricPlatformCalib calib(K, coff_dis, v_rot, v_trans);
    calib.run();
    return 0;
}

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
#include "src/utils/plog_utils.h"
#include "src/ui/CISCamera_imageGrab/cis_camera_image.h"
#include "src/test/test_tiny_spline.h"
#include "src/ui/jointView/joint_view.h"
#include "src/test/test_frmVisionDisplay.h"
#include "src/test/test_multiRoi.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CrashHandler::Init(L"data/debug");  // 初始化Mini转储
    PlogUtils::initPlog();                           // 初始化日志类
    // CISWidget w;
    // w.show();

    test_multiRoi t;
    t.show();

    // JointView w;
    // w.show();

    return a.exec();
}
// #ifndef M_PI
// #define M_PI 3.14159265358979323846
// #endif
// #include <direct.h>

// #include <Eigen/Dense>
// #include <QDateTime>
// #include <QDir>
// #include <QMutex>
// #include <QObject>
// #include <QString>
// #include <filesystem>
// #include <fstream>
// #include <iostream>
// #include <memory>
// #include <opencv2/opencv.hpp>
// #include <vector>

// #include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"
// int main() {
//     CalibrationData calibRead;
//     TelecentricLineCalibrator calib;

//     if (!calibRead.load("./data/calibration_config/optimized_calib_data.json")) {
//         throw std::runtime_error("无法加载标定文件");
//     }

//     // 从标定数据中读取参数
//     Eigen::Matrix3d K = calibRead.K;
//     Eigen::Matrix<double, 1, 5> coff_dis = calibRead.coff_dis;
//     double m = calibRead.m;
//     std::vector<Eigen::Vector3d> v_rot1 = calibRead.v_rot;
//     std::vector<Eigen::Vector3d> v_trans1 = calibRead.v_trans;
//     Eigen::Vector3d v_rot = v_rot1[1];
//     Eigen::Vector3d v_trans = v_trans1[1];
//     // 理想归一化坐标（齐次坐标）
//     Eigen::MatrixXd pixelPts(2, 2);
//     pixelPts << 4287.350586, 5683.791992, 4758.567871, 5688.670898;

//     // -------------------- 原始世界平面点 (示例) --------------------
//     // -------------------- 像素 -> 相机（去畸变+K^-1） --------------------
//     Eigen::MatrixXd cam_back = calib.pixelToCameraCoordinates(pixelPts, K, coff_dis);
//     std::cout << "\n像素 -> 相机(去畸变归一化):\n" << cam_back << "\n";

//     // -------------------- 相机 -> 世界（平面逆） --------------------
//     Eigen::MatrixXd world_back = calib.cameraToWorldCoordinates(cam_back, v_rot, v_trans);
//     std::cout << "\n像素 -> 世界(反算):\n" << world_back << "\n";

//     // -------------------- 误差评估 --------------------
//     return 0;
// }

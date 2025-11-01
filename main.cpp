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
// #include "src/ui/CISCamera_imageGrab/cis_camera_image.h"
// #include "src/ui/ImageViewWindow.h"

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
#include <Eigen/Dense>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"

namespace fs = std::filesystem;

// 读取一个 txt 文件中的点
bool readPointsFromTxt(const std::string& path, std::vector<Eigen::Vector2d>& pts) {
    std::ifstream fin(path);
    if (!fin.is_open()) {
        std::cerr << "无法打开文件: " << path << std::endl;
        return false;
    }

    std::string line;
    pts.clear();
    // 跳过第一行标题
    std::getline(fin, line);

    double idx, x, y;
    while (fin >> idx >> x >> y) {
        pts.emplace_back(x, y);
    }

    if (pts.empty()) {
        std::cerr << "文件 " << path << " 无有效点。" << std::endl;
        return false;
    }
    std::cout << "读取 " << path << " 成功，共 " << pts.size() << " 个点\n";
    return true;
}

// 主程序
int main() {
    std::string folder = "./data/CISCamera_Image/txt";
    std::vector<std::vector<Eigen::Vector2d>> all_image_points;

    // 遍历文件夹读取所有txt
    for (auto& entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".txt") {
            std::vector<Eigen::Vector2d> pts;
            if (readPointsFromTxt(entry.path().string(), pts)) {
                all_image_points.push_back(pts);
            }
        }
    }

    if (all_image_points.empty()) {
        std::cerr << "没有读取到任何标定点文件！" << std::endl;
        return -1;
    }

    // 构造世界坐标系下圆心点
    const int W = 8, H = 11;
    const double spacingMM = 10.0;
    std::vector<Eigen::Vector2d> worldPts;
    worldPts.reserve(W * H);
    for (int r = 0; r < H; ++r)
        for (int c = 0; c < W; ++c) worldPts.emplace_back(c * spacingMM, r * spacingMM);

    // 图像参数
    const double dx = 25.4 / 1200.0;  // mm/pixel (1200 dpi)
    const double dy = dx;             // 正方像素
    const int width = 30688, height = 16100;

    // 输出结果
    Eigen::Matrix3d K;
    double rmse;
    std::vector<Pose> poses;

    TelecentricLineCalibrator calib;
    calib.calibrateCameraFromPointsDemo(all_image_points, worldPts, width, height, dx, dy, K, rmse, poses);

    std::cout << "\n========== 最终结果 ==========\n";
    std::cout << "内参矩阵 K = \n" << K << std::endl;
    std::cout << "平均重投影误差 RMSE = " << rmse << " px\n";
    std::cout << "共求得 " << poses.size() << " 组外参\n";
    return 0;
}

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
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

int main() {
    std::string filePath = R"(D:\Code\CISCamera_DALSA\data\CISCamera_Image\Splice_20251107_212946982.bmp)";

    // 读取图像
    cv::Mat img = cv::imread(filePath, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "❌ 无法读取图像: " << filePath << std::endl;
        return -1;
    }

    auto result = std::make_shared<cv::Mat>(img);

    // 转为灰度图
    cv::Mat gray;
    if (result->channels() == 3)
        cv::cvtColor(*result, gray, cv::COLOR_BGR2GRAY);
    else
        gray = *result;

    const uchar whiteThresh = 250;
    int left = 0, right = gray.cols - 1;

    // ---------------------- 使用 reduce 计算列平均 ----------------------
    // 从下往上处理全黑行，把全黑行置为白色
    for (int r = gray.rows - 1; r >= 0; --r) {
        cv::Mat row = gray.row(r);
        double maxVal;
        cv::minMaxLoc(row, nullptr, &maxVal);
        if (maxVal == 0) {
            row.setTo(255);  // 全黑行置白
        } else {
            break;  // 遇到非全黑行停止
        }
    }

    // 使用 reduce 计算每列平均值
    cv::Mat colMean;
    cv::reduce(gray, colMean, 0, cv::REDUCE_AVG, CV_32F);

    // 找第一个非白列
    for (int c = 0; c < colMean.cols; ++c) {
        if (colMean.at<float>(0, c) < whiteThresh) {
            left = c;
            std::cout << "第一个非白列 (left) = " << left << std::endl;
            break;
        }
    }

    // 找最后一个非白列
    for (int c = colMean.cols - 1; c >= 0; --c) {
        if (colMean.at<float>(0, c) < whiteThresh) {
            right = c;
            std::cout << "第一个非白列 (right) = " << right << std::endl;
            break;
        }
    }

    // 防止越界，确保宽度大于10
    if (right > left + 10) {
        cv::Rect roi(left, 0, right - left + 1, result->rows);
        *result = (*result)(roi).clone();
    }

    // 显示原图和裁剪后的结果
    // 保存裁剪后的结果
    std::string savePath = R"(D:\Code\CISCamera_DALSA\data\CISCamera_Image\Splice_20251107_212946982_cropped.bmp)";
    cv::imwrite(savePath, *result);
    std::cout << "裁剪后的图像已保存到: " << savePath << std::endl;

    cv::waitKey(0);
    return 0;
}

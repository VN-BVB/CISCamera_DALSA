#include "lib_cb_detecor.h"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <chrono>
#include <ctime>
#include <iostream>

int LibCBDetector::file_counter = 0;  // 初始化静态变量

LibCBDetector::LibCBDetector() {
    // 初始化函数可以留空
}

void LibCBDetector::detect(const std::string& image_path, cbdetect::CornerType corner_type) {
    cbdetect::Corner corners;
    std::vector<cbdetect::Board> boards;
    cbdetect::Params params;
    std::vector<std::vector<cv::Point2d>> board_points_sorted;
    params.corner_type = corner_type;

    cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);

    auto t1 = std::chrono::high_resolution_clock::now();
    cbdetect::find_corners(img, corners, params);
    auto t2 = std::chrono::high_resolution_clock::now();

    auto t3 = std::chrono::high_resolution_clock::now();
    cbdetect::boards_from_corners(img, corners, boards, params);
    auto t4 = std::chrono::high_resolution_clock::now();

    std::cout << "Find corners took: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0
              << " ms\n";
    std::cout << "Find boards took: " << std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / 1000.0
              << " ms\n";
    std::cout << "Total took: "
              << (std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0 +
                  std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / 1000.0)
              << " ms\n";

    cbdetect::plot_board_points(img, corners, boards, board_points_sorted);

    if (!board_points_sorted.empty()) {
        const auto& points = board_points_sorted[0];  // 取第一个棋盘格
        saveBoardPoints(points);
    } else {
        std::cerr << "[警告] 未检测到棋盘格，无法保存角点坐标！" << std::endl;
    }
}

void LibCBDetector::processImagesInDirectory(const std::string& dir_path) {
    QDir dir(QString::fromStdString(dir_path));
    QStringList filters;
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    PLOGD << "正在检测棋盘格 ";
    for (const QFileInfo& fileInfo : files) {
        std::string file_path = fileInfo.absoluteFilePath().toStdString();
        PLOGD << "正在检测第 " << file_counter++ << " 图像: " << file_path;
        detect(file_path, cbdetect::SaddlePoint);  // 选择你需要的 CornerType
    }
}

void LibCBDetector::saveBoardPoints(const std::vector<cv::Point2d>& points) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char timestamp[64];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);
    std::string txtFile = "./data/CISCamera_Image/txt/Board1_Points_" + std::string(timestamp) + ".txt";

    std::ofstream ofs(txtFile);
    if (ofs.is_open()) {
        ofs << "# Index\tX\tY\n";
        for (size_t i = 0; i < points.size(); ++i) {
            ofs << i << "\t" << points[i].x << "\t" << points[i].y << "\n";
        }
        ofs.close();
        std::cout << "[保存完成] 棋盘格1角点已写入：" << txtFile << std::endl;
    } else {
        std::cerr << "[错误] 无法创建输出文件：" << txtFile << std::endl;
    }
}

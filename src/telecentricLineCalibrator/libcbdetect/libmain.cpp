// #include <QDir>
// #include <chrono>
// #include <fstream>
// #include <opencv2/opencv.hpp>
// #include <vector>

// #include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/boards_from_corners.h"
// #include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
// #include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/find_corners.h"
// #include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/plot_boards.h"
// #include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/plot_corners.h"

// using namespace std::chrono;

// void detect(const char* str, cbdetect::CornerType corner_type) {
//     cbdetect::Corner corners;
//     std::vector<cbdetect::Board> boards;
//     cbdetect::Params params;
//     params.corner_type = corner_type;

//     cv::Mat img = cv::imread(str, cv::IMREAD_GRAYSCALE);

//     auto t1 = high_resolution_clock::now();
//     cbdetect::find_corners(img, corners, params);
//     auto t2 = high_resolution_clock::now();
//     // cbdetect::plot_corners(img, corners);
//     // std::cout<<"plot corners:"<<corners<<std::endl;
//     auto t3 = high_resolution_clock::now();
//     cbdetect::boards_from_corners(img, corners, boards, params);
//     auto t4 = high_resolution_clock::now();
//     printf("Find corners took: %.3f ms\n", duration_cast<microseconds>(t2 - t1).count() / 1000.0);
//     printf("Find boards took: %.3f ms\n", duration_cast<microseconds>(t4 - t3).count() / 1000.0);
//     printf("Total took: %.3f ms\n",
//            duration_cast<microseconds>(t2 - t1).count() / 1000.0 + duration_cast<microseconds>(t4 - t3).count() / 1000.0);
//     // cbdetect::plot_boards(img, corners, boards, params);
//     std::vector<std::vector<cv::Point2d>> board_points_sorted;
//     cbdetect::plot_board_points(img, corners, boards, board_points_sorted);
//     // for (size_t board_idx = 0; board_idx < board_points_sorted.size(); ++board_idx) {
//     //     std::cout << "Board " << board_idx << ":\n";
//     //     const auto& points = board_points_sorted[board_idx];
//     //     for (size_t pt_idx = 0; pt_idx < points.size(); ++pt_idx) {
//     //         const auto& pt = points[pt_idx];
//     //         std::cout << "  Point " << pt_idx << ": (" << pt.x << ", " << pt.y << ")\n";
//     //     }
//     // }
//     // static int i = 0;
//     // // 假设第0个board用于标定
//     // if (!board_points_sorted.empty()) {
//     //     const auto& img_points = board_points_sorted[0];

//     //     int board_width = 8;        // 列数
//     //     int board_height = 6;       // 行数
//     //     float square_size = 35.0f;  // mm

//     //     // 调用独立的透视变换函数
//     //     cv::Mat img_warped =
//     //         cbdetect::warpToPlane(img, std::vector<cv::Point2f>(img_points.begin(), img_points.end()), board_width,
//     //         board_height, square_size, 5.0);

//     //     cv::imwrite("./result/calibdect_warped" + std::to_string(i++) + ".bmp", img_warped);
//     //     cv::imshow("Warped with Points", img_warped);
//     //     cv::waitKey(5);
//     // }
//     // ============= 在这里添加保存第一个棋盘格的角点 =============
//     if (!board_points_sorted.empty()) {
//         const auto& points = board_points_sorted[0];  // 取第一个棋盘格
//         auto now = std::chrono::system_clock::now();
//         std::time_t t = std::chrono::system_clock::to_time_t(now);
//         std::tm tm{};
// #ifdef _WIN32
//         localtime_s(&tm, &t);
// #else
//         localtime_r(&t, &tm);
// #endif
//         char timestamp[64];
//         std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);
//         std::string txtFile = "./example_data2/txt/Board1_Points_" + std::string(timestamp) + ".txt";
//         std::ofstream ofs(txtFile);
//         if (ofs.is_open()) {
//             ofs << "# Index\tX\tY\n";
//             for (size_t i = 0; i < points.size(); ++i) {
//                 ofs << i << "\t" << points[i].x << "\t" << points[i].y << "\n";
//             }
//             ofs.close();
//             std::cout << "[保存完成] 棋盘格1角点已写入：" << txtFile << std::endl;
//         } else {
//             std::cerr << "[错误] 无法创建输出文件：" << txtFile << std::endl;
//         }
//     } else {
//         std::cerr << "[警告] 未检测到棋盘格，无法保存角点坐标！" << std::endl;
//     }
// }

// int main(int argc, char* argv[]) {
//     printf("chessboards...");
//     // QDir dir("./calib/camera1/normal");
//     QDir dir("./example_data2/qbg");
//     QStringList filters;
//     QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
//     static int i = 0;
//     for (const QFileInfo& fileInfo : files) {
//         std::cout << " 正在检测第" << (i++) << "图像" << std::endl;
//         QString filePath = fileInfo.absoluteFilePath();
//         std::string stdFilePath = filePath.toStdString();
//         detect(stdFilePath.c_str(), cbdetect::SaddlePoint);
//     }

//     // printf("deltilles...");
//     // detect("../../example_data/e6.png", cbdetect::MonkeySaddlePoint);
//     return 0;
// }
// // QDir dir("./example_data");
// // QStringList filters;
// // filters << "*.bmp" << "*.png" << "*.jpg";

// // QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
// // for (const QFileInfo &fileInfo : files) {
// //     QString filePath = fileInfo.absoluteFilePath();
// //     qDebug() << "Detecting:" << filePath;

// //     detect(filePath.toStdString().c_str(), cbdetect::SaddlePoint);
// // }

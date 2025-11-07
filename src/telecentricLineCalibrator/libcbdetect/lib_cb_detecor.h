#ifndef LIB_CB_DETECOR_H
#define LIB_CB_DETECOR_H
#include <QDir>
#include <chrono>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/boards_from_corners.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/find_corners.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/plot_boards.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/plot_corners.h"

class LibCBDetector {
public:
    LibCBDetector();  // 构造函数
    void detect(const std::string& image_path, cbdetect::CornerType corner_type);
    void processImagesInDirectory(const std::string& dir_path);

private:
    void saveBoardPoints(const std::vector<cv::Point2d>& points);
    std::vector<std::vector<cv::Point2d>> board_points_sorted;
    static int file_counter;
};

#endif  // LIB_CB_DETECOR_H

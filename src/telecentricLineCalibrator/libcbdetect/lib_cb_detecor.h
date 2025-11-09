#ifndef LIB_CB_DETECOR_H
#define LIB_CB_DETECOR_H
#include <src/utils/ThreadPool.h>

#include <QDir>
#include <QObject>
#include <chrono>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "plog/Log.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/boards_from_corners.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/find_corners.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/plot_boards.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/plot_corners.h"

class LibCBDetector : public QObject {
    Q_OBJECT
public:
    LibCBDetector();  // 构造函数
    void detect(const std::string& image_path, cbdetect::CornerType corner_type);
    void processImagesInDirectory(const std::string& dir_path);

private:
    void saveBoardPoints(const std::vector<cv::Point2d>& points);
    // ThreadPool pool(std::thread::hardware_concurrency());
    static int file_counter;
signals:
    void sendImageReady(cv::Mat result);
    void sendText(const QString& msg);
};

#endif  // LIB_CB_DETECOR_H

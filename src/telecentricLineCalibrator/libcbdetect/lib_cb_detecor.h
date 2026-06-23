#ifndef LIB_CB_DETECOR_H
#define LIB_CB_DETECOR_H
#include <src/utils/ThreadPool.h>

#include <QDir>
#include <QObject>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "plog/Log.h"
#include "src/config/calibration_data_io.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/boards_from_corners.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/config.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/find_corners.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/plot_boards.h"
#include "src/telecentricLineCalibrator/libcbdetect/libcbdetect/plot_corners.h"
class LibCBDetector : public QObject {
    Q_OBJECT
public:
    LibCBDetector();  // 构造函数
    std::vector<std::vector<cv::Point2d>> detect(const cv::Mat& img, cbdetect::CornerType corner_type);
    std::vector<std::vector<cv::Point2d>> processSingleImage(const cv::Mat& img, int file_index);
    void test();

    void processImagesInDirectoryFilePath(const std::string& dir_path, std::vector<std::vector<std::vector<cv::Point2d>>>& allImagesBoardsPts);

    void processImagesFromMats(const std::vector<cv::Mat>& images, std::vector<std::vector<std::vector<cv::Point2d>>>& allImagesBoardsPts);
    cv::Mat visualizeCorners(const cv::Mat& img, const std::vector<cv::Point2d>& corners, const std::string& save_path);

private:
    void saveBoardPoints(const std::vector<cv::Point2d>& points);
    void saveBoardPointsFile(const std::vector<cv::Point2d>& points, const std::string& imageName);
    // ThreadPool pool(std::thread::hardware_concurrency());
    static int file_counter_;
    std::string parent_path_ = "./data/PaltfromCalibrate";
signals:
    void sendImageReady(cv::Mat result);
    void sendText(const QString& msg);
};

#endif  // LIB_CB_DETECOR_H

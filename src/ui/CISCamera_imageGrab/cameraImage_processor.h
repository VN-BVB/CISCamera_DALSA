#ifndef CAMERAIMAGE_PROCESSOR_H
#define CAMERAIMAGE_PROCESSOR_H
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

#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"
class CameraImageProcessor : public QObject {
    Q_OBJECT
public:
    explicit CameraImageProcessor(QObject* parent = nullptr);
    ~CameraImageProcessor();

    // 可选：开关拼接（也可在每次调用时传入）
    void setSpliceEnabled(bool enabled);

    bool readPointsFromTxt(const std::string& path, std::vector<Eigen::Vector2d>& pts);
signals:
    void text(const QString& msg);
    void error(const QString& msg);
    void imageReady(std::shared_ptr<cv::Mat> result);
    void sendSignalToCalibrate(const std::vector<std::vector<Eigen::Vector2d>>& all_imgPts,
                               const std::vector<Eigen::Vector2d>& worldPts, int width, int height, double dx, double dy,
                               Eigen::Matrix3d K_out, double rmse_out, std::vector<Pose> poses_out);

public slots:
    void processPair(std::shared_ptr<cv::Mat> master, std::shared_ptr<cv::Mat> slave, bool spliceEnabled, bool useColumnCheck);

    // 处理单张图（直通 / 为了复用通道）
    void processSingle(std::shared_ptr<cv::Mat> image);

    // 保存最近一次处理结果（或回退到master/slave）
    // dir：目录；prefix：文件前缀；ext：后缀（".png" ".tif" ".exr" …）
    // alsoSaveSingles：是否同时保存 master/slave（若存在）
    void saveResult(const QString& dir, const QString& prefix = "Splice", const QString& ext = ".png",
                    bool alsoSaveSingles = false);

    // 清空内部缓存
    void clear();

    void whenCameraCalibrate();

private:
    // 将两张图对齐为可拼接（统一类型/通道，行数一致）
    // 返回 false 则不可拼接
    bool prepareForConcat(const cv::Mat& m, const cv::Mat& s, cv::Mat& mOut, cv::Mat& sOut);

    // 根据扩展名选择参数保存
    bool imwriteSmart(const QString& path, const cv::Mat& img, QString& err);

private:
    std::shared_ptr<cv::Mat> lastMaster_;
    std::shared_ptr<cv::Mat> lastSlave_;
    std::shared_ptr<cv::Mat> lastResult_;
    //---棋盘格图像参数---
    const int W = 8, H = 11;
    const double spacingMM = 10.0;
    const double dx = 25.4 / 1200.0;  // mm/pixel (1200 dpi)
    const double dy = 17.0 / 800.0;   // 正方像素 （2（D + 1 ） / M）
    const int width = 31104, height = 16100;

    bool spliceEnabled_ = true;
    QMutex mtx_;  // 保护 last* 指针
    std::vector<std::vector<Eigen::Vector2d>> all_image_points;
};
#endif  // CAMERAIMAGE_PROCESSOR_H

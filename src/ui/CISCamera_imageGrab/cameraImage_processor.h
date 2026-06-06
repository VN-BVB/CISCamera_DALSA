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

#include "src/config/calibration_data_io.h"
class LibCBDetector;
class TelecentricLineCalibrator;
class TelecentricPlatformCalib;
class CameraImageProcessor : public QObject {
    Q_OBJECT
public:
    explicit CameraImageProcessor(QObject* parent = nullptr);
    ~CameraImageProcessor();

    // 可选：开关拼接（也可在每次调用时传入）
    void setSpliceEnabled(bool enabled);
    void whenDetectChessboard();
    void initCameraCalibrator();
    void lodaCameraCalibrateParams();
    void lodaCam2PlatCalibrateParams();
    std::vector<Eigen::Vector2d> convertToWorld(const std::vector<Eigen::Vector2d>& pix_pts);
signals:
    void text(const QString& msg);
    void error(const QString& msg);
    void imageReady(std::shared_ptr<cv::Mat> result);
    void sendSignalToCalibrate(const std::vector<std::vector<Eigen::Vector2d>>& all_imgPts, const std::vector<Eigen::Vector2d>& worldPts, int width,
                               int height, double dx, double dy, Eigen::Matrix3d K_out, double rmse_out, std::vector<Pose> poses_out);

public slots:
    void processPair(std::shared_ptr<cv::Mat> master, std::shared_ptr<cv::Mat> slave, bool spliceEnabled, bool useColumnCheck);

    // 处理单张图（直通 / 为了复用通道）
    void processSingle(std::shared_ptr<cv::Mat> image);

    // 保存最近一次处理结果（或回退到master/slave）
    // dir：目录；prefix：文件前缀；ext：后缀（".png" ".tif" ".exr" …）
    // alsoSaveSingles：是否同时保存 master/slave（若存在）
    void saveResult(const QString& dir, const QString& prefix = "Splice", const QString& ext = ".png", bool alsoSaveSingles = false);

    // 清空内部缓存
    void clear();

    void whenCameraCalibrate();

    void savePlatfromCailbImg(const QString& prefix, const QString& ext, int paltIndex);

    void loadPlatformCalibImages();

    void whenClearPlatFromFile(const QString& subFolder);

    void whenCalibrateCP();

private:
    bool prepareForConcat(const cv::Mat& m, const cv::Mat& s, cv::Mat& mOut, cv::Mat& sOut);
    bool imwriteSmart(const QString& path, const cv::Mat& img, QString& err);

private:
    std::shared_ptr<cv::Mat> lastMaster_;
    std::shared_ptr<cv::Mat> lastSlave_;
    std::shared_ptr<cv::Mat> lastResult_;
    double m_;
    Eigen::Matrix3d K_;
    Eigen::Matrix<double, 1, 5> coff_dis_;
    std::vector<Eigen::Vector3d> v_rot_s;
    std::vector<Eigen::Vector3d> v_trans_s;

    //---棋盘格图像参数---
    std::vector<Eigen::Vector2d> worldPts;
    const int W_ = 8, H_ = 11;
    const double spacingMM_ = 10.0;
    const double dx_ = 25.4 / 1200.0;  // mm/pixel (1200 dpi)
    const double dy_ = 20.0 / 945.0;   // 正方像素 （2（D + 1 ） / M）
    const int width_ = 30688, height_ = 33300;
    std::string readPointsPath_ = "./data/CISCamera_Image/mattxt";
    std::string readImgPath_ = "./data/CISCamera_Image/cameraCalibrate/img";
    // const int W = 8, H = 11;
    // const double spacingMM = 10.0;
    // const double dx = 25.4 / 1200.0;
    // const double dy = 25.4 / 1200.0;
    // const int width = 30688, height = 16100;
    bool spliceEnabled_ = true;
    QMutex mtx_;  // 保护 last* 指针
    std::vector<std::vector<Eigen::Vector2d>> all_image_points_;
    std::shared_ptr<LibCBDetector> libcbDetector{nullptr};
    std::shared_ptr<TelecentricLineCalibrator> telecentricLineCalibrator{nullptr};
    std::shared_ptr<TelecentricPlatformCalib> telecentricPlatCalibrator{nullptr};
    //-----对位平台标定------
    bool loadMode_ = true;
    int maxPlatformCount_ = 9;
    QString readPlatfromImg_ = "./data/PaltfromCalibrate/";
    std::vector<std::vector<cv::Mat>> all_platfromCalibImg_;
    std::vector<Eigen::Vector3d> allRotVecs_;    // 所有平台的旋转向量
    std::vector<Eigen::Vector3d> allTransVecs_;  // 所有平台的平移向量
};
#endif  // CAMERAIMAGE_PROCESSOR_H

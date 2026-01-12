#ifndef TELECENTRIC_LINE_CALIBRATOR_H
#define TELECENTRIC_LINE_CALIBRATOR_H
#define _USE_MATH_DEFINES
// clang-format off
#include "py_telecentric_optimizer.h"
// clang-format on
#include <Eigen/Core>
#include <Eigen/Dense>
#include <QObject>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "calib_utils.h"
#include "plog/Log.h"
#include "src/config/calibration_data_io.h"
#include "telecentric_lm_optimizer.h"
enum class PatternType { CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };

class TelecentricLineCalibrator : public QObject {
    Q_OBJECT
public:
    TelecentricLineCalibrator();

    /**
     * @brief 线阵远心镜头标定（DLT 初值求解）
     * @param images         输入：标定板图像列表（cv::Mat）
     * @param boardSize      输入：标定板圆点阵规格（列×行）
     * @param circleRadiusMM 输入：圆点半径（mm）
     * @param spacingMM      输入：相邻圆心间距（mm）
     * @param patternType    输入：标定板类型
     * @param dx             输入：初始X方向像素尺度
     * @param dy             输入：初始Y方向像素尺度
     * @param patternType    输入：标定板类型
     * @param K_out          输出：相机内参矩阵（Eigen::Matrix3d）
     * @param rmse_out       输出：平均重投影误差（像素）
     * @param poses_out      输出：每幅图像的外参（旋转 R，平移 t，及对应误差）
     * @return true 若标定成功
     */

    bool calculate_Image_Points(cv::Mat imageInput, cv::Size boardSize, std::vector<cv::Point2d>& imagePoints);

    bool calibrateCameraFromPointsDemo(const std::vector<std::vector<Eigen::Vector2d>>& all_imgPts, const std::vector<Eigen::Vector2d>& worldPts,
                                       int width, int height, double dx, double dy, Eigen::Matrix3d K_out, double rmse_out,
                                       std::vector<Pose> poses_out);
    // -------------------- 齐次坐标 --------------------
    Eigen::MatrixXd toHomogeneous(const Eigen::MatrixXd& points);

    // -------------------- 正向畸变 --------------------
    Eigen::MatrixXd distort(const Eigen::Matrix<double, 1, 5>& coff_dis, const Eigen::MatrixXd& normalized_proj);

    // -------------------- 迭代去畸变 --------------------
    Eigen::MatrixXd undistortPointsIter(const Eigen::MatrixXd& points_px, const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& coff_dis,
                                        int max_iter = 200, double tol = 1e-9);

    // -------------------- 像素坐标 → 相机坐标 --------------------
    Eigen::MatrixXd pixelToCameraCoordinates(const Eigen::MatrixXd& points_px, const Eigen::Matrix3d& K,
                                             const Eigen::Matrix<double, 1, 5>& coff_dis = {});
    // -------------------- 相机坐标 → 世界坐标 --------------------
    Eigen::MatrixXd cameraToWorldCoordinates(const Eigen::MatrixXd& cam_pts, const Eigen::Vector3d& v_rot, const Eigen::Vector3d& v_trans);
    Eigen::MatrixXd cameraToWorldCoordinatesSO2(const Eigen::MatrixXd& cam_pts, const Eigen::Vector3d& v_rot, const Eigen::Vector3d& v_trans);

    Pose estimateTelecentricPose(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& coff_dis, const double m, const double u0,
                                 const double v0, const double dx, const double dy, const std::vector<Eigen::Vector2d>& worldPts,
                                 const std::vector<Eigen::Vector2d>& imgPts);
    double computeReprojectionErrorDemo(const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imagePts, const Pose& pose,
                                        const Eigen::Matrix3d& K, const std::string& savePath, const Eigen::Matrix<double, 1, 5>& coff_dis);

    bool optimizeExtrinsicsWithLeastSquares(const Eigen::MatrixXd& transformedPts, const std::vector<Eigen::Vector2d>& trueWorldPts,
                                            const Eigen::Matrix3d& R_in, const Eigen::Vector3d& t_in, Eigen::Matrix3d& R_out, Eigen::Vector3d& t_out);

private:
    Eigen::Matrix3d computeHomography(const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imagePts);
    double compute_m_from_H(const Eigen::Matrix3d& H, double dx, double dy);
    bool estimateIntrinsicsFromHomographies(const std::vector<Eigen::Matrix3d>& Hs, double dx, double dy, double u0, double v0,
                                            const std::vector<double>& reprojErrors);
    Pose extractPoseFromHomography(const Eigen::Matrix3d& H, const Eigen::Matrix3d& K, const std::vector<Eigen::Vector2d>& worldPts,
                                   const std::vector<Eigen::Vector2d>& imagePts, const double m, const double u0, const double v0, const double dx,
                                   const double dy);
    Eigen::Matrix3d initIntrinsic(double m, double dx, double dy, double u0, double v0);
    double computeReprojectionError(const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imagePts, const Pose& pose,
                                    const Eigen::Matrix3d& K, const std::string& savePath = " ",
                                    const Eigen::Matrix<double, 1, 5>& coff_dis = {0.0, 0.0, 0.0, 0.0, 0.0});
    double computeReprojectionErrorFinal(const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imagePts, const Pose& pose,
                                         const Eigen::Matrix3d& K, double k);

private:
    double m_, dx_, dy_;
    double u0_, v0_;
    Eigen::Matrix3d K_;
    Eigen::Matrix<double, 1, 5> coff_dis_;
    const std::string calib_data_path_ = "./data/calibration_config/before_optimization_calib_data.json";
signals:
    void sendSignalSuccessCalib();
};
#endif  // TELECENTRIC_LINE_CALIBRATOR_H

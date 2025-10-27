#ifndef TELECENTRIC_LINE_CALIBRATOR_H
#define TELECENTRIC_LINE_CALIBRATOR_H
#define _USE_MATH_DEFINES
#include <Eigen/Core>
#include <Eigen/Dense>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "plog/Log.h"
#include "telecentric_lm_optimizer.h"
enum class PatternType { CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
// struct Pose {
//     Eigen::Matrix3d R;
//     Eigen::Vector3d t;
//     double reprojErr;
// };
class TelecentricLineCalibrator {
public:
    TelecentricLineCalibrator();
    Eigen::Vector3d rotMatToVec(const Eigen::Matrix3d& R) const;

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
    bool calibrateCamera(const std::vector<cv::Mat>& images, cv::Size boardSize, double circleRadiusMM, double spacingMM,
                         PatternType patternType, double dx, double dy, Eigen::Matrix3d& K_out, double& rmse_out,
                         std::vector<Pose>& poses_out);

    // 圆点检测函数（由你提供的版本）
    bool calculate_Image_Points(cv::Mat imageInput, cv::Size boardSize, std::vector<cv::Point2d>& imagePoints);

    bool calibrateCameraFromPointsDemo(const std::vector<std::vector<Eigen::Vector2d>>& all_imgPts,
                                       const std::vector<Eigen::Vector2d>& worldPts, int width, int height, double dx, double dy,
                                       Eigen::Matrix3d& K_out, double& rmse_out, std::vector<Pose>& poses_out);

private:
    Eigen::Matrix3d computeHomography(const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imagePts);
    double compute_m_from_H(const Eigen::Matrix3d& H, double dx, double dy);
    bool estimateIntrinsicsFromHomographies(const std::vector<Eigen::Matrix3d>& Hs, double dx, double dy, double u0, double v0,
                                            const std::vector<double>& reprojErrors);
    Pose extractPoseFromHomography(const Eigen::Matrix3d& H, const Eigen::Matrix3d& K,
                                   const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imagePts,
                                   const double m);
    Eigen::Matrix3d initIntrinsic(const Eigen::Matrix3d& H, double dx, double dy, double u0, double v0);
    double computeReprojectionError(const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imagePts,
                                    const Pose& pose, const Eigen::Matrix3d& K);
    double computeReprojectionErrorFinal(const std::vector<Eigen::Vector2d>& worldPts,
                                         const std::vector<Eigen::Vector2d>& imagePts, const Pose& pose, const Eigen::Matrix3d& K,
                                         double k);

private:
    double m_, dx_, dy_;
    double u0_, v0_;
    Eigen::Matrix3d K_;
};
#endif  // TELECENTRIC_LINE_CALIBRATOR_H

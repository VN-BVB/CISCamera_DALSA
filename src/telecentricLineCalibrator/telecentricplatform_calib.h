#ifndef TELECENTRICPLATFORM_CALIB_H
#define TELECENTRICPLATFORM_CALIB_H

#include <Eigen/Dense>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "src/config/calibration_data_io.h"
#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"
class TelecentricPlatformCalib {
public:
    TelecentricPlatformCalib();
    TelecentricPlatformCalib(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& dist, const Eigen::Vector3d& rvec,
                             const Eigen::Vector3d& tvec);
    ~TelecentricPlatformCalib() = default;

    // 主流程
    void runDemo(std::vector<std::vector<Eigen::Vector2d>> pts = {});
    bool estimatePlatformPoseFromBoards(const std::vector<std::vector<std::vector<cv::Point2d>>>& onePlatformBoards,
                                        Eigen::Vector3d& rvec_plat, Eigen::Vector3d& t_plat_cam);
    std::vector<Eigen::Vector2d> matToVec(const Eigen::MatrixXd& M);
    Eigen::MatrixXd vecToMat(const std::vector<Eigen::Vector2d>& v);

private:
    // pixel → camera → world
    std::vector<Eigen::Vector2d> convertToWorld(const std::vector<Eigen::Vector2d>& pix_pts);

    // 方向拟合（SVD）
    Eigen::Vector3d computeDirectionLS(const std::vector<Eigen::Vector2d>& ptsA, const std::vector<Eigen::Vector2d>& ptsB);

    // 多组旋转求旋转中心
    Eigen::Vector2d computeRotationCenterSequential(const std::vector<std::vector<Eigen::Vector2d>>& pts);
    Eigen::Vector2d fitCircleTaubin(const std::vector<Eigen::Vector2d>& pts);
    Eigen::Vector2d computeRotationCenterCircleFit(const std::vector<std::vector<Eigen::Vector2d>>& pts);

private:
    // 相机参数
    Eigen::Matrix3d K_;
    Eigen::Matrix<double, 1, 5> dist_;
    Eigen::Vector3d v_rot_;
    Eigen::Vector3d v_trans_;

    // 内部使用的 TelecentricLineCalibrator（你已有类）
    class TelecentricLineCalibrator* lineCalib_;
};

#endif  // TELECENTRICPLATFORM_CALIB_H

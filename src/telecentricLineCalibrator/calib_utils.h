#ifndef CALIB_UTILS_H
#define CALIB_UTILS_H
#include <Eigen/Dense>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

inline Eigen::MatrixXd vecToMat(const std::vector<Eigen::Vector2d>& v) {
    Eigen::MatrixXd M(v.size(), 2);
    for (int i = 0; i < v.size(); i++) M.row(i) = v[i];
    return M;
}

inline std::vector<Eigen::Vector2d> matToVec(const Eigen::MatrixXd& M) {
    std::vector<Eigen::Vector2d> v;
    v.reserve(M.rows());
    for (int i = 0; i < M.rows(); i++) v.emplace_back(M(i, 0), M(i, 1));
    return v;
}
inline Eigen::Vector3d rotMatToVec(const Eigen::Matrix3d& R) {
    cv::Mat R_cv(3, 3, CV_64F);
    cv::Mat rvec_cv;
    // Eigen矩阵转OpenCV矩阵
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) R_cv.at<double>(i, j) = R(i, j);
    // Rodrigues变换（旋转矩阵→旋转向量）
    cv::Rodrigues(R_cv, rvec_cv);
    // OpenCV矩阵转Eigen向量
    return Eigen::Vector3d(rvec_cv.at<double>(0), rvec_cv.at<double>(1), rvec_cv.at<double>(2));
}
inline Eigen::Matrix3d vecToRotMat(const Eigen::Vector3d& rvec) {
    // 构造 OpenCV 的旋转向量
    cv::Mat rvec_cv(3, 1, CV_64F);
    rvec_cv.at<double>(0) = rvec.x();
    rvec_cv.at<double>(1) = rvec.y();
    rvec_cv.at<double>(2) = rvec.z();

    // Rodrigues: 旋转向量 → 旋转矩阵
    cv::Mat R_cv;
    cv::Rodrigues(rvec_cv, R_cv);

    // OpenCV → Eigen
    Eigen::Matrix3d R;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) R(i, j) = R_cv.at<double>(i, j);

    return R;
}

#endif  // CALIB_UTILS_H

#ifndef TELECENTRIC_LM_OPTIMIZER_H
#define TELECENTRIC_LM_OPTIMIZER_H
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
struct Pose {
    Eigen::Matrix3d R;
    Eigen::Vector3d t;
    double reprojErr;
};
/**
 * LM非线性优化类
 * 功能：优化相机内参、外参、倾斜角、畸变系数，最小化重投影误差
 */
class TelecentricLMOptimizer {
public:
    /**
     * 构造函数：初始化标定数据与参数初始值
     * @param all_image_pts 所有姿态的实际图像点（u,v，亚像素精度）
     * @param world_pts 世界坐标系下的标定板特征点（Xw,Yw，Z=0）
     * @param init_poses 外参初始值（来自DLT初步标定）
     * @param init_m 放大倍率m初始值（参考文档1.71节公式）
     * @param init_dx 水平像素尺寸dx（已知，镜头手册，不优化）
     * @param init_dy 垂直像素尺寸dy初始值（参考文档1.72节h_v/h_l）
     * @param init_u0 主点水平坐标u0初始值（图像中心）
     * @param init_v0 主点垂直坐标v0初始值（图像中心）
     * @param init_theta 倾斜角θ初始值（参考文档1.91节）
     * @param init_k 径向畸变系数k初始值（参考文档1.91节）
     */
    TelecentricLMOptimizer(const std::vector<std::vector<Eigen::Vector2d>>& all_image_pts,
                           const std::vector<Eigen::Vector2d>& world_pts, const std::vector<Pose>& init_poses, double init_m,
                           double init_dx, double init_dy, double init_u0, double init_v0, double init_theta, double init_k);

    /**
     * 执行LM优化
     * @param max_iter 最大迭代次数（建议100，参考文档1.92节）
     * @param eps_error 误差收敛阈值（建议1e-8）
     * @param eps_param 参数变化阈值（建议1e-10）
     * @param init_lambda 初始阻尼系数（建议1.0）
     * @return 优化是否成功
     */
    bool optimize(int max_iter = 100, double eps_error = 1e-8, double eps_param = 1e-10, double init_lambda = 1.0);

    /**
     * 获取优化结果
     * @param m 优化后的放大倍率
     * @param dy 优化后的垂直像素尺寸
     * @param u0 优化后的主点水平坐标
     * @param v0 优化后的主点垂直坐标
     * @param theta 优化后的倾斜角
     * @param k 优化后的径向畸变系数
     * @param optimized_poses 优化后的外参
     * @param total_reprojErr 总重投影RMSE
     */
    void getOptimizedParams(double& m, double& dy, double& u0, double& v0, double& theta, double& k,
                            std::vector<Pose>& optimized_poses, double& total_reprojErr) const;

private:
    // 待优化参数（参考文档1.86节）
    double m_;                         // 内参：放大倍率
    double dy_;                        // 内参：垂直像素尺寸
    double u0_;                        // 内参：主点水平坐标
    double v0_;                        // 内参：主点垂直坐标
    double theta_;                     // 运动参数：倾斜角
    double k_;                         // 畸变参数：径向畸变系数
    std::vector<Pose> current_poses_;  // 外参：所有姿态的R、t
    const int num_global_params = 6;   // 全局参数数量
    const int num_pose_params = 6;     // 外部参数数量
    double init_m_;                    // 放大倍率初始值
    double init_dx_;                   // 水平像素尺寸初始值
    double init_dy_;                   // 垂直像素尺寸初始值
    double init_u0_;                   // 主点x初始值
    double init_v0_;                   // 主点y初始值
    double init_theta_;                // 倾斜角初始值
    double init_k_;                    // 畸变系数初始值

    // 标定数据（固定）
    const std::vector<std::vector<Eigen::Vector2d>> all_image_pts_;  // 实际图像点
    const std::vector<Eigen::Vector2d> world_pts_;                   // 世界点
    const std::vector<Pose> init_poses_;                             // 外参初始值
    const double dx_;                                                // 水平像素尺寸（已知）

    // 参数保存与恢复（LM回溯用）
    double saved_m_, saved_dy_, saved_u0_, saved_v0_, saved_theta_, saved_k_;
    std::vector<Pose> saved_poses_;

    /**
     * 世界点→重投影像素点（核心投影函数，含倾斜角与畸变）
     * @param world_pt 世界点（Xw,Yw）
     * @param pose 外参（R,t）
     * @return 重投影像素点（hat_u, hat_v）
     */
    Eigen::Vector2d projectWorldToImage(const Eigen::Vector2d& world_pt, const Pose& pose) const;

    /**
     * 计算单姿态的重投影RMSE
     * @param image_pts 单姿态的实际图像点
     * @param pose 单姿态的外参
     * @return 重投影RMSE
     */
    double computePoseReprojectionError(const std::vector<Eigen::Vector2d>& image_pts, const Pose& pose) const;

    /**
     * 计算所有姿态的总重投影RMSE（优化目标函数值）
     * @return 总重投影RMSE
     */
    double computeTotalReprojectionError() const;

    /**
     * 构建雅克比矩阵J与误差向量e
     * @param J 输出雅克比矩阵（2*N × M，N为总点数，M为参数数）
     * @param e 输出误差向量（2*N × 1）
     */
    void buildJacobianAndError(Eigen::MatrixXd& J, Eigen::VectorXd& e) const;

    /**
     * 计算误差对全局参数的偏导数（填充雅克比矩阵）
     * @param world_pt 世界点
     * @param pose 外参
     * @param hat_pt 重投影点
     * @param J_row_u u方向误差的雅克比行
     * @param J_row_v v方向误差的雅克比行
     * @param param_start_idx 全局参数在J中的起始列索引
     */
    void computeGlobalParamDerivatives(const Eigen::Vector2d& world_pt, const Pose& pose, const Eigen::Vector2d& hat_pt,
                                       Eigen::RowVectorXd& J_row_u, Eigen::RowVectorXd& J_row_v, int param_start_idx) const;

    /**
     * 计算误差对姿态外参的偏导数（填充雅克比矩阵）
     * @param world_pt 世界点
     * @param pose 外参
     * @param hat_pt 重投影点
     * @param J_row_u u方向误差的雅克比行
     * @param J_row_v v方向误差的雅克比行
     * @param param_start_idx 当前姿态参数在J中的起始列索引
     */
    void computePoseParamDerivatives(const Eigen::Vector2d& world_pt, const Pose& pose, const Eigen::Vector2d& hat_pt,
                                     Eigen::RowVectorXd& J_row_u, Eigen::RowVectorXd& J_row_v, int param_start_idx) const;

    // 保存当前参数（用于LM回溯）
    void saveCurrentParams();

    // 恢复保存的参数（LM更新无效时）
    void restoreSavedParams();

    // 用增量更新所有参数
    void updateParams(const Eigen::VectorXd& delta);

    /**
     * 旋转矩阵→欧拉角（参数化转换）
     * TODO：需与初步标定的旋转参数化方式一致（如ZYX/XYZ顺序）
     * @param R 旋转矩阵
     * @param rx x轴旋转角（弧度）
     * @param ry y轴旋转角（弧度）
     * @param rz z轴旋转角（弧度）
     */
    void eigen2Euler(const Eigen::Matrix3d& R, double& rx, double& ry, double& rz) const;

    /**
     * 欧拉角→旋转矩阵（参数化转换）
     * TODO：需与eigen2Euler对应（同一种欧拉角顺序）
     * @param rx x轴旋转角（弧度）
     * @param ry y轴旋转角（弧度）
     * @param rz z轴旋转角（弧度）
     * @param R 输出旋转矩阵
     */
    void euler2Eigen(double rx, double ry, double rz, Eigen::Matrix3d& R) const;
    Eigen::Matrix3d computeDRdx(double rx, double ry, double rz) const;
    Eigen::Matrix3d computeDRdy(double rx, double ry, double rz) const;
    Eigen::Matrix3d computeDRdz(double rx, double ry, double rz) const;
    void printParamChanges();
    Eigen::Vector3d rotMatToVec(const Eigen::Matrix3d &R) const;
    Eigen::Matrix3d rotVecToMat(const Eigen::Vector3d &rvec) const;
    void computeDRdr(const Eigen::Vector3d &rvec, Eigen::Matrix3d &dRdr1, Eigen::Matrix3d &dRdr2, Eigen::Matrix3d &dRdr3) const;
    Eigen::VectorXd packParams() const;
    void unpackParams(const Eigen::VectorXd &params);
};

#endif  // TELECENTRIC_LM_OPTIMIZER_H

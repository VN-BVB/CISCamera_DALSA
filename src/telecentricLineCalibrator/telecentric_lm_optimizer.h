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
#include <unsupported/Eigen/NonLinearOptimization>  // LM
#include <unsupported/Eigen/NumericalDiff>          // 数值导数自动计算
#include <vector>

#include "plog/Log.h"
// 姿态结构体（与原有代码保持一致）
#include "src/config/calibration_data_io.h"
extern double rms;
class TelecentricLMOptimizer {
public:
    using Scalar = double;
    // 构造函数：输入初始数据
    TelecentricLMOptimizer(const std::vector<std::vector<Eigen::Vector2d>>& all_img_pts,
                           const std::vector<Eigen::Vector2d>& world_pts, const std::vector<Pose>& init_poses, double init_m,
                           double init_dx, double init_dy, double init_u0, double init_v0, double init_theta, double init_k);

    // 执行优化
    bool optimize(int max_iter, double eps_error, double eps_param, double init_lambda);

    // 获取优化结果
    void getOptimizedParams(double& opt_m, double& opt_dy, double& opt_u0, double& opt_v0, double& opt_theta, double& opt_k,
                            std::vector<Pose>& opt_poses, double& total_reproj_err) const;
    // 参数可动性与缩放控制
    std::vector<bool> param_fixed_;    // 是否固定
    std::vector<double> param_scale_;  // 敏感度缩放比例

private:
    // 优化问题定义（友元类用于访问私有成员）
    struct CostFunctor {
        using Scalar = TelecentricLMOptimizer::Scalar;
        using InputType = Eigen::VectorXd;
        using ValueType = Eigen::VectorXd;
        using JacobianType = Eigen::MatrixXd;

        // 关键新增：数值微分需要的编译期参数维度（动态）
        static constexpr int InputsAtCompileTime = Eigen::Dynamic;
        // 关键新增：数值微分需要的编译期残差维度（动态）
        static constexpr int ValuesAtCompileTime = Eigen::Dynamic;

        const TelecentricLMOptimizer& optimizer;

        CostFunctor(const TelecentricLMOptimizer& opt) : optimizer(opt) {}

        int operator()(const InputType& params, ValueType& residuals) const;
        int inputs() const { return optimizer.param_count_; }
        int values() const { return optimizer.residual_count_; }
    };
    using NumericalDiffFunctor = Eigen::NumericalDiff<CostFunctor>;
    // 数据存储
    std::vector<std::vector<Eigen::Vector2d>> all_img_pts_;  // 所有图像点
    std::vector<Eigen::Vector2d> world_pts_;                 // 世界坐标点
    int img_count_;                                          // 图像数量
    int points_per_img_;                                     // 每幅图像的点数

    // 参数维度
    int param_count_;     // 总参数数量
    int residual_count_;  // 残差数量
    // 优化前后的参数
    Eigen::VectorXd optimized_params_;
    double init_m_;
    double init_dx_;
    double init_dy_;
    double init_u0_;
    double init_v0_;
    double init_theta_;
    double init_k_;
    std::vector<Pose> init_poses_;

    // 编码：将参数转换为优化向量
    Eigen::VectorXd encodeParams() const;

    // 解码：从优化向量恢复参数
    void decodeParams(const Eigen::VectorXd& params, double& m, double& dy, double& u0, double& v0, double& theta, double& k,
                      std::vector<Pose>& poses) const;

    // 计算重投影残差
    void computeResiduals(const Eigen::VectorXd& params, Eigen::VectorXd& residuals) const;
    void setParamFixed(int idx, bool fixed);
    void setParamScale(int idx, double scale);

    friend class TelecentricLineCalibrator;
};

#endif  // TELECENTRIC_LM_OPTIMIZER_H

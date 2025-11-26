#include "telecentric_lm_optimizer.h"

#include <cmath>
#include <iostream>
double totalErr;
double rms;
// 旋转向量转旋转矩阵（Rodrigues变换）
Eigen::Matrix3d rotVecToMat(const Eigen::Vector3d& rvec) {
    double theta = rvec.norm();
    if (theta < 1e-8) return Eigen::Matrix3d::Identity();

    Eigen::Vector3d r = rvec.normalized();
    double c = cos(theta);
    double s = sin(theta);
    double cc = 1 - c;

    Eigen::Matrix3d R;
    R << c + r.x() * r.x() * cc, r.x() * r.y() * cc - r.z() * s, r.x() * r.z() * cc + r.y() * s, r.y() * r.x() * cc + r.z() * s,
        c + r.y() * r.y() * cc, r.y() * r.z() * cc - r.x() * s, r.z() * r.x() * cc - r.y() * s, r.z() * r.y() * cc + r.x() * s,
        c + r.z() * r.z() * cc;
    return R;
}

TelecentricLMOptimizer::TelecentricLMOptimizer(const std::vector<std::vector<Eigen::Vector2d>>& all_img_pts,
                                               const std::vector<Eigen::Vector2d>& world_pts, const std::vector<Pose>& init_poses,
                                               double init_m, double init_dx, double init_dy, double init_u0, double init_v0,
                                               double init_theta, double init_k)
    : all_img_pts_(all_img_pts),
      world_pts_(world_pts),
      init_poses_(init_poses),
      init_m_(init_m),
      init_dx_(init_dx),
      init_dy_(init_dy),
      init_u0_(init_u0),
      init_v0_(init_v0),
      init_theta_(init_theta),
      init_k_(init_k) {
    img_count_ = all_img_pts.size();
    points_per_img_ = world_pts.size();

    // 计算参数数量：6个内参 + 每幅图像6个外参(3旋转+3平移)
    param_count_ = 6 + img_count_ * 6;
    // 残差数量：每点2个残差(u和v方向)
    residual_count_ = img_count_ * points_per_img_ * 2;
    // --- 初始化控制参数 ---
    param_fixed_.resize(param_count_, false);  // 默认所有参数都可优化
    param_scale_.resize(param_count_, 1.0);    // 默认缩放比例为1.0
    // === 内参灵敏度 === >1快  < 1 man
    setParamScale(0, 1.0);  // m：放大倍率，
    setParamScale(1, 1.0);  // dy：默认
    setParamScale(2, 1.0);  // u0：像素坐标
    setParamScale(3, 1.0);  // v0：像素坐标
    setParamScale(4, 1.0);  // theta：倾斜角
    setParamScale(5, 1.0);  // k：畸变系数

    // === 外参灵敏度 ===
    int idx = 6;
    for (int i = 0; i < img_count_; ++i) {
        setParamScale(idx + 0, 1);    // r1
        setParamScale(idx + 1, 1);    // r2
        setParamScale(idx + 2, 1);    // r3
        setParamScale(idx + 3, 1.0);  // tx
        setParamScale(idx + 4, 1.0);  // ty
        setParamScale(idx + 5, 1);    // tz（z方向可能影响小，但保留）
        idx += 6;
    }
}

Eigen::VectorXd TelecentricLMOptimizer::encodeParams() const {
    Eigen::VectorXd params(param_count_);
    int idx = 0;

    // 内参：m, dy, u0, v0, theta, k
    params[idx++] = init_m_;
    params[idx++] = init_dy_;
    params[idx++] = init_u0_;
    params[idx++] = init_v0_;
    params[idx++] = init_theta_;
    params[idx++] = init_k_;

    // 外参：每幅图像的旋转向量(3)和平移向量(3)
    for (const auto& pose : init_poses_) {
        // 旋转矩阵转旋转向量
        Eigen::Vector3d rvec;
        Eigen::Matrix3d R = pose.R;
        double theta = acos((R.trace() - 1) / 2);
        if (theta > 1e-8) {
            Eigen::Vector3d r_axis;
            r_axis << R(2, 1) - R(1, 2), R(0, 2) - R(2, 0), R(1, 0) - R(0, 1);
            rvec = theta * r_axis.normalized();
        } else {
            rvec.setZero();
        }

        params[idx++] = rvec.x();
        params[idx++] = rvec.y();
        params[idx++] = rvec.z();
        params[idx++] = pose.t.x();
        params[idx++] = pose.t.y();
        params[idx++] = pose.t.z();
    }

    // 应用灵敏度缩放
    for (int i = 0; i < param_count_; ++i) params[i] /= param_scale_[i];

    return params;
}

void TelecentricLMOptimizer::decodeParams(const Eigen::VectorXd& params, double& m, double& dy, double& u0, double& v0,
                                          double& theta, double& k, std::vector<Pose>& poses) const {
    int idx = 0;
    poses.resize(img_count_);
    Eigen::VectorXd scaled = params;
    // 解缩放
    for (int i = 0; i < param_count_; ++i) scaled[i] *= param_scale_[i];

    // 内参
    m = scaled[idx++];
    dy = scaled[idx++];
    u0 = scaled[idx++];
    v0 = scaled[idx++];
    theta = scaled[idx++];
    k = scaled[idx++];

    // 外参
    for (int i = 0; i < img_count_; ++i) {
        Eigen::Vector3d rvec(scaled[idx], scaled[idx + 1], scaled[idx + 2]);
        idx += 3;
        Eigen::Vector3d t(scaled[idx], scaled[idx + 1], scaled[idx + 2]);
        idx += 3;
        poses[i].R = rotVecToMat(rvec);
        poses[i].t = t;
    }
}

void TelecentricLMOptimizer::computeResiduals(const Eigen::VectorXd& params, Eigen::VectorXd& residuals) const {
    double m, dy, u0, v0, theta, k;
    std::vector<Pose> poses;
    decodeParams(params, m, dy, u0, v0, theta, k, poses);

    // 内参矩阵计算
    double tan_t = tan(theta);
    double cos_t = cos(theta);
    Eigen::Matrix3d K;
    K << m / init_dx_, -m * tan_t / init_dx_, u0, 0, 1 / dy * cos_t, v0 / m, 0, 0, 1;
    int res_idx = 0;
    totalErr = 0;
    // 计算每幅图像的残差
    for (int i = 0; i < img_count_; ++i) {
        const auto& img_pts = all_img_pts_[i];
        const auto& pose = poses[i];

        Eigen::Matrix2d R2 = pose.R.block<2, 2>(0, 0);
        Eigen::Vector2d t2 = pose.t.head<2>();  // 仅取平面平移
        int everyImgres_idx = 0;
        Eigen::VectorXd everyImgResiduals(2 * img_pts.size());
        everyImgResiduals.setZero();
        // 每点的残差
        for (int j = 0; j < points_per_img_; ++j) {
            const Eigen::Vector2d& Pw = world_pts_[j];
            const Eigen::Vector2d& img_pt = img_pts[j];

            // 世界坐标 -> 相机坐标
            Eigen::Vector2d img_affine = R2 * Pw + t2;
            Eigen::Vector3d Pc(img_affine(0), img_affine(1), 1.0);
            // 相机坐标 -> 图像坐标（无畸变）
            Eigen::Vector3d uvw = K * Pc;
            double u = uvw.x() / uvw.z();
            double v = uvw.y() / uvw.z();

            // 应用畸变模型
            double delta_x = k * u * (u * u + (v0 * dy) * (v0 * dy));
            double delta_y = -k * v0 * dy * (u * u + (v0 * dy) * (v0 * dy));
            u += delta_x;
            v += delta_y;

            // 残差 = 预测值 - 观测值
            residuals[res_idx++] = u - img_pt.x();
            residuals[res_idx++] = v - img_pt.y();
            everyImgResiduals[everyImgres_idx++] = u - img_pt.x();
            everyImgResiduals[everyImgres_idx++] = v - img_pt.y();
        }
        double rms = std::sqrt(everyImgResiduals.squaredNorm() / img_pts.size());
        totalErr += rms;
    }
}

int TelecentricLMOptimizer::CostFunctor::operator()(const InputType& params, ValueType& residuals) const {
    optimizer.computeResiduals(params, residuals);
    // 打印当前残差均方根（RMS）
    rms = totalErr / 12;
    std::cout << "当前RMS = " << rms << std::endl;
    return 0;
}

bool TelecentricLMOptimizer::optimize(int max_iter, double eps_error, double eps_param, double init_lambda) {
    // 初始参数
    Eigen::VectorXd params = encodeParams();
    optimized_params_ = params;

    // 构建优化器
    CostFunctor functor(*this);
    NumericalDiffFunctor num_diff_functor(functor);                        // 数值微分包装
    Eigen::LevenbergMarquardt<NumericalDiffFunctor> lm(num_diff_functor);  // 传入包装后的函数
    lm.parameters.maxfev = max_iter;                                       // 最大函数评估次数（对应原max_iter）
    lm.parameters.xtol = eps_param;                                        // 参数变化阈值
    lm.parameters.ftol = eps_error;                                        // 误差变化阈值
    lm.parameters.factor = init_lambda;                                    // 初始阻尼系数（原init_lambda）
    // 可选：设置梯度阈值（根据需要调整）
    lm.parameters.gtol = 1e-6;

    // 执行优化（获取返回状态）
    Eigen::LevenbergMarquardtSpace::Status status = lm.minimize(optimized_params_);

    // 状态解释字符串
    std::string status_str;
    switch (status) {
        case Eigen::LevenbergMarquardtSpace::NotStarted:
            status_str = "未开始优化（内部状态错误）";
            break;
        case Eigen::LevenbergMarquardtSpace::Running:
            status_str = "正在运行（未正常终止）";
            break;
        case Eigen::LevenbergMarquardtSpace::ImproperInputParameters:
            status_str = "输入参数无效（如维度不匹配）";
            break;
        case Eigen::LevenbergMarquardtSpace::RelativeReductionTooSmall:
            status_str = "目标函数相对减少量过小（收敛）";
            break;
        case Eigen::LevenbergMarquardtSpace::RelativeErrorTooSmall:
            status_str = "相对误差过小（收敛）";
            break;
        case Eigen::LevenbergMarquardtSpace::RelativeErrorAndReductionTooSmall:
            status_str = "相对误差和减少量均过小（收敛）";
            break;
        case Eigen::LevenbergMarquardtSpace::CosinusTooSmall:
            status_str = "梯度与参数增量夹角过小（收敛）";
            break;
        case Eigen::LevenbergMarquardtSpace::TooManyFunctionEvaluation:
            status_str = "达到最大迭代次数";
            break;
        case Eigen::LevenbergMarquardtSpace::FtolTooSmall:
            status_str = "ftol（误差阈值）过小（收敛）";
            break;
        case Eigen::LevenbergMarquardtSpace::XtolTooSmall:
            status_str = "xtol（参数阈值）过小（收敛）";
            break;
        case Eigen::LevenbergMarquardtSpace::GtolTooSmall:
            status_str = "gtol（梯度阈值）过小（收敛）";
            break;
        case Eigen::LevenbergMarquardtSpace::UserAsked:
            status_str = "用户主动终止（通常不触发）";
            break;
        default:
            status_str = "未知状态";
    }

    // 打印状态
    std::cout << "\nLM优化终止状态：" << status_str << "（状态码：" << status << "）\n";

    // 判断是否成功（1-9为正常收敛状态）
    bool success = (status >= Eigen::LevenbergMarquardtSpace::RelativeReductionTooSmall &&
                    status <= Eigen::LevenbergMarquardtSpace::UserAsked);

    if (!success) {
        std::cerr << "优化失败！请检查输入数据或调整优化参数（如增大max_iter）\n";
    }
    return success;
}

void TelecentricLMOptimizer::getOptimizedParams(double& opt_m, double& opt_dy, double& opt_u0, double& opt_v0, double& opt_theta,
                                                double& opt_k, std::vector<Pose>& opt_poses, double& total_reproj_err) const {
    // 解码优化后的参数
    decodeParams(optimized_params_, opt_m, opt_dy, opt_u0, opt_v0, opt_theta, opt_k, opt_poses);

    // 计算总重投影误差
    Eigen::VectorXd residuals(residual_count_);
    computeResiduals(optimized_params_, residuals);
    total_reproj_err = std::sqrt(residuals.squaredNorm() / (residual_count_ / 2));  // 平均每个点的误差
}
void TelecentricLMOptimizer::setParamFixed(int idx, bool fixed) {
    if (idx >= 0 && idx < param_fixed_.size()) param_fixed_[idx] = fixed;
}

void TelecentricLMOptimizer::setParamScale(int idx, double scale) {
    if (idx >= 0 && (idx < param_scale_.size()) && scale > 0) {
        param_scale_[idx] = scale;
    }
}

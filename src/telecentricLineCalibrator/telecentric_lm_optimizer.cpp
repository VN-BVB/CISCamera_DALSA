#include "telecentric_lm_optimizer.h"

// 构造函数实现
TelecentricLMOptimizer::TelecentricLMOptimizer(const std::vector<std::vector<Eigen::Vector2d>>& all_image_pts,
                                               const std::vector<Eigen::Vector2d>& world_pts, const std::vector<Pose>& init_poses,
                                               double init_m, double init_dx, double init_dy, double init_u0, double init_v0,
                                               double init_theta, double init_k)
    : all_image_pts_(all_image_pts),
      world_pts_(world_pts),
      init_poses_(init_poses),
      init_m_(init_m),
      init_dx_(init_dx),
      init_dy_(init_dy),
      init_u0_(init_u0),
      init_v0_(init_v0),
      init_theta_(init_theta),
      init_k_(init_k),
      m_(init_m),
      dx_(init_dx),
      dy_(init_dy),
      u0_(init_u0),
      v0_(init_v0),
      theta_(init_theta),
      k_(init_k),
      current_poses_(init_poses) {  // 初始化当前外参为初始值
    // 输入合法性检查（参考文档1.82节N≥6的要求）
    assert(all_image_pts.size() == init_poses.size() && "图像点数量与姿态数量不匹配");
    assert(world_pts.size() >= 6 && "世界点数量需≥6（至少6个点才能解算参数）");
    for (const auto& pts : all_image_pts) {
        assert(pts.size() == world_pts.size() && "单姿态图像点与世界点数量不匹配");
    }
}

// 执行LM优化（MATLAB风格：JTJ对角线阻尼 + 旋转向量外参更新）
bool TelecentricLMOptimizer::optimize(int max_iter, double eps_error, double eps_param, double init_lambda) {
    double prev_total_error = computeTotalReprojectionError();
    double lambda = init_lambda;

    for (int iter = 0; iter < max_iter; ++iter) {
        Eigen::MatrixXd J;
        Eigen::VectorXd e;
        buildJacobianAndError(J, e);

        // 求解LM增量方程：(J^T J + λ*diag(diag(JTJ)))Δ = J^T e（MATLAB核心策略）
        Eigen::MatrixXd JtJ = J.transpose() * J;
        Eigen::VectorXd Jte = J.transpose() * e;
        const int total_params = JtJ.rows();

        Eigen::MatrixXd I = Eigen::MatrixXd::Identity(total_params, total_params);
        Eigen::VectorXd delta = (JtJ + lambda * I).ldlt().solve(Jte);
        std::cout << "迭代" << iter + 1 << "：delta范数=" << delta.norm() << std::endl;
        std::cout << "迭代" << iter + 1 << "：全局内参delta（m, dy, u0, v0, theta, k）：" << delta(0) << ", " << delta(1) << ", "
                  << delta(2) << ", " << delta(3) << ", " << delta(4) << ", " << delta(5) << std::endl;

        // 尝试更新参数（外参用旋转向量增量左乘更新）
        saveCurrentParams();
        updateParams(delta);

        // 判断更新是否有效（MATLAB风格阻尼调整：成功/失败分别×0.1/×10）
        double curr_total_error = computeTotalReprojectionError();
        std::cout << "curr_total_error " << curr_total_error << std::endl;
        if (curr_total_error <= prev_total_error && !std::isnan(curr_total_error)) {
            prev_total_error = curr_total_error;
            lambda *= 0.1;  // 减小阻尼（更接近高斯牛顿，加速收敛）
            std::cout << "迭代" << iter + 1 << "更新有效！！！：总重投影误差=" << prev_total_error << "，λ=" << lambda
                      << std::endl;

            // 检查收敛条件
            if (prev_total_error < eps_error || delta.norm() < eps_param) {
                std::cout << "优化收敛！迭代次数=" << iter + 1 << "，最终误差=" << prev_total_error << std::endl;
                return true;
            }
        } else {
            restoreSavedParams();
            lambda *= 1;  // 增大阻尼（更接近梯度下降，保证稳定）
            std::cout << "迭代" << iter + 1 << "：更新无效，恢复参数，λ=" << lambda << std::endl;

            if (lambda > 1e10) {
                std::cerr << "阻尼系数过大，优化终止（可能未收敛）" << std::endl;
                return false;
            }
        }
    }

    std::cout << "达到最大迭代次数，优化终止，最终误差=" << prev_total_error << std::endl;
    printParamChanges();
    return true;
}

// 获取优化结果
void TelecentricLMOptimizer::getOptimizedParams(double& m, double& dy, double& u0, double& v0, double& theta, double& k,
                                                std::vector<Pose>& optimized_poses, double& total_reprojErr) const {
    m = m_;
    dy = dy_;
    u0 = u0_;
    v0 = v0_;
    theta = theta_;
    k = k_;
    optimized_poses = current_poses_;
    total_reprojErr = computeTotalReprojectionError();
}

// 世界点→重投影像素点（核心投影函数）
Eigen::Vector2d TelecentricLMOptimizer::projectWorldToImage(const Eigen::Vector2d& world_pt, const Pose& pose) const {
    // 1. 世界坐标→相机坐标（使用2x2旋转矩阵部分，与computeReprojectionErrorFinal保持一致）
    Eigen::Matrix2d R2 = pose.R.block<2, 2>(0, 0);    // 仅取旋转矩阵前两列
    Eigen::Vector2d t2 = pose.t.head<2>();            // 仅取平面平移
    Eigen::Vector2d img_affine = R2 * world_pt + t2;  // 仿射变换

    // 2. 相机坐标→理想图像坐标（无畸变）
    double x_u = img_affine.x();  // 理想X方向物理坐标（mm）
    double y_u = img_affine.y();  // 理想Y方向物理坐标（mm）

    // 3. 理想图像坐标→实际图像坐标（加径向畸变，文档1.60节式3、4）
    double r_sq = x_u * x_u + (v0_ * dy_) * (v0_ * dy_);  // r² = x_u² + (v0·dy)²
    double delta_x = k_ * x_u * r_sq;                     // X方向畸变分量
    double delta_y = -k_ * v0_ * dy_ * r_sq;              // Y方向畸变分量（注意负号）
    double x_d = x_u + delta_x;                           // 实际X方向物理坐标
    double y_d = y_u + delta_y;                           // 实际Y方向物理坐标

    // 4. 实际图像坐标→重投影像素坐标（含倾斜角θ，文档1.81节式12）
    double hat_u = (m_ / dx_) * x_d - (m_ * tan(theta_)) / dx_ * y_d + u0_;  // u方向标准式
    double hat_v = (1.0 / (dy_ * cos(theta_))) * y_d + (v0_ / m_);           // v方向标准式

    return Eigen::Vector2d(hat_u, hat_v);
}

// 计算单姿态的重投影RMSE
double TelecentricLMOptimizer::computePoseReprojectionError(const std::vector<Eigen::Vector2d>& image_pts,
                                                            const Pose& pose) const {
    double sum_sq_err = 0.0;
    for (size_t i = 0; i < image_pts.size(); ++i) {
        Eigen::Vector2d hat_pt = projectWorldToImage(world_pts_[i], pose);
        sum_sq_err += (image_pts[i] - hat_pt).squaredNorm();
    }
    return sqrt(sum_sq_err / image_pts.size());
}

// 计算总重投影RMSE
double TelecentricLMOptimizer::computeTotalReprojectionError() const {
    double total_sum_sq_err = 0.0;
    int total_pts = 0;
    for (size_t j = 0; j < all_image_pts_.size(); ++j) {
        const auto& image_pts = all_image_pts_[j];
        const auto& pose = current_poses_[j];
        for (size_t i = 0; i < image_pts.size(); ++i) {
            Eigen::Vector2d hat_pt = projectWorldToImage(world_pts_[i], pose);
            total_sum_sq_err += (image_pts[i] - hat_pt).squaredNorm();
            total_pts++;
        }
        const_cast<Pose&>(current_poses_[j]).reprojErr = computePoseReprojectionError(image_pts, pose);
    }
    return sqrt(total_sum_sq_err / total_pts);
}

// 构建雅克比矩阵与误差向量（适配旋转向量外参）
void TelecentricLMOptimizer::buildJacobianAndError(Eigen::MatrixXd& J, Eigen::VectorXd& e) const {
    const int num_poses = all_image_pts_.size();
    const int num_pts_per_pose = world_pts_.size();
    const int total_pts = num_poses * num_pts_per_pose;

    // 参数总数：全局参数（6） + 每个姿态外参（6：3旋转向量+3平移）
    const int total_params = num_global_params + num_poses * num_pose_params;

    J.resize(2 * total_pts, total_params);
    e.resize(2 * total_pts);
    J.setZero();
    e.setZero();

    // 填充J和e
    for (int j = 0; j < num_poses; ++j) {
        const auto& image_pts = all_image_pts_[j];
        const auto& pose = current_poses_[j];

        for (int i = 0; i < num_pts_per_pose; ++i) {
            const auto& world_pt = world_pts_[i];
            const auto& actual_pt = image_pts[i];
            const auto hat_pt = projectWorldToImage(world_pt, pose);

            // 误差向量e = 实际点 - 重投影点
            const int e_idx_u = 2 * (j * num_pts_per_pose + i);
            const int e_idx_v = 2 * (j * num_pts_per_pose + i) + 1;
            e(e_idx_u) = actual_pt.x() - hat_pt.x();
            e(e_idx_v) = actual_pt.y() - hat_pt.y();
            // -------------------------- 关键修改：临时对象中转 --------------------------
            // 1. 创建临时行向量（大小=J的列数，与参数总数一致）
            Eigen::RowVectorXd temp_u(total_params);
            Eigen::RowVectorXd temp_v(total_params);
            temp_u.setZero();  // 初始化为0，避免未赋值的元素干扰
            temp_v.setZero();

            // 2. 调用偏导数函数，传入临时对象（类型匹配：RowVectorXd&）
            computeGlobalParamDerivatives(world_pt, pose, hat_pt, temp_u, temp_v, 0);
            const int pose_param_start = num_global_params + j * num_pose_params;
            computePoseParamDerivatives(world_pt, pose, hat_pt, temp_u, temp_v, pose_param_start);

            // 3. 将临时对象的值赋值给J的对应行（Block类型兼容RowVectorXd）
            J.row(e_idx_u) = temp_u;
            J.row(e_idx_v) = temp_v;
            // --------------------------------------------------------------------------
        }
    }
}

// 误差对全局参数的偏导数（保留原有推导逻辑）
void TelecentricLMOptimizer::computeGlobalParamDerivatives(const Eigen::Vector2d& world_pt, const Pose& pose,
                                                           const Eigen::Vector2d& hat_pt, Eigen::RowVectorXd& J_row_u,
                                                           Eigen::RowVectorXd& J_row_v, int param_start_idx) const {
    // 中间变量（沿用论文简化场景：2×2旋转矩阵+平面平移）
    Eigen::Matrix2d R2 = pose.R.block<2, 2>(0, 0);        // 仅取旋转矩阵前2×2部分
    Eigen::Vector2d t2 = pose.t.head<2>();                // 仅取平面平移
    Eigen::Vector2d Pc = R2 * world_pt + t2;              // 仿射变换得到相机平面坐标
    double x_u = Pc.x(), y_u = Pc.y();                    // 理想图像坐标（未畸变）
    double r_sq = x_u * x_u + (v0_ * dy_) * (v0_ * dy_);  // 畸变计算项r²
    double x_d = x_u + k_ * x_u * r_sq;                   // 含畸变的x坐标（x_d）
    double y_d = y_u - k_ * v0_ * dy_ * r_sq;             // 含畸变的y坐标（y_d）

    // -------------------------- u方向偏导数（∂hat_u/∂param）--------------------------
    // 1. 对m的偏导数（∂hat_u/∂m）
    const double d_hat_u__dm = (x_d - tan(theta_) * y_d) / dx_;
    // 2. 对dy的偏导数（∂hat_u/∂dy）：含畸变间接影响（修正项）
    double dr_sq__ddy = 2 * (v0_ * dy_) * v0_;                      // r²对dy的偏导数
    double d_delta_y__ddy = -k_ * v0_ * (r_sq + dy_ * dr_sq__ddy);  // δy对dy的偏导数
    double d_y_d__ddy = d_delta_y__ddy;                             // y_d对dy的偏导数
    const double d_hat_u__ddy = -(m_ * tan(theta_) / dx_) * d_y_d__ddy;
    // 3. 对u0的偏导数（∂hat_u/∂u0）
    const double d_hat_u__du0 = 1.0;
    // 4. 对v0的偏导数（∂hat_u/∂v0）：简化场景下忽略（近似0）
    const double d_hat_u__dv0 = 0.0;
    // 5. 对theta的偏导数（∂hat_u/∂theta）
    const double d_hat_u__dtheta = -(m_ / (dx_ * cos(theta_) * cos(theta_))) * y_d;
    // 6. 对k的偏导数（∂hat_u/∂k）
    const double d_hat_u__dk = (m_ / dx_) * x_u * r_sq + (m_ * tan(theta_) / dx_) * v0_ * dy_ * r_sq;

    // -------------------------- v方向偏导数（核心修改：匹配公式(12)的2项内参）--------------------------
    // 修改后的v方向公式：hat_v = (1/(dy·cosθ)) * y_d + (v0/m)
    // 1. 对m的偏导数（∂hat_v/∂m）：新增v0/m对m的偏导
    const double d_hat_v__dm = -(v0_ / (m_ * m_));  // 仅新增这一项（v0/m对m的偏导为 -v0/m²）
    // 2. 对dy的偏导数（∂hat_v/∂dy）：修改为1/(dy·cosθ)的偏导
    const double d_hat_v__ddy = -(1.0 * y_d) / (dy_ * dy_ * cos(theta_));  // 原m/dy→1/(dy·cosθ)，偏导同步修改
    // 3. 对u0的偏导数（∂hat_v/∂u0）：v公式不含u0，为0（不变）
    const double d_hat_v__du0 = 0.0;
    // 4. 对v0的偏导数（∂hat_v/∂v0）：修改为v0/m的偏导
    const double d_hat_v__dv0 = 1.0 / m_;  // 原v0→v0/m，偏导从1.0改为1/m
    // 5. 对theta的偏导数（∂hat_v/∂theta）：新增1/(dy·cosθ)对theta的偏导
    const double d_hat_v__dtheta = (1.0 * y_d * tan(theta_)) / (dy_ * cos(theta_));  // 1/(dy·cosθ)对theta偏导为 tanθ/(dy·cosθ)
    // 6. 对k的偏导数（∂hat_v/∂k）：通过y_d传递（原逻辑不变，仅适配新的v公式）
    const double d_hat_v__dk =
        (1.0 / (dy_ * cos(theta_))) * (-v0_ * dy_ * r_sq);  // y_d对k的偏导仍为 -v0·dy·r²，仅外层系数改为1/(dy·cosθ)

    // -------------------------- 填充雅克比矩阵（仅v方向偏导适配修改，其他不变）--------------------------
    // u方向误差偏导数（完全保留原逻辑，无修改）
    J_row_u(param_start_idx + 0) = -d_hat_u__dm;      // m
    J_row_u(param_start_idx + 1) = -d_hat_u__ddy;     // dy
    J_row_u(param_start_idx + 2) = -d_hat_u__du0;     // u0
    J_row_u(param_start_idx + 3) = -d_hat_u__dv0;     // v0
    J_row_u(param_start_idx + 4) = -d_hat_u__dtheta;  // theta
    J_row_u(param_start_idx + 5) = -d_hat_u__dk;      // k

    // v方向误差偏导数（仅适配修改后的偏导，其他不变）
    J_row_v(param_start_idx + 0) = -d_hat_v__dm;      // m（新偏导）
    J_row_v(param_start_idx + 1) = -d_hat_v__ddy;     // dy（新偏导）
    J_row_v(param_start_idx + 2) = -d_hat_v__du0;     // u0（不变）
    J_row_v(param_start_idx + 3) = -d_hat_v__dv0;     // v0（新偏导）
    J_row_v(param_start_idx + 4) = -d_hat_v__dtheta;  // theta（新偏导）
    J_row_v(param_start_idx + 5) = -d_hat_v__dk;      // k（适配新v公式，系数修改）
}

// 误差对姿态外参的偏导数（核心适配论文v方向内参：hat_v = (1/(dy·cosθ))·y_d + v0/m）
void TelecentricLMOptimizer::computePoseParamDerivatives(const Eigen::Vector2d& world_pt, const Pose& pose,
                                                         const Eigen::Vector2d& hat_pt, Eigen::RowVectorXd& J_row_u,
                                                         Eigen::RowVectorXd& J_row_v, int param_start_idx) const {
    // -------------------------- 外参参数化：旋转向量（r1,r2,r3）+ 平移（tx, ty, tz）--------------------------
    // 旋转向量 = Rodrigues向量（与MATLAB一致），旋转矩阵 R = rotVecToMat(rvec)
    Eigen::Vector3d rvec = rotMatToVec(pose.R);  // 从当前旋转矩阵提取旋转向量
    const double tx = pose.t.x(), ty = pose.t.y(), tz = pose.t.z();

    // -------------------------- 中间变量计算（与论文内参模型对齐）--------------------------
    // 世界点（平面标定板，Z=0）：Pw = [Xw, Yw, 0]^T（论文简化场景）
    Eigen::Vector3d Pw(world_pt.x(), world_pt.y(), 0.0);
    double Xw = Pw.x(), Yw = Pw.y();

    // 相机坐标系坐标：Pc = R * Pw + t → [x_u, y_u, z_u]^T（x_u, y_u用于成像，论文定义一致）
    Eigen::Vector3d Pc = pose.R * Pw + pose.t;
    double x_u = Pc.x();  // 理想X坐标（无畸变，论文内参输入项）
    double y_u = Pc.y();  // 理想Y坐标（无畸变，论文内参输入项）

    // 畸变计算项：r² = x_u² + (v0*dy)²（论文简化径向畸变模型，保留原逻辑）
    double r_sq = x_u * x_u + (v0_ * dy_) * (v0_ * dy_);

    // 含畸变的坐标（论文公式，与原有实现一致）：
    // x_d = x_u + k * x_u * r² （X方向畸变）
    // y_d = y_u - k * v0 * dy * r² （Y方向畸变）
    double x_d = x_u + k_ * x_u * r_sq;
    double y_d = y_u - k_ * v0_ * dy_ * r_sq;

    // -------------------------- 1. 对平移向量(tx, ty, tz)的偏导数（适配论文v方向内参）--------------------------
    // 1.1 u方向偏导（与论文u方向内参一致，无修改）
    // 论文u方向公式：hat_u = (m/dx)*(x_d - tanθ·y_d) + u0，偏导逻辑不变
    const double d_xd__dtx = 1 + k_ * (r_sq + 2 * x_u * x_u);             // ∂x_d/∂tx（x_u对tx偏导=1，论文假设）
    const double d_yd__dty = 1 - k_ * v0_ * dy_ * 2 * (v0_ * dy_) * dy_;  // ∂y_d/∂ty（y_u对ty偏导=1，论文假设）
    const double d_hat_u__dtx = (m_ / dx_) * d_xd__dtx;                   // ∂hat_u/∂tx（论文u方向系数m/dx，不变）
    const double d_hat_u__dty = -(m_ * tan(theta_) / dx_) * d_yd__dty;    // ∂hat_u/∂ty（论文u方向倾斜项，不变）
    const double d_hat_u__dtz = 0.0;                                      // 远心镜头：z_u不影响x_u，论文简化假设

    // 1.2 v方向偏导（核心适配论文内参：hat_v = (1/(dy·cosθ))·y_d + v0/m）
    // 关键调整：v方向系数从“m/(dy·cosθ)”改为“1/(dy·cosθ)”，与论文一致
    const double d_hat_v__dtx = 0.0;                                      // x_d不影响v方向，论文内参无x_d项
    const double d_hat_v__dty = (1.0 / (dy_ * cos(theta_))) * d_yd__dty;  // ∂hat_v/∂ty（论文v方向系数1/(dy·cosθ)）
    const double d_hat_v__dtz = 0.0;                                      // 远心镜头：z_u不影响y_d，论文简化假设

    // -------------------------- 2. 对旋转向量(r1, r2, r3)的偏导数（适配论文v方向内参）--------------------------
    // 2.1 旋转向量对旋转矩阵的偏导数（dR/dr1, dR/dr2, dR/dr3）（与MATLAB逻辑一致，论文外参推导兼容）
    Eigen::Matrix3d dRdr1, dRdr2, dRdr3;
    computeDRdr(rvec, dRdr1, dRdr2, dRdr3);  // 调用旋转向量偏导计算函数

    // 2.2 Pc对旋转向量的偏导数：∂Pc/∂r = (∂R/∂r) * Pw（t与旋转无关，论文外参假设）
    Eigen::Vector3d dPc_dr1 = dRdr1 * Pw;  // ∂Pc/∂r1（x_u/y_u对r1的偏导载体）
    Eigen::Vector3d dPc_dr2 = dRdr2 * Pw;  // ∂Pc/∂r2（x_u/y_u对r2的偏导载体）
    Eigen::Vector3d dPc_dr3 = dRdr3 * Pw;  // ∂Pc/∂r3（x_u/y_u对r3的偏导载体）

    // 2.3 x_u, y_u对旋转向量的偏导数（取Pc的X、Y分量，论文内参输入项的偏导）
    double d_xu_dr1 = dPc_dr1.x();  // ∂x_u/∂r1
    double d_yu_dr1 = dPc_dr1.y();  // ∂y_u/∂r1
    double d_xu_dr2 = dPc_dr2.x();  // ∂x_u/∂r2
    double d_yu_dr2 = dPc_dr2.y();  // ∂y_u/∂r2
    double d_xu_dr3 = dPc_dr3.x();  // ∂x_u/∂r3
    double d_yu_dr3 = dPc_dr3.y();  // ∂y_u/∂r3

    // 2.4 r²对旋转向量的偏导数：∂r²/∂r = 2x_u·∂x_u/∂r（第二项与旋转无关，论文畸变推导一致）
    double d_r_sq_dr1 = 2 * x_u * d_xu_dr1;  // ∂r²/∂r1
    double d_r_sq_dr2 = 2 * x_u * d_xu_dr2;  // ∂r²/∂r2
    double d_r_sq_dr3 = 2 * x_u * d_xu_dr3;  // ∂r²/∂r3

    // 2.5 含畸变坐标(x_d, y_d)对旋转向量的偏导数（链式法则，论文畸变传递逻辑）
    double d_xd_dr1 = d_xu_dr1 + k_ * (d_xu_dr1 * r_sq + x_u * d_r_sq_dr1);  // ∂x_d/∂r1
    double d_xd_dr2 = d_xu_dr2 + k_ * (d_xu_dr2 * r_sq + x_u * d_r_sq_dr2);  // ∂x_d/∂r2
    double d_xd_dr3 = d_xu_dr3 + k_ * (d_xu_dr3 * r_sq + x_u * d_r_sq_dr3);  // ∂x_d/∂r3
    double d_yd_dr1 = d_yu_dr1 - k_ * v0_ * dy_ * d_r_sq_dr1;                // ∂y_d/∂r1
    double d_yd_dr2 = d_yu_dr2 - k_ * v0_ * dy_ * d_r_sq_dr2;                // ∂y_d/∂r2
    double d_yd_dr3 = d_yu_dr3 - k_ * v0_ * dy_ * d_r_sq_dr3;                // ∂y_d/∂r3

    // 2.6 重投影坐标对旋转向量的偏导数（适配论文v方向内参）
    // u方向偏导（与论文u方向内参一致，无修改）
    const double d_hat_u_dr1 = (m_ / dx_) * (d_xd_dr1 - tan(theta_) * d_yd_dr1);  // ∂hat_u/∂r1
    const double d_hat_u_dr2 = (m_ / dx_) * (d_xd_dr2 - tan(theta_) * d_yd_dr2);  // ∂hat_u/∂r2
    const double d_hat_u_dr3 = (m_ / dx_) * (d_xd_dr3 - tan(theta_) * d_yd_dr3);  // ∂hat_u/∂r3

    // v方向偏导（核心适配论文内参：系数改为1/(dy·cosθ)）
    const double d_hat_v_dr1 = (1.0 / (dy_ * cos(theta_))) * d_yd_dr1;  // ∂hat_v/∂r1（论文v方向系数）
    const double d_hat_v_dr2 = (1.0 / (dy_ * cos(theta_))) * d_yd_dr2;  // ∂hat_v/∂r2（论文v方向系数）
    const double d_hat_v_dr3 = (1.0 / (dy_ * cos(theta_))) * d_yd_dr3;  // ∂hat_v/∂r3（论文v方向系数）

    // -------------------------- 3. 填充雅克比矩阵（误差偏导数 = -模型偏导数，论文误差定义一致）--------------------------
    // 列顺序：[r1, r2, r3, tx, ty, tz]（旋转向量在前，与MATLAB参数化对齐）
    J_row_u(param_start_idx + 0) = -d_hat_u_dr1;   // e_u对r1的偏导数
    J_row_u(param_start_idx + 1) = -d_hat_u_dr2;   // e_u对r2的偏导数
    J_row_u(param_start_idx + 2) = -d_hat_u_dr3;   // e_u对r3的偏导数
    J_row_u(param_start_idx + 3) = -d_hat_u__dtx;  // e_u对tx的偏导数
    J_row_u(param_start_idx + 4) = -d_hat_u__dty;  // e_u对ty的偏导数
    J_row_u(param_start_idx + 5) = -d_hat_u__dtz;  // e_u对tz的偏导数

    J_row_v(param_start_idx + 0) = -d_hat_v_dr1;   // e_v对r1的偏导数（适配论文v方向系数）
    J_row_v(param_start_idx + 1) = -d_hat_v_dr2;   // e_v对r2的偏导数（适配论文v方向系数）
    J_row_v(param_start_idx + 2) = -d_hat_v_dr3;   // e_v对r3的偏导数（适配论文v方向系数）
    J_row_v(param_start_idx + 3) = -d_hat_v__dtx;  // e_v对tx的偏导数
    J_row_v(param_start_idx + 4) = -d_hat_v__dty;  // e_v对ty的偏导数（适配论文v方向系数）
    J_row_v(param_start_idx + 5) = -d_hat_v__dtz;  // e_v对tz的偏导数
}

// 保存当前参数（含旋转向量外参）
void TelecentricLMOptimizer::saveCurrentParams() {
    saved_m_ = m_;
    saved_dy_ = dy_;
    saved_u0_ = u0_;
    saved_v0_ = v0_;
    saved_theta_ = theta_;
    saved_k_ = k_;
    saved_poses_ = current_poses_;  // 保存当前姿态（含旋转矩阵）
}

// 恢复保存的参数
void TelecentricLMOptimizer::restoreSavedParams() {
    m_ = saved_m_;
    dy_ = saved_dy_;
    u0_ = saved_u0_;
    v0_ = saved_v0_;
    theta_ = saved_theta_;
    k_ = saved_k_;
    current_poses_ = saved_poses_;  // 恢复姿态
}

// 更新参数（外参用旋转向量增量左乘，MATLAB风格）
void TelecentricLMOptimizer::updateParams(const Eigen::VectorXd& delta) {
    const int num_poses = current_poses_.size();

    // -------------------------- 1. 更新全局参数（与原逻辑一致）--------------------------
    m_ += delta(0);
    dy_ += delta(1);
    // u0_ += delta(2);
    // v0_ += delta(3);
    u0_ = saved_u0_;
    v0_ = saved_v0_;
    theta_ += delta(4);
    k_ += delta(5);
    // -------------------------- 2. 打印全局参数变化（复用 saveCurrentParams 保存的旧值）--------------------------
    std::cout << "\n=== 全局参数更新前后对比（基于 saveCurrentParams）===" << std::endl;
    std::cout << "m: " << saved_m_ << " → " << m_ << " (变化量: " << m_ - saved_m_ << ")" << std::endl;
    std::cout << "dy: " << saved_dy_ << " → " << dy_ << " (变化量: " << dy_ - saved_dy_ << ")" << std::endl;
    std::cout << "u0: " << saved_u0_ << " → " << u0_ << " (变化量: " << u0_ - saved_u0_ << ")" << std::endl;
    std::cout << "v0: " << saved_v0_ << " → " << v0_ << " (变化量: " << v0_ - saved_v0_ << ")" << std::endl;
    std::cout << "theta: " << saved_theta_ << " rad → " << theta_ << " rad (变化量: " << theta_ - saved_theta_ << " rad)"
              << std::endl;
    std::cout << "k: " << saved_k_ << " → " << k_ << " (变化量: " << k_ - saved_k_ << ")" << std::endl;

    /*    // 全局参数动态边界：初始值 ± 合理波动幅度（按参数类型设不同幅度）
        const double m_fluct = 0.1 * init_m_;    // m：初始值的±10%（放大倍率波动不宜过大）
        const double dy_fluct = 0.2 * init_dy_;  // dy：初始值的±20%（像素尺寸测量有小误差）
        const double u0_fluct = 10.0;            // u0：
        const double v0_fluct = 10.0;            // v0：
        const double theta_fluct = M_PI / 36;    // theta：±M_PI / 36--5°（倾斜角初始值附近小范围调整）
        const double k_fluct = 0.1;              // k：±（远心镜头畸变系数接近0，波动极小）

        // 应用动态边界
        m_ = std::max(init_m_ - m_fluct, std::min(init_m_ + m_fluct, m_));                          // m：初始值±10%
        dy_ = std::max(init_dy_ - dy_fluct, std::min(init_dy_ + dy_fluct, dy_));                    // dy：初始值±20%
        u0_ = std::max(init_u0_ - u0_fluct, std::min(init_u0_ + u0_fluct, u0_));                    // u0：
        v0_ = std::max(init_v0_ - v0_fluct, std::min(init_v0_ + v0_fluct, v0_));                    // v0：
        theta_ = std::max(init_theta_ - theta_fluct, std::min(init_theta_ + theta_fluct, theta_));  // theta：±5°
        k_ = std::max(init_k_ - k_fluct, std::min(init_k_ + k_fluct, k_));  */                        // k：±2e-4

    // -------------------------- 2. 更新外参（MATLAB风格：旋转向量增量左乘）--------------------------
    for (int j = 0; j < num_poses; ++j) {
        const int pose_param_start = num_global_params + j * num_pose_params;
        // 提取旋转向量增量（r1, r2, r3）和平移增量（tx, ty, tz）
        double dr1 = delta(pose_param_start + 0);
        double dr2 = delta(pose_param_start + 1);
        double dr3 = delta(pose_param_start + 2);
        double dtx = delta(pose_param_start + 3);
        double dty = delta(pose_param_start + 4);
        double dtz = delta(pose_param_start + 5);

        // 2.1 计算旋转向量增量对应的旋转矩阵（Delta_R）
        Eigen::Vector3d drvec(dr1, dr2, dr3);
        Eigen::Matrix3d Delta_R = rotVecToMat(drvec);  // 增量旋转矩阵

        // 2.2 旋转矩阵更新：R_new = Delta_R * R_old（MATLAB左乘逻辑）
        Eigen::Matrix3d R_old = current_poses_[j].R;
        Eigen::Matrix3d R_new = Delta_R * R_old;

        // 2.3 平移向量更新：t_new = t_old + delta_t（与原逻辑一致）
        Eigen::Vector3d t_old = current_poses_[j].t;
        Eigen::Vector3d t_new = t_old + Eigen::Vector3d(dtx, dty, dtz);

        // // 打印外参变化（对比 saved_poses_ 中的旧姿态）
        // std::cout << "\n=== 姿态 " << j + 1 << " 更新前后对比 ===" << std::endl;
        // // 旋转变化：旧旋转向量（saved_poses_）→ 新旋转向量（current_poses_）
        // Eigen::Vector3d old_rvec = rotMatToVec(saved_poses_[j].R);
        // Eigen::Vector3d new_rvec = rotMatToVec(R_new);
        // std::cout << "旋转向量: [" << old_rvec(0) << ", " << old_rvec(1) << ", " << old_rvec(2) << "] → [" << new_rvec(0) << ",
        // "
        //           << new_rvec(1) << ", " << new_rvec(2) << "] (变化量: [" << new_rvec(0) - old_rvec(0) << ", "
        //           << new_rvec(1) - old_rvec(1) << ", " << new_rvec(2) - old_rvec(2) << "])" << std::endl;
        // // 平移变化：旧平移（saved_poses_）→ 新平移（t_new）
        // Eigen::Vector3d old_t = saved_poses_[j].t;
        // std::cout << "平移向量: [" << old_t(0) << ", " << old_t(1) << ", " << old_t(2) << "] → [" << t_new(0) << ", " <<
        // t_new(1)
        //           << ", " << t_new(2) << "] (变化量: [" << t_new(0) - old_t(0) << ", " << t_new(1) - old_t(1) << ", "
        //           << t_new(2) - old_t(2) << "])" << std::endl;

        // // 2.4 外参动态边界（可选，按初始姿态约束）
        // const Pose& init_pose = init_poses_[j];
        // double init_tx = init_pose.t.x(), init_ty = init_pose.t.y(), init_tz = init_pose.t.z();
        // const double tx_ty_fluct = 10.0;  // 平移波动±10mm
        // new_tx = std::max(init_tx - tx_ty_fluct, std::min(init_tx + tx_ty_fluct, t_new.x()));
        // new_ty = std::max(init_ty - tx_ty_fluct, std::min(init_ty + tx_ty_fluct, t_new.y()));
        // new_tz = t_new.z();  // 远心Z向可放宽约束

        // 2.5 赋值更新后的外参
        current_poses_[j].R = R_new;
        current_poses_[j].t = t_new;
    }
}

// 打印参数优化前后的变化（适配旋转向量外参）
void TelecentricLMOptimizer::printParamChanges() {
    std::cout << "\n===================== 参数优化前后变化对比 =====================" << std::endl;

    // 1. 全局内参变化（6个参数）
    std::cout << "\n【1. 全局内参变化】" << std::endl;
    // 定义参数名称、初始值、当前值、单位（用于格式化输出）
    struct GlobalParam {
        std::string name;
        double init_val;
        double curr_val;
        std::string unit;
    };
    std::vector<GlobalParam> global_params = {
        {"放大倍率 m",        init_m_,                  m_,                  ""            },
        {"垂直像素尺寸 dy", init_dy_,                 dy_,                 " (mm/像素)"},
        {"主点x坐标 u0",      init_u0_,                 u0_,                 " (像素)"   },
        {"主点y坐标 v0",      init_v0_,                 v0_,                 " (像素)"   },
        {"倾斜角 theta",       init_theta_ * 180 / M_PI, theta_ * 180 / M_PI, " (°)"       }, // 弧度转角度，更直观
        {"畸变系数 k",        init_k_,                  k_,                  ""            }
    };
    // 遍历打印每个内参的变化
    for (const auto& param : global_params) {
        double abs_change = param.curr_val - param.init_val;      // 绝对变化量
        double rel_change = (abs_change / param.init_val) * 100;  // 相对变化率（百分比）
        std::cout << std::fixed << std::setprecision(6) << param.name << "：" << std::endl
                  << "  初始值 = " << param.init_val << param.unit << std::endl
                  << "  当前值 = " << param.curr_val << param.unit << std::endl
                  << "  绝对变化 = " << abs_change << param.unit << std::endl
                  << "  相对变化 = " << rel_change << "%" << std::endl;
    }

    // 2. 外参变化（每个姿态6个参数：旋转向量+平移）
    std::cout << "\n【2. 外参变化（每个姿态）】" << std::endl;
    for (int j = 0; j < current_poses_.size(); ++j) {
        const Pose& init_pose = init_poses_[j];
        const Pose& curr_pose = current_poses_[j];
        std::cout << "\n姿态 " << j + 1 << "：" << std::endl;

        // 2.1 旋转向量变化（旋转矩阵→旋转向量，与MATLAB参数化一致）
        Eigen::Vector3d init_rvec = rotMatToVec(init_pose.R);  // 初始旋转向量
        Eigen::Vector3d curr_rvec = rotMatToVec(curr_pose.R);  // 当前旋转向量

        // 2.2 平移向量变化
        Eigen::Vector3d init_t = init_pose.t;
        Eigen::Vector3d curr_t = curr_pose.t;

        // 打印旋转向量变化（单位：弧度，可转角度更直观）
        std::cout << "  旋转向量（Rodrigues，单位：弧度）：" << std::endl
                  << "    r1：初始=" << init_rvec(0) << "，当前=" << curr_rvec(0) << "，变化=" << curr_rvec(0) - init_rvec(0)
                  << std::endl
                  << "    r2：初始=" << init_rvec(1) << "，当前=" << curr_rvec(1) << "，变化=" << curr_rvec(1) - init_rvec(1)
                  << std::endl
                  << "    r3：初始=" << init_rvec(2) << "，当前=" << curr_rvec(2) << "，变化=" << curr_rvec(2) - init_rvec(2)
                  << std::endl;
        // 打印平移变化
        std::cout << "  平移向量（单位：mm）：" << std::endl
                  << "    tx：初始=" << init_t.x() << "，当前=" << curr_t.x() << "，变化=" << curr_t.x() - init_t.x() << std::endl
                  << "    ty：初始=" << init_t.y() << "，当前=" << curr_t.y() << "，变化=" << curr_t.y() - init_t.y() << std::endl
                  << "    tz：初始=" << init_t.z() << "，当前=" << curr_t.z() << "，变化=" << curr_t.z() - init_t.z()
                  << std::endl;
    }

    std::cout << "\n==============================================================" << std::endl;
}

// =========================================================
// MATLAB风格外参更新辅助函数（旋转向量相关）
// =========================================================
// 1. 旋转矩阵→旋转向量（Rodrigues公式，与MATLAB的rodrigues函数完全一致）
Eigen::Vector3d TelecentricLMOptimizer::rotMatToVec(const Eigen::Matrix3d& R) const {
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

// 2. 旋转向量→旋转矩阵（Rodrigues公式，与MATLAB一致）
Eigen::Matrix3d TelecentricLMOptimizer::rotVecToMat(const Eigen::Vector3d& rvec) const {
    cv::Mat rvec_cv(3, 1, CV_64F);
    cv::Mat R_cv;
    // Eigen向量转OpenCV矩阵
    rvec_cv.at<double>(0) = rvec(0);
    rvec_cv.at<double>(1) = rvec(1);
    rvec_cv.at<double>(2) = rvec(2);
    // Rodrigues变换（旋转向量→旋转矩阵）
    cv::Rodrigues(rvec_cv, R_cv);
    // OpenCV矩阵转Eigen矩阵
    Eigen::Matrix3d R;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) R(i, j) = R_cv.at<double>(i, j);
    return R;
}

// 3. 计算旋转向量对旋转矩阵的偏导数（dR/dr1, dR/dr2, dR/dr3），适配MATLAB梯度计算
void TelecentricLMOptimizer::computeDRdr(const Eigen::Vector3d& rvec, Eigen::Matrix3d& dRdr1, Eigen::Matrix3d& dRdr2,
                                         Eigen::Matrix3d& dRdr3) const {
    double r = rvec.norm();
    if (r < 1e-12) {
        // 旋转角接近0时的近似（避免除零，与MATLAB初始化一致）
        dRdr1 << 0, 0, 0, 0, 0, -1, 0, 1, 0;  // 对应x轴旋转的反对称矩阵
        dRdr2 << 0, 0, 1, 0, 0, 0, -1, 0, 0;  // 对应y轴旋转的反对称矩阵
        dRdr3 << 0, -1, 0, 1, 0, 0, 0, 0, 0;  // 对应z轴旋转的反对称矩阵
        return;
    }

    // 旋转向量单位化
    Eigen::Vector3d rhat = rvec / r;
    double c = cos(r);
    double s = sin(r);

    // 旋转向量的反对称矩阵
    Eigen::Matrix3d rhat_skew;
    rhat_skew << 0, -rhat(2), rhat(1), rhat(2), 0, -rhat(0), -rhat(1), rhat(0), 0;

    // 偏导数公式（基于Rodrigues推导，与MATLAB梯度计算逻辑一致）
    Eigen::Matrix3d term1 = (s / r) * Eigen::Matrix3d::Identity();
    Eigen::Matrix3d term2 = (1 - c) / r * rhat_skew;
    Eigen::Matrix3d term3 = (r - s) / r * rhat * rhat.transpose();

    // 分别计算对r1, r2, r3的偏导数
    dRdr1 = term1 * rhat(0) + term2 * (-rhat_skew(0, 1) * rhat(2) + rhat_skew(0, 2) * rhat(1)) + term3 * rhat(0);
    dRdr2 = term1 * rhat(1) + term2 * (rhat_skew(1, 0) * rhat(2) - rhat_skew(1, 2) * rhat(0)) + term3 * rhat(1);
    dRdr3 = term1 * rhat(2) + term2 * (-rhat_skew(2, 0) * rhat(1) + rhat_skew(2, 1) * rhat(0)) + term3 * rhat(2);
}

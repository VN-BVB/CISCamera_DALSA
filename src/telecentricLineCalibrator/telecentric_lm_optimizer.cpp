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

// 执行LM优化
bool TelecentricLMOptimizer::optimize(int max_iter, double eps_error, double eps_param, double init_lambda) {
    double prev_total_error = computeTotalReprojectionError();
    double lambda = init_lambda;

    for (int iter = 0; iter < max_iter; ++iter) {
        Eigen::MatrixXd J;
        Eigen::VectorXd e;
        buildJacobianAndError(J, e);

        // 求解LM增量方程：(J^T J + λI)Δ = J^T e
        Eigen::MatrixXd JtJ = J.transpose() * J;
        Eigen::MatrixXd I = Eigen::MatrixXd::Identity(JtJ.rows(), JtJ.cols());
        Eigen::VectorXd Jte = J.transpose() * e;
        // 分配阻尼
        const int total_params = JtJ.rows();
        Eigen::VectorXd param_weights(total_params);

        // 1.1 全局内参权重（按需求调整，例：m/dy需稳定，权重设1.0；u0/v0可稍灵活，设0.8）
        param_weights(0) = 1.0;  // m（放大倍率）
        param_weights(1) = 1.0;  // dy（像素尺寸）
        param_weights(2) = 1.0;  // u0（主点x）
        param_weights(3) = 1.0;  // v0（主点y）
        param_weights(4) = 1.0;  // theta（倾斜角）
        param_weights(5) = 1.0;  // k（畸变系数）

        // 1.2 外参权重（每个姿态6个参数，例：旋转角敏感，权重0.4；平移稍灵活，设0.6）
        for (int j = 0; j < current_poses_.size(); ++j) {
            int pose_start = num_global_params + j * num_pose_params;
            param_weights(pose_start + 0) = 1.0;  // rx（旋转角，敏感，低阻尼避免过度约束）
            param_weights(pose_start + 1) = 1.0;  // ry（同上）
            param_weights(pose_start + 2) = 1.0;  // rz（平面旋转，稍灵活）
            param_weights(pose_start + 3) = 1.0;  // tx（平移x，灵活）
            param_weights(pose_start + 4) = 1.0;  // ty（平移y，灵活）
            param_weights(pose_start + 5) = 1.0;  // tz（远心镜头不敏感，极低阻尼）
        }

        // 2. 构造对角加权矩阵，替换原有单位矩阵I
        Eigen::MatrixXd W = param_weights.asDiagonal();  // 每个参数独立权重的对角矩阵

        // 3. 求解LM增量方程：(J^T J + λ*W)Δ = J^T e（差异化阻尼生效）
        Eigen::VectorXd delta = (JtJ + lambda * W).ldlt().solve(Jte);

        // 尝试更新参数
        saveCurrentParams();
        updateParams(delta);

        // 判断更新是否有效
        double curr_total_error = computeTotalReprojectionError();
        if (curr_total_error < prev_total_error && !std::isnan(curr_total_error)) {
            prev_total_error = curr_total_error;
            lambda *= 0.1;  // 减小阻尼（更接近高斯牛顿）
            std::cout << "迭代" << iter + 1 << "：总重投影误差=" << prev_total_error << "，λ=" << lambda << std::endl;

            // 检查收敛条件
            if (prev_total_error < eps_error || delta.norm() < eps_param) {
                std::cout << "优化收敛！迭代次数=" << iter + 1 << "，最终误差=" << prev_total_error << std::endl;
                return true;
            }
        } else {
            restoreSavedParams();
            lambda *= 2;  // 增大阻尼（更接近梯度下降）
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
    // TODO：若修改过v方向投影公式（如乘m），需在此处同步修改hat_v的计算
    double hat_u = (m_ / dx_) * x_d - (m_ * tan(theta_)) / dx_ * y_d + u0_;  // u方向标准式
    double hat_v = (m_ / (dy_ * cos(theta_))) * y_d + (v0_ / 1.0);           // v方向标准式

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

// 构建雅克比矩阵与误差向量
void TelecentricLMOptimizer::buildJacobianAndError(Eigen::MatrixXd& J, Eigen::VectorXd& e) const {
    const int num_poses = all_image_pts_.size();
    const int num_pts_per_pose = world_pts_.size();
    const int total_pts = num_poses * num_pts_per_pose;

    // 参数总数：全局参数（6） + 每个姿态外参（6）
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

// 误差对全局参数的偏导数
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

    // -------------------------- v方向偏导数（∂hat_v/∂param）：按你的修改推导--------------------------
    // 修改后的v方向公式：hat_v = (m / (dy * cos(theta_))) * y_d + v0_
    // 1. 对m的偏导数（∂hat_v/∂m）
    const double d_hat_v__dm = y_d / (dy_ * cos(theta_));
    // 2. 对dy的偏导数（∂hat_v/∂dy）
    const double d_hat_v__ddy = -(m_ * y_d) / (dy_ * dy_ * cos(theta_));  // 注意负号（1/dy求导）
    // 3. 对u0的偏导数（∂hat_v/∂u0）：v公式不含u0，为0
    const double d_hat_v__du0 = 0.0;
    // 4. 对v0的偏导数（∂hat_v/∂v0）：v0为常数项，导数为1
    const double d_hat_v__dv0 = 1.0;
    // 5. 对theta的偏导数（∂hat_v/∂theta）：利用d(1/cosθ)/dθ = tanθ/cosθ
    const double d_hat_v__dtheta = (m_ * y_d * tan(theta_)) / (dy_ * cos(theta_));
    // 6. 对k的偏导数（∂hat_v/∂k）：通过y_d传递（y_d含k）
    const double d_hat_v__dk = (m_ / (dy_ * cos(theta_))) * (-v0_ * dy_ * r_sq);  // y_d对k的偏导数为 -v0*dy*r²

    // -------------------------- 填充雅克比矩阵（误差偏导数 = -模型偏导数）--------------------------
    // u方向误差偏导数（∂e_u/∂param = -∂hat_u/∂param）
    J_row_u(param_start_idx + 0) = -d_hat_u__dm;      // m
    J_row_u(param_start_idx + 1) = -d_hat_u__ddy;     // dy
    J_row_u(param_start_idx + 2) = -d_hat_u__du0;     // u0
    J_row_u(param_start_idx + 3) = -d_hat_u__dv0;     // v0
    J_row_u(param_start_idx + 4) = -d_hat_u__dtheta;  // theta
    J_row_u(param_start_idx + 5) = -d_hat_u__dk;      // k

    // v方向误差偏导数（∂e_v/∂param = -∂hat_v/∂param）
    J_row_v(param_start_idx + 0) = -d_hat_v__dm;      // m
    J_row_v(param_start_idx + 1) = -d_hat_v__ddy;     // dy
    J_row_v(param_start_idx + 2) = -d_hat_v__du0;     // u0
    J_row_v(param_start_idx + 3) = -d_hat_v__dv0;     // v0
    J_row_v(param_start_idx + 4) = -d_hat_v__dtheta;  // theta
    J_row_v(param_start_idx + 5) = -d_hat_v__dk;      // k
}

// 误差对姿态外参的偏导数（核心需修改部分）
void TelecentricLMOptimizer::computePoseParamDerivatives(const Eigen::Vector2d& world_pt, const Pose& pose,
                                                         const Eigen::Vector2d& hat_pt, Eigen::RowVectorXd& J_row_u,
                                                         Eigen::RowVectorXd& J_row_v, int param_start_idx) const {
    // -------------------------- 外参参数化：ZYX欧拉角（rx, ry, rz）+ 平移（tx, ty, tz）--------------------------
    // 旋转矩阵R = Rz(rz) * Ry(ry) * Rx(rx)（ZYX顺序，先绕X轴，再Y轴，最后Z轴）
    double rx = 0.0, ry = 0.0, rz = 0.0;
    eigen2Euler(pose.R, rx, ry, rz);  // 从3×3旋转矩阵提取欧拉角
    const double tx = pose.t.x(), ty = pose.t.y(), tz = pose.t.z();

    // -------------------------- 中间变量计算（含公式注释）--------------------------
    // 世界点（平面标定板，Z=0）：Pw = [Xw, Yw, 0]^T
    Eigen::Vector3d Pw(world_pt.x(), world_pt.y(), 0.0);
    double Xw = Pw.x(), Yw = Pw.y();

    // 相机坐标系坐标：Pc = R * Pw + t → [x_u, y_u, z_u]^T（x_u, y_u用于成像）
    Eigen::Vector3d Pc = pose.R * Pw + pose.t;
    double x_u = Pc.x();  // 理想X坐标（无畸变）
    double y_u = Pc.y();  // 理想Y坐标（无畸变）

    // 畸变计算项：r² = x_u² + (v0*dy)²（论文简化的径向畸变模型）
    double r_sq = x_u * x_u + (v0_ * dy_) * (v0_ * dy_);

    // 含畸变的坐标（论文公式）：
    // x_d = x_u + k * x_u * r² （X方向畸变）
    // y_d = y_u - k * v0 * dy * r² （Y方向畸变）
    double x_d = x_u + k_ * x_u * r_sq;
    double y_d = y_u - k_ * v0_ * dy_ * r_sq;

    // -------------------------- 1. 对平移向量(tx, ty, tz)的偏导数 --------------------------
    // 1.1 u方向重投影公式：hat_u = (m/dx)*(x_d - tanθ·y_d) + u0
    // 偏导数公式：∂hat_u/∂t = (m/dx)*(∂x_d/∂t - tanθ·∂y_d/∂t)
    const double d_xd__dtx = 1 + k_ * (r_sq + 2 * x_u * x_u);             // ∂x_d/∂tx（x_u对tx偏导=1，链式法则）
    const double d_yd__dty = 1 - k_ * v0_ * dy_ * 2 * (v0_ * dy_) * dy_;  // ∂y_d/∂ty（y_u对ty偏导=1）
    const double d_hat_u__dtx = (m_ / dx_) * d_xd__dtx;                   // ∂hat_u/∂tx
    const double d_hat_u__dty = -(m_ * tan(theta_) / dx_) * d_yd__dty;    // ∂hat_u/∂ty
    const double d_hat_u__dtz = 0.0;  // 远心镜头：z_u对x_u无影响 → ∂x_d/∂tz=0，∂y_d/∂tz=0

    // 1.2 v方向重投影公式（你的修改）：hat_v = (m/(dy·cosθ))·y_d + v0
    // 偏导数公式：∂hat_v/∂t = (m/(dy·cosθ))·∂y_d/∂t
    const double d_hat_v__dtx = 0.0;                                     // x_d不影响v方向 → ∂y_d/∂tx=0
    const double d_hat_v__dty = (m_ / (dy_ * cos(theta_))) * d_yd__dty;  // ∂hat_v/∂ty
    const double d_hat_v__dtz = 0.0;                                     // 远心镜头：∂y_d/∂tz=0

    // -------------------------- 2. 对旋转角(rx, ry, rz)的偏导数（核心推导）--------------------------
    // 2.1 旋转矩阵对欧拉角的导数矩阵（ZYX顺序）
    Eigen::Matrix3d dR_drx = computeDRdx(rx, ry, rz);  // R对rx的导数
    Eigen::Matrix3d dR_dry = computeDRdy(rx, ry, rz);  // R对ry的导数
    Eigen::Matrix3d dR_drz = computeDRdz(rx, ry, rz);  // R对rz的导数

    // 2.2 Pc对旋转角的偏导数：∂Pc/∂r = (∂R/∂r) * Pw（因t与旋转角无关）
    Eigen::Vector3d dPc_drx = dR_drx * Pw;  // ∂Pc/∂rx
    Eigen::Vector3d dPc_dry = dR_dry * Pw;  // ∂Pc/∂ry
    Eigen::Vector3d dPc_drz = dR_drz * Pw;  // ∂Pc/∂rz

    // 2.3 x_u, y_u对旋转角的偏导数（取Pc的X、Y分量）
    double d_xu_drx = dPc_drx.x();  // ∂x_u/∂rx
    double d_yu_drx = dPc_drx.y();  // ∂y_u/∂rx
    double d_xu_dry = dPc_dry.x();  // ∂x_u/∂ry
    double d_yu_dry = dPc_dry.y();  // ∂y_u/∂ry
    double d_xu_drz = dPc_drz.x();  // ∂x_u/∂rz
    double d_yu_drz = dPc_drz.y();  // ∂y_u/∂rz

    // 2.4 r²对旋转角的偏导数：∂r²/∂r = 2x_u·∂x_u/∂r（因r²=x_u²+(v0dy)²，第二项与r无关）
    double d_r_sq_drx = 2 * x_u * d_xu_drx;  // ∂r²/∂rx
    double d_r_sq_dry = 2 * x_u * d_xu_dry;  // ∂r²/∂ry
    double d_r_sq_drz = 2 * x_u * d_xu_drz;  // ∂r²/∂rz

    // 2.5 含畸变坐标(x_d, y_d)对旋转角的偏导数（链式法则）
    // x_d对r的偏导：∂x_d/∂r = ∂x_u/∂r + k·(∂x_u/∂r·r² + x_u·∂r²/∂r)
    double d_xd_drx = d_xu_drx + k_ * (d_xu_drx * r_sq + x_u * d_r_sq_drx);
    double d_xd_dry = d_xu_dry + k_ * (d_xu_dry * r_sq + x_u * d_r_sq_dry);
    double d_xd_drz = d_xu_drz + k_ * (d_xu_drz * r_sq + x_u * d_r_sq_drz);

    // y_d对r的偏导：∂y_d/∂r = ∂y_u/∂r - k·v0·dy·∂r²/∂r
    double d_yd_drx = d_yu_drx - k_ * v0_ * dy_ * d_r_sq_drx;
    double d_yd_dry = d_yu_dry - k_ * v0_ * dy_ * d_r_sq_dry;
    double d_yd_drz = d_yu_drz - k_ * v0_ * dy_ * d_r_sq_drz;

    // 2.6 重投影坐标(hat_u, hat_v)对旋转角的偏导数（代入重投影公式）
    // hat_u对r的偏导：∂hat_u/∂r = (m/dx)·(∂x_d/∂r - tanθ·∂y_d/∂r)
    const double d_hat_u_drx = (m_ / dx_) * (d_xd_drx - tan(theta_) * d_yd_drx);
    const double d_hat_u_dry = (m_ / dx_) * (d_xd_dry - tan(theta_) * d_yd_dry);
    const double d_hat_u_drz = (m_ / dx_) * (d_xd_drz - tan(theta_) * d_yd_drz);

    // hat_v对r的偏导（你的修改）：∂hat_v/∂r = (m/(dy·cosθ))·∂y_d/∂r
    const double d_hat_v_drx = (m_ / (dy_ * cos(theta_))) * d_yd_drx;
    const double d_hat_v_dry = (m_ / (dy_ * cos(theta_))) * d_yd_dry;
    const double d_hat_v_drz = (m_ / (dy_ * cos(theta_))) * d_yd_drz;

    // -------------------------- 3. 填充雅克比矩阵（误差偏导数 = -模型偏导数）--------------------------
    // 列顺序：[rx, ry, rz, tx, ty, tz]
    J_row_u(param_start_idx + 0) = -d_hat_u_drx;   // e_u对rx的偏导数
    J_row_u(param_start_idx + 1) = -d_hat_u_dry;   // e_u对ry的偏导数
    J_row_u(param_start_idx + 2) = -d_hat_u_drz;   // e_u对rz的偏导数
    J_row_u(param_start_idx + 3) = -d_hat_u__dtx;  // e_u对tx的偏导数
    J_row_u(param_start_idx + 4) = -d_hat_u__dty;  // e_u对ty的偏导数
    J_row_u(param_start_idx + 5) = -d_hat_u__dtz;  // e_u对tz的偏导数

    J_row_v(param_start_idx + 0) = -d_hat_v_drx;   // e_v对rx的偏导数
    J_row_v(param_start_idx + 1) = -d_hat_v_dry;   // e_v对ry的偏导数
    J_row_v(param_start_idx + 2) = -d_hat_v_drz;   // e_v对rz的偏导数
    J_row_v(param_start_idx + 3) = -d_hat_v__dtx;  // e_v对tx的偏导数
    J_row_v(param_start_idx + 4) = -d_hat_v__dty;  // e_v对ty的偏导数
    J_row_v(param_start_idx + 5) = -d_hat_v__dtz;  // e_v对tz的偏导数
}
// 计算R对rx的导数（ZYX欧拉角，R = Rz*Ry*Rx）
Eigen::Matrix3d TelecentricLMOptimizer::computeDRdx(double rx, double ry, double rz) const {
    double crx = cos(rx), srx = sin(rx);
    double cry = cos(ry), sry = sin(ry);
    double crz = cos(rz), srz = sin(rz);
    Eigen::Matrix3d dR;
    dR << -crz * sry * srx - srz * crx, -srz * sry * srx + crz * crx, 0, crz * sry * crx - srz * srx,
        -srz * sry * crx - crz * srx, 0, crz * cry * srx, -srz * cry * srx, 0;
    return dR;
}

// 计算R对ry的导数
Eigen::Matrix3d TelecentricLMOptimizer::computeDRdy(double rx, double ry, double rz) const {
    double crx = cos(rx), srx = sin(rx);
    double cry = cos(ry), sry = sin(ry);
    double crz = cos(rz), srz = sin(rz);
    Eigen::Matrix3d dR;
    dR << -crz * cry * crx - srz * srx, -srz * cry * crx + crz * srx, crz * sry, crz * cry * srx - srz * crx,
        -srz * cry * srx - crz * crx, crz * sry * srx - srz * crx, -crz * sry, srz * sry, -cry;
    return dR;
}

// 计算R对rz的导数
Eigen::Matrix3d TelecentricLMOptimizer::computeDRdz(double rx, double ry, double rz) const {
    double crx = cos(rx), srx = sin(rx);
    double cry = cos(ry), sry = sin(ry);
    double crz = cos(rz), srz = sin(rz);
    Eigen::Matrix3d dR;
    dR << -srz * cry * crx - crz * srx, -crz * cry * crx + srz * srx, -srz * sry, -srz * cry * srx + crz * crx,
        -crz * cry * srx - srz * crx, -srz * sry * srx + crz * crx, 0, 0, 0;
    return dR;
}
// 保存当前参数
void TelecentricLMOptimizer::saveCurrentParams() {
    saved_m_ = m_;
    saved_dy_ = dy_;
    saved_u0_ = u0_;
    saved_v0_ = v0_;
    saved_theta_ = theta_;
    saved_k_ = k_;
    saved_poses_ = current_poses_;
}

// 恢复保存的参数
void TelecentricLMOptimizer::restoreSavedParams() {
    m_ = saved_m_;
    dy_ = saved_dy_;
    u0_ = saved_u0_;
    v0_ = saved_v0_;
    theta_ = saved_theta_;
    k_ = saved_k_;
    current_poses_ = saved_poses_;
}

// 更新参数（含合理性约束）
void TelecentricLMOptimizer::updateParams(const Eigen::VectorXd& delta) {
    const int num_poses = current_poses_.size();

    // -------------------------- 1. 更新全局参数（先更新，后动态约束）--------------------------
    m_ += delta(0);
    dy_ += delta(1);
    u0_ += delta(2);
    v0_ += delta(3);
    theta_ += delta(4);
    k_ += delta(5);

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

    // -------------------------- 2. 更新外参（按初始姿态设动态边界）--------------------------
    for (int j = 0; j < num_poses; ++j) {
        const int pose_param_start = num_global_params + j * num_pose_params;
        double drx = delta(pose_param_start + 0);
        double dry = delta(pose_param_start + 1);
        double drz = delta(pose_param_start + 2);
        double dtx = delta(pose_param_start + 3);
        double dty = delta(pose_param_start + 4);
        double dtz = delta(pose_param_start + 5);

        // 2.1 读取当前姿态的外参初始值（从init_poses_中获取）
        const Pose& init_pose = init_poses_[j];
        double init_rx, init_ry, init_rz;
        eigen2Euler(init_pose.R, init_rx, init_ry, init_rz);  // 初始旋转角
        double init_tx = init_pose.t.x();                     // 初始平移x
        double init_ty = init_pose.t.y();                     // 初始平移y
        double init_tz = init_pose.t.z();                     // 初始平移z

        // 2.2 计算更新后的外参（未约束）
        // 平移更新
        double new_tx = current_poses_[j].t.x() + dtx;
        double new_ty = current_poses_[j].t.y() + dty;
        double new_tz = current_poses_[j].t.z() + dtz;
        // 旋转角更新（先转欧拉角）
        double rx, ry, rz;
        eigen2Euler(current_poses_[j].R, rx, ry, rz);
        double new_rx = rx + drx;
        double new_ry = ry + dry;
        double new_rz = rz + drz;

        // // 2.3 外参动态边界：初始姿态±波动幅度（机械安装误差范围内）
        // const double rx_ry_fluct = M_PI / 72;  // rx/ry：±2.5°（比初始值波动更小）
        // const double rz_fluct = M_PI / 18;     // rz：±10°（平面旋转可稍大）
        // const double tx_ty_fluct = 10.0;       // tx/ty：±10mm（平移初始值附近小调整）
        // const double tz_fluct = 0.0;           // tz：±0mm（远心镜头Z向波动稍大）

        // // 应用外参动态边界
        // new_rx = std::max(init_rx - rx_ry_fluct, std::min(init_rx + rx_ry_fluct, new_rx));
        // new_ry = std::max(init_ry - rx_ry_fluct, std::min(init_ry + rx_ry_fluct, new_ry));
        // new_rz = std::max(init_rz - rz_fluct, std::min(init_rz + rz_fluct, new_rz));
        // new_tx = std::max(init_tx - tx_ty_fluct, std::min(init_tx + tx_ty_fluct, new_tx));
        // new_ty = std::max(init_ty - tx_ty_fluct, std::min(init_ty + tx_ty_fluct, new_ty));
        // new_tz = std::max(init_tz - tz_fluct, std::min(init_tz + tz_fluct, new_tz));

        // 2.4 赋值约束后的外参
        current_poses_[j].t.x() = new_tx;
        current_poses_[j].t.y() = new_ty;
        current_poses_[j].t.z() = new_tz;
        euler2Eigen(new_rx, new_ry, new_rz, current_poses_[j].R);
    }
}

// 旋转矩阵→欧拉角（ZYX顺序示例）
// TODO：修改点！需与您的旋转参数化方式一致（如XYZ/YXZ等）
void TelecentricLMOptimizer::eigen2Euler(const Eigen::Matrix3d& R, double& rx, double& ry, double& rz) const {
    rz = atan2(R(1, 0), R(0, 0));
    double cy = cos(rz);
    double sy = sin(rz);
    ry = atan2(-R(2, 0), cy * R(0, 0) + sy * R(1, 0));
    double cx = cos(ry);
    double sx = sin(ry);
    rx = atan2(sy * R(0, 2) - cy * R(1, 2), -sx * R(0, 1) + cx * R(1, 1));
}

// 欧拉角→旋转矩阵（ZYX顺序示例）
// TODO：修改点！需与eigen2Euler的顺序一致
void TelecentricLMOptimizer::euler2Eigen(double rx, double ry, double rz, Eigen::Matrix3d& R) const {
    Eigen::AngleAxisd ax(rx, Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd ay(ry, Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd az(rz, Eigen::Vector3d::UnitZ());
    R = az * ay * ax;  // ZYX顺序：先X旋转，再Y，最后Z
}
// 打印参数优化前后的变化
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

    // 2. 外参变化（每个姿态6个参数：rx, ry, rz（角度）、tx, ty, tz）
    std::cout << "\n【2. 外参变化（每个姿态）】" << std::endl;
    for (int j = 0; j < current_poses_.size(); ++j) {
        const Pose& init_pose = init_poses_[j];
        const Pose& curr_pose = current_poses_[j];
        std::cout << "\n姿态 " << j + 1 << "：" << std::endl;

        // 2.1 旋转角变化（先将初始/当前旋转矩阵转为欧拉角，再对比）
        double init_rx, init_ry, init_rz;
        double curr_rx, curr_ry, curr_rz;
        eigen2Euler(init_pose.R, init_rx, init_ry, init_rz);  // 初始欧拉角（弧度）
        eigen2Euler(curr_pose.R, curr_rx, curr_ry, curr_rz);  // 当前欧拉角（弧度）
        // 弧度转角度（更直观）
        init_rx *= 180 / M_PI;
        init_ry *= 180 / M_PI;
        init_rz *= 180 / M_PI;
        curr_rx *= 180 / M_PI;
        curr_ry *= 180 / M_PI;
        curr_rz *= 180 / M_PI;

        // 2.2 平移向量变化
        double init_tx = init_pose.t.x(), init_ty = init_pose.t.y(), init_tz = init_pose.t.z();
        double curr_tx = curr_pose.t.x(), curr_ty = curr_pose.t.y(), curr_tz = curr_pose.t.z();

        // 打印旋转角变化
        std::cout << "  旋转角（ZYX顺序，单位：°）：" << std::endl
                  << "    rx：初始=" << init_rx << "，当前=" << curr_rx << "，变化=" << curr_rx - init_rx << std::endl
                  << "    ry：初始=" << init_ry << "，当前=" << curr_ry << "，变化=" << curr_ry - init_ry << std::endl
                  << "    rz：初始=" << init_rz << "，当前=" << curr_rz << "，变化=" << curr_rz - init_rz << std::endl;
        // 打印平移变化
        std::cout << "  平移向量（单位：mm）：" << std::endl
                  << "    tx：初始=" << init_tx << "，当前=" << curr_tx << "，变化=" << curr_tx - init_tx << std::endl
                  << "    ty：初始=" << init_ty << "，当前=" << curr_ty << "，变化=" << curr_ty - init_ty << std::endl
                  << "    tz：初始=" << init_tz << "，当前=" << curr_tz << "，变化=" << curr_tz - init_tz << std::endl;
    }

    std::cout << "\n==============================================================" << std::endl;
}

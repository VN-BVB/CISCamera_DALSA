#include "telecentricplatform_calib.h"

TelecentricPlatformCalib::TelecentricPlatformCalib() {}
// void TelecentricPlatformCalib::iniCalibParams() {
//     CalibrationData calibParam;
//     if (!calibParam.load("./data/calibration_config/optimized_calib_data.json")) {
//         throw std::runtime_error("无法加载标定文件");
//     }
//     K_ = calibParam.K;
//     dist_ = calibParam.coff_dis;
// }
TelecentricPlatformCalib::TelecentricPlatformCalib(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& dist, const Eigen::Vector3d& rvec,
                                                   const Eigen::Vector3d& tvec)
    : K_(K), dist_(dist), v_rot_(rvec), v_trans_(tvec) {
    lineCalib_ = new TelecentricLineCalibrator();
}

std::vector<Eigen::Vector2d> TelecentricPlatformCalib::convertToWorld(const std::vector<Eigen::Vector2d>& pix_pts) {
    if (pix_pts.empty()) return {};

    Eigen::MatrixXd px = vecToMat(pix_pts);

    // pixel → camera (去畸变 + 归一化)
    Eigen::MatrixXd cam_norm = lineCalib_->pixelToCameraCoordinates(px, K_, dist_);
    // Eigen::MatrixXd world = cam_norm;
    // camera → world（逆平面变换）
    Eigen::MatrixXd world = lineCalib_->cameraToWorldCoordinates(cam_norm, v_rot_, v_trans_);

    return matToVec(world);
}
// ======================== 仿射最小二乘求解 ========================
void TelecentricPlatformCalib::solveAffineFromRelativeMotion(const std::vector<Eigen::Vector2d>& w1, const std::vector<Eigen::Vector2d>& w2,
                                                             const std::vector<Eigen::Vector2d>& w3, Eigen::Matrix2d& A_ls, int method) {
    const int N = static_cast<int>(w1.size());
    if (w2.size() != N || w3.size() != N || N < 3) {
        throw std::runtime_error("点数不一致或过少");
    }
    if (method == 1) {
        Eigen::MatrixXd M(4 * N, 4);
        Eigen::VectorXd b(4 * N);

        int row = 0;
        for (int i = 0; i < N; ++i) {
            Eigen::Vector2d d12 = w2[i] - w1[i];
            Eigen::Vector2d d23 = w3[i] - w2[i];

            // A * d12 = [50, 0]
            M.row(row) << d12.x(), d12.y(), 0, 0;
            b(row++) = 50.0;
            M.row(row) << 0, 0, d12.x(), d12.y();
            b(row++) = 0.0;

            // A * d23 = [0, 50]
            M.row(row) << d23.x(), d23.y(), 0, 0;
            b(row++) = 0.0;
            M.row(row) << 0, 0, d23.x(), d23.y();
            b(row++) = 50.0;
        }

        Eigen::VectorXd x = M.colPivHouseholderQr().solve(b);

        A_ls << x(0), x(1), x(2), x(3);
    } else if (method == 2) {
        Eigen::Vector2d dw12 = Eigen::Vector2d::Zero();
        Eigen::Vector2d dw23 = Eigen::Vector2d::Zero();

        for (size_t i = 0; i < w1.size(); ++i) {
            dw12 += (w2[i] - w1[i]);  // 实测 p1 → p2 位移（应 ≈ [-50, 0]）
            dw23 += (w3[i] - w2[i]);  // 实测 p2 → p3 位移（应 ≈ [0, 50]）
        }

        dw12 /= static_cast<double>(w1.size());
        dw23 /= static_cast<double>(w1.size());

        // 构建实测位移矩阵 D = [dw12, dw23]
        Eigen::Matrix2d D;
        D.col(0) = dw12;
        D.col(1) = dw23;

        // 构建理想位移矩阵：X 方向 -50mm，Y 方向 +50mm
        Eigen::Matrix2d I_ideal;
        I_ideal << 50.0, 0.0, 0.0, 50.0;

        // 求解 A * D = I_ideal  =>  A = I_ideal * D^{-1}
        double det_D = D.determinant();
        if (std::abs(det_D) < 1e-8) {
            throw std::runtime_error("平均位移矩阵 D 奇异（运动退化或数据异常）");
        }

        A_ls = I_ideal * D.inverse();

        std::cout << "A_ls (平均位移，修正版) =\n" << A_ls << std::endl;
    }
}

Eigen::Vector3d TelecentricPlatformCalib::computeDirectionLS(const std::vector<Eigen::Vector2d>& ptsA, const std::vector<Eigen::Vector2d>& ptsB) {
    if (ptsA.size() != ptsB.size() || ptsA.empty()) {
        PLOGE << "方向拟合尺寸点数不匹配 ";
        return Eigen::Vector3d::Zero();
    }

    // 1. 构造差分矩阵 D
    Eigen::MatrixXd D(ptsA.size(), 2);
    for (int i = 0; i < ptsA.size(); i++) {
        D(i, 0) = ptsB[i].x() - ptsA[i].x();
        D(i, 1) = ptsB[i].y() - ptsA[i].y();
    }

    // 2. 用 SVD 求最小二乘拟合方向
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(D, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::Vector2d dir = svd.matrixV().col(0);

    // 3. 判断正负：用 D 的平均位移方向投影
    Eigen::Vector2d avg_disp = D.colwise().mean();
    if (dir.dot(avg_disp) < 0) {
        dir = -dir;  // 方向反转
    }

    return Eigen::Vector3d(dir.x(), dir.y(), 0.0).normalized();
}

Eigen::Vector2d TelecentricPlatformCalib::computeRotationCenterSequential(const std::vector<std::vector<Eigen::Vector2d>>& pts) {
    if (pts.size() < 2) return Eigen::Vector2d::Zero();

    Eigen::Vector2d sumC = Eigen::Vector2d::Zero();
    int count = 0;

    for (int k = 0; k < pts.size() - 1; k++) {
        const auto& P = pts[k];
        const auto& Q = pts[k + 1];

        if (P.size() != Q.size() || P.empty()) continue;

        int n = P.size();
        Eigen::MatrixXd Pm(2, n), Qm(2, n);

        for (int i = 0; i < n; i++) {
            Pm.col(i) << P[i].x(), P[i].y();
            Qm.col(i) << Q[i].x(), Q[i].y();
        }

        Eigen::Vector2d meanP = Pm.rowwise().mean();
        Eigen::Vector2d meanQ = Qm.rowwise().mean();

        Eigen::MatrixXd Pc = Pm.colwise() - meanP;
        Eigen::MatrixXd Qc = Qm.colwise() - meanQ;

        Eigen::Matrix2d H = Pc * Qc.transpose();
        Eigen::JacobiSVD<Eigen::Matrix2d> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);

        Eigen::Matrix2d R = svd.matrixV() * svd.matrixU().transpose();
        if (R.determinant() < 0) {
            Eigen::Matrix2d V = svd.matrixV();
            V.col(1) *= -1;
            R = V * svd.matrixU().transpose();
        }

        double ang = std::atan2(R(1, 0), R(0, 0)) * 180.0 / M_PI;

        // 跳过旋转角太小的 pair（I-R 接近奇异，求逆会产生 inf/NaN）
        if (std::abs(ang) < 0.005) {
            std::cout << "第 " << k << " 组旋转角 = " << ang << "° 过小，跳过\n";
            continue;
        }

        Eigen::Matrix2d I = Eigen::Matrix2d::Identity();
        Eigen::Vector2d C = (I - R).inverse() * (meanQ - R * meanP);

        // 防 NaN
        if (!std::isfinite(C.x()) || !std::isfinite(C.y())) {
            std::cout << "第 " << k << " 组旋转中心 NaN，跳过\n";
            continue;
        }

        sumC += C;
        count++;
        std::cout << u8"第 " << k << u8" 组旋转中心 = " << C.transpose() << u8" 角度 = " << ang << "°\n";
    }

    if (count == 0) {
        std::cout << u8"警告: 没有有效的旋转对，返回零旋转中心\n";
        return Eigen::Vector2d::Zero();
    }
    return sumC / count;
}
// -------- Taubin圆拟合单组点 --------
Eigen::Vector2d TelecentricPlatformCalib::fitCircleTaubin(const std::vector<Eigen::Vector2d>& pts) {
    size_t n = pts.size();
    Eigen::MatrixXd Z(n, 3);
    Eigen::VectorXd ones = Eigen::VectorXd::Ones(n);

    double mean_x = 0, mean_y = 0;
    for (const auto& p : pts) {
        mean_x += p.x();
        mean_y += p.y();
    }
    mean_x /= n;
    mean_y /= n;

    for (size_t i = 0; i < n; ++i) {
        double xi = pts[i].x() - mean_x;
        double yi = pts[i].y() - mean_y;
        Z(i, 0) = xi * xi + yi * yi;
        Z(i, 1) = xi;
        Z(i, 2) = yi;
    }

    Eigen::Vector3d coeff = (Z.transpose() * Z).ldlt().solve(Z.transpose() * ones);

    double a = coeff(1) / 2.0;
    double b = coeff(2) / 2.0;
    double x_c = mean_x - a;
    double y_c = mean_y - b;

    return Eigen::Vector2d(x_c, y_c);
}

Eigen::Vector2d TelecentricPlatformCalib::computeRotationCenterCircleFit(const std::vector<std::vector<Eigen::Vector2d>>& pts) {
    // 至少需要 3 组点
    const size_t K = pts.size();
    if (K < 3) {
        std::cerr << "至少需要三组点来确定圆！\n";
        return Eigen::Vector2d::Zero();
    }

    // 各组点的数量必须一致
    const size_t N = pts[0].size();
    for (size_t k = 1; k < K; ++k) {
        if (pts[k].size() != N) {
            std::cerr << "所有点集的点数量必须一致！\n";
            return Eigen::Vector2d::Zero();
        }
    }

    std::vector<Eigen::Vector2d> circle_centers;
    circle_centers.reserve(N);

    // -------- 每个点 index 单独拟合一个圆心 --------
    for (size_t i = 0; i < N; ++i) {
        std::vector<Eigen::Vector2d> Pi;
        Pi.reserve(K);

        for (size_t k = 0; k < K; ++k) {
            Pi.push_back(pts[k][i]);  // 第 k 组的第 i 个点
        }

        // 拟合该 index 对应的旋转圆
        Eigen::Vector2d c = fitCircleTaubin(Pi);
        circle_centers.push_back(c);

        std::cout << "点 index " << i << " 拟合圆心: [" << c.transpose() << "]\n";
    }

    // -------- 所有圆心求平均 --------
    Eigen::Vector2d final_center = Eigen::Vector2d::Zero();
    for (auto& c : circle_centers) final_center += c;

    final_center /= circle_centers.size();

    std::cout << "最终平均旋转中心: [" << final_center.transpose() << "]\n";
    return final_center;
}
void TelecentricPlatformCalib::runDemo(std::vector<std::vector<Eigen::Vector2d>> pts) {
    K_ << 47.283237490301396, -0.657929607742621, 15551.964431991371, 0.0, 47.05230788272559, 8043.186819107249, 0.0, 0.0, 1.0;

    dist_ << -5.363602460785097e-10, -6.586873205793823e-07, -3.9297031624526706e-07, 2.075287196873092e-06, -2.0074419972225162e-06;
    v_rot_ = {2.074776703520814, 2.0584407379860554, -0.21906585124567024};
    v_trans_ = {-249.01625128531074, -134.99191717289557, 0.0};
    // ===========================================================
    // 修正远心相机下的旋转向量，并显式分离出 S（误差源）
    // 输入 / 输出：全局 v_rot_
    // 物理含义：
    //   Q —— 平面刚体旋转（SO(2)）
    //   S —— 平面尺度 / 剪切（误差来源）
    // ===========================================================

    // -------- 1. Rodrigues 向量 → SO(3) --------
    cv::Mat rvec_cv(3, 1, CV_64F);
    for (int i = 0; i < 3; ++i) rvec_cv.at<double>(i, 0) = v_rot_(i);

    cv::Mat R_cv;
    cv::Rodrigues(rvec_cv, R_cv);

    Eigen::Matrix3d R;
    cv::cv2eigen(R_cv, R);

    // -------- 2. 提取 xy 子块 --------
    Eigen::Matrix2d R_xy = R.block<2, 2>(0, 0);

    // -------- 3. 极分解：R_xy = Q * S --------
    Eigen::JacobiSVD<Eigen::Matrix2d> svd(R_xy, Eigen::ComputeFullU | Eigen::ComputeFullV);

    Eigen::Matrix2d U = svd.matrixU();
    Eigen::Matrix2d V = svd.matrixV();
    Eigen::Vector2d sigma = svd.singularValues();

    // --- 3.1 刚体旋转部分 Q ∈ SO(2)
    Eigen::Matrix2d Q = U * V.transpose();

    // 保证 det(Q) = +1
    if (Q.determinant() < 0) {
        U.col(1) *= -1;
        Q = U * V.transpose();
    }

    // --- 3.2 对称正定部分 S（误差来源）
    Eigen::Matrix2d Sigma = Eigen::Matrix2d::Zero();
    Sigma(0, 0) = sigma(0);
    Sigma(1, 1) = sigma(1);

    Eigen::Matrix2d S = V * Sigma * V.transpose();

    // -------- 4. SO(2) → SO(3)（仅允许绕 Z 轴）--------
    Eigen::Matrix3d R_fixed = Eigen::Matrix3d::Identity();
    R_fixed.block<2, 2>(0, 0) = Q;

    // -------- 5. SO(3) → Rodrigues，回写全局旋转向量 --------
    cv::Mat R_fixed_cv;
    cv::eigen2cv(R_fixed, R_fixed_cv);

    cv::Mat rvec_fixed_cv;
    cv::Rodrigues(R_fixed_cv, rvec_fixed_cv);

    // for (int i = 0; i < 3; ++i) v_rot_(i) = rvec_fixed_cv.at<double>(i, 0);

    // -------- 6. 打印：用于误差分析（非常重要）--------
    std::cout << "\n===== Polar Decomposition Analysis =====\n";
    std::cout << "R_xy =\n" << R_xy << "\n\n";

    std::cout << "Q (SO(2)) =\n" << Q << "\n";
    std::cout << "det(Q) = " << Q.determinant() << "\n\n";

    std::cout << "S (Symmetric, SPD) =\n" << S << "\n";
    std::cout << "Singular values = " << sigma.transpose() << "\n";

    std::cout << "Anisotropy |s1 - s2| = " << std::abs(sigma(0) - sigma(1)) << "\n";

    std::cout << "========================================\n";
    std::vector<Eigen::Vector2d> p1, p2, p3, p4, p5;
    readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform10.txt", p1);
    readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform21.txt", p2);
    readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform30d.txt", p3);
    readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform40d.txt", p4);
    readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform50d.txt", p5);

    auto w1 = convertToWorld(p1);
    auto w2 = convertToWorld(p2);
    auto w3 = convertToWorld(p3);
    auto w4 = convertToWorld(p4);
    auto w5 = convertToWorld(p5);

    Eigen::Vector3d xdir = computeDirectionLS(w1, w2);
    Eigen::Vector3d ydir = computeDirectionLS(w2, w3);

    std::cout << "X方向 = " << xdir.transpose() << std::endl;
    std::cout << "Y方向 = " << ydir.transpose() << std::endl;
    // ================= 最小二乘仿射 =================
    Eigen::Matrix2d A_ls;
    solveAffineFromRelativeMotion(w1, w2, w3, A_ls, 1);
    std::cout << "A_ls (最小二乘) =\n" << A_ls << std::endl;
    // ================= 计算旋转中心 =================
    std::vector<std::vector<Eigen::Vector2d>> ptsRot{w3, w4, w5};
    Eigen::Vector2d C = computeRotationCenterSequential(ptsRot);

    // 把 world 点变换到“临时平台坐标系”（原点=world原点）
    std::vector<Eigen::Vector2d> p3_tempPlat, p4_tempPlat, p5_tempPlat;
    for (auto& pt : w3) p3_tempPlat.push_back(A_ls * pt);
    for (auto& pt : w4) p4_tempPlat.push_back(A_ls * pt);
    for (auto& pt : w5) p5_tempPlat.push_back(A_ls * pt);

    Eigen::Vector2d C_in_temp_platform = computeRotationCenterSequential({p3_tempPlat, p4_tempPlat, p5_tempPlat});

    std::cout << "旋转中心 = " << C_in_temp_platform.transpose() << std::endl;
    auto checkAffineDirection = [](const std::string& name, const Eigen::Matrix2d& A, const Eigen::Vector3d& xdir, const Eigen::Vector3d& ydir) {
        Eigen::Vector2d ax = A.col(0).normalized();
        Eigen::Vector2d ay = A.col(1).normalized();

        Eigen::Vector2d xdir2(xdir.x(), xdir.y());
        Eigen::Vector2d ydir2(ydir.x(), ydir.y());

        double cos_x = ax.dot(xdir2.normalized());
        double cos_y = ay.dot(ydir2.normalized());

        std::cout << "---- " << name << " 方向一致性 ----" << std::endl;
        std::cout << "col0 vs xdir cos = " << cos_x << std::endl;
        std::cout << "col1 vs ydir cos = " << cos_y << std::endl;
    };
    auto backProjectDisplacement = [](const std::string& name, const Eigen::Matrix2d& A, const std::vector<Eigen::Vector2d>& w1,
                                      const std::vector<Eigen::Vector2d>& w2, const std::vector<Eigen::Vector2d>& w3) {
        double err_x = 0.0;
        double err_y = 0.0;
        int N = static_cast<int>(w1.size());

        std::cout << "\n==== " << name << " 回代位移验证 (N=" << N << ") ====" << std::endl;
        std::cout << "A =\n" << A << std::endl;
        std::cout << "期望: d12 -> [50, 0]^T,  d23 -> [0, 50]^T (单位: mm)\n" << std::endl;

        for (int i = 0; i < N; ++i) {
            Eigen::Vector2d d12 = w2[i] - w1[i];
            Eigen::Vector2d d23 = w3[i] - w2[i];

            Eigen::Vector2d p12 = A * d12;
            Eigen::Vector2d p23 = A * d23;

            Eigen::Vector2d residual_x = p12 - Eigen::Vector2d(50.0, 0.0);
            Eigen::Vector2d residual_y = p23 - Eigen::Vector2d(0.0, 50.0);

            double sq_err_x = residual_x.squaredNorm();
            double sq_err_y = residual_y.squaredNorm();

            err_x += sq_err_x;
            err_y += sq_err_y;

            // 调试打印前几个点（避免刷屏）
            if (i < 3 || i == N - 1) {
                std::cout << "[点 " << i << "] " << "d12=" << d12.transpose() << " → p12=" << p12.transpose() << " (res_x=" << residual_x.transpose()
                          << ", |res|=" << std::sqrt(sq_err_x) << ")" << std::endl;
                std::cout << "         " << "d23=" << d23.transpose() << " → p23=" << p23.transpose() << " (res_y=" << residual_y.transpose()
                          << ", |res|=" << std::sqrt(sq_err_y) << ")" << std::endl;
            }
            if (i == 2 && N > 4) {
                std::cout << "      ... (省略中间点) ..." << std::endl;
            }
        }

        err_x = std::sqrt(err_x / N);
        err_y = std::sqrt(err_y / N);

        std::cout << "\n---- " << name << " 回代误差 ----" << std::endl;
        std::cout << "X 方向 RMS = " << err_x << " mm" << std::endl;
        std::cout << "Y 方向 RMS = " << err_y << " mm" << std::endl;
        std::cout << "========================================\n" << std::endl;
    };
    checkAffineDirection("A_ls", A_ls, xdir, ydir);
    backProjectDisplacement("A_ls", A_ls, w1, w2, w3);
    // Step 1: 从 v_rot_ 构建 3x3 旋转矩阵 R_wc (world -> camera)
    cv::Mat rvec_cv34 = (cv::Mat_<double>(3, 1) << v_rot_[0], v_rot_[1], v_rot_[2]);
    cv::Mat R_cv34;
    cv::Rodrigues(rvec_cv34, R_cv34);  // R_cv34: world -> camera

    Eigen::Matrix3d R_wc;
    cv::cv2eigen(R_cv34, R_wc);

    // Step 2: 提取 2x2 旋转块 和 2D 平移
    Eigen::Matrix2d R_wc_2 = R_wc.block<2, 2>(0, 0);   // 注意：不是正交！
    Eigen::Vector2d t_wc_2(v_trans_[0], v_trans_[1]);  // world -> camera 平移前两维

    // Step 3: 计算 platform -> camera 的仿射参数
    Eigen::Matrix2d M = R_wc_2 * A_ls.inverse();  // 线性部分
    Eigen::Vector2d C_world = A_ls.inverse() * C_in_temp_platform;
    Eigen::Vector2d b = R_wc_2 * C_world + t_wc_2;  // 平移部分

    // 输出结果
    std::cout << "\n=== Platform -> Camera (2D Affine) ===" << std::endl;
    std::cout << "M =\n" << M << std::endl;
    std::cout << "b = " << b.transpose() << std::endl;
    // 提取 M 的元素
    double m11 = M(0, 0), m21 = M(1, 0);  // 第一列 (x)
    double m12 = M(0, 1), m22 = M(1, 1);  // 第二列 (y)

    // 计算可能的 |r13|, |r23|（由单位长度约束）
    double r13_sq = 1.0 - m11 * m11 - m21 * m21;
    double r23_sq = 1.0 - m12 * m12 - m22 * m22;

    // 数值安全处理
    r13_sq = std::max(0.0, r13_sq);
    r23_sq = std::max(0.0, r23_sq);

    double abs_r13 = std::sqrt(r13_sq);
    double abs_r23 = std::sqrt(r23_sq);

    // 评估函数：越小越好（正交性 + Z朝前）
    auto score_func = [&](double r13, double r23) -> double {
        Eigen::Vector3d c1(m11, m21, r13);
        Eigen::Vector3d c2(m12, m22, r23);
        c1.normalize();
        c2.normalize();

        double ortho_error = std::abs(c1.dot(c2));  // 应接近 0
        Eigen::Vector3d c3 = c1.cross(c2);
        double z_penalty = (c3.z() < 0) ? 1e6 : 0.0;  // 强制 Z 朝前

        return ortho_error + z_penalty;
    };

    // 枚举符号组合
    double best_score = 1e9;
    Eigen::Matrix3d bestR = Eigen::Matrix3d::Identity();

    for (int s1 : {-1, 1}) {
        for (int s2 : {-1, 1}) {
            double r13 = s1 * abs_r13;
            double r23 = s2 * abs_r23;

            double score = score_func(r13, r23);
            if (score < best_score) {
                best_score = score;

                Eigen::Vector3d c1(m11, m21, r13);
                Eigen::Vector3d c2(m12, m22, r23);
                c1.normalize();
                c2.normalize();
                Eigen::Vector3d c3 = c1.cross(c2);
                c3.normalize();

                // 确保右手系且光轴朝前（Z>0）
                if (c3.z() < 0) {
                    c1 = -c1;
                    c2 = -c2;
                    c3 = -c3;
                }

                bestR.col(0) = c1;
                bestR.col(1) = c2;
                bestR.col(2) = c3;
            }
        }
    }

    // 转为旋转向量
    cv::Mat R_cv45, rvec_cv45;
    cv::eigen2cv(bestR, R_cv45);
    cv::Rodrigues(R_cv45, rvec_cv45);

    Eigen::Vector3d rvec_platform_to_camera(rvec_cv45.at<double>(0), rvec_cv45.at<double>(1), rvec_cv45.at<double>(2));
    Eigen::Vector3d t_platform_to_camera(b.x(), b.y(), 0.0);

    // -------------------- 非线性优化 --------------------
    TelecentricPYOptimizer optimizer;

    Eigen::Vector3d rvec_opt, tvec_opt;
    double rms = 0.0;

    std::vector<std::vector<Eigen::Vector2d>> all_pts{p1, p2, p3, p4, p5};

    bool ok = optimizer.refinePlatformExtrinsicsLM(K_, dist_, all_pts, rvec_platform_to_camera, t_platform_to_camera,
                                                   50.0,  // dx 约束
                                                   50.0,  // dy 约束
                                                   45.0,  // 旋转角约束（deg）
                                                   rvec_opt, tvec_opt, rms);

    if (ok) {
        std::cout << "LM 优化成功\n";
        std::cout << "rvec_opt = " << rvec_opt.transpose() << std::endl;
        std::cout << "tvec_opt = " << tvec_opt.transpose() << std::endl;
        std::cout << "RMS = " << rms << std::endl;
    }
    // // ==================== 1. 提取像素坐标（复用已加载的 p1~p5） ====================
    // auto p1_pix = p1;  // platform10.txt
    // auto p2_pix = p2;  // platform21.txt
    // auto p3_pix = p3;  // platform30d.txt
    // auto p4_pix = p4;  // platform40d.txt
    // auto p5_pix = p5;  // platform50d.txt

    // // ==================== 2. pixel -> camera ====================
    // auto pixelToCamera = [&](const std::vector<Eigen::Vector2d>& pix_pts) {
    //     Eigen::MatrixXd px = vecToMat(pix_pts);
    //     Eigen::MatrixXd cam = lineCalib_->pixelToCameraCoordinates(px, K_, dist_);
    //     return matToVec(cam);
    // };

    // auto p1_cam = pixelToCamera(p1_pix);
    // auto p2_cam = pixelToCamera(p2_pix);
    // auto p3_cam = pixelToCamera(p3_pix);
    // auto p4_cam = pixelToCamera(p4_pix);
    // auto p5_cam = pixelToCamera(p5_pix);

    // // ==================== 3. pixel -> world ====================
    // auto p1_world = convertToWorld(p1_pix);
    // auto p2_world = convertToWorld(p2_pix);
    // auto p3_world = convertToWorld(p3_pix);
    // auto p4_world = convertToWorld(p4_pix);
    // auto p5_world = convertToWorld(p5_pix);

    // // ===========================================================================
    // // ✅ 第一部分：X/Y 方向向量的一致性验证（使用 p1, p2, p3）
    // // ===========================================================================
    // // 在各自坐标系下拟合方向
    // Eigen::Vector3d xdir_pix = computeDirectionLS(p1_pix, p2_pix).normalized();
    // Eigen::Vector3d ydir_pix = computeDirectionLS(p2_pix, p3_pix).normalized();

    // Eigen::Vector3d xdir_cam = computeDirectionLS(p1_cam, p2_cam).normalized();
    // Eigen::Vector3d ydir_cam = computeDirectionLS(p2_cam, p3_cam).normalized();

    // Eigen::Vector3d xdir_world = computeDirectionLS(p1_world, p2_world).normalized();
    // Eigen::Vector3d ydir_world = computeDirectionLS(p2_world, p3_world).normalized();

    // // 获取外参旋转矩阵（world → camera）
    // cv::Mat rvec_cv23(3, 1, CV_64F);
    // for (int i = 0; i < 3; ++i) rvec_cv23.at<double>(i, 0) = v_rot_(i);
    // cv::Mat R_cv23;
    // cv::Rodrigues(rvec_cv23, R_cv23);
    // Eigen::Matrix3d R_world_cam;
    // cv::cv2eigen(R_cv23, R_world_cam);
    // Eigen::Matrix3d R_cam_world = R_world_cam.transpose();  // camera → world

    // // 将相机系方向转到世界系（用于对比）
    // Eigen::Vector3d xdir_cam_to_world = R_cam_world * xdir_cam;
    // Eigen::Vector3d ydir_cam_to_world = R_cam_world * ydir_cam;

    // // 将世界系方向转到相机系（用于对比）
    // Eigen::Vector3d xdir_world_to_cam = R_world_cam * xdir_world;
    // Eigen::Vector3d ydir_world_to_cam = R_world_cam * ydir_world;

    // // 打印方向验证
    // std::cout << "\n========== Direction Vector Consistency Verification ==========\n";
    // std::cout << "X dir (world LS)      = " << xdir_world.head<2>().transpose() << "\n";
    // std::cout << "Y dir (world LS)      = " << ydir_world.head<2>().transpose() << "\n";
    // std::cout << "X dir (world→cam)     = " << xdir_world_to_cam.head<2>().transpose() << "\n";
    // std::cout << "Y dir (world→cam)     = " << ydir_world_to_cam.head<2>().transpose() << "\n";
    // std::cout << "X dir (camera LS)     = " << xdir_cam.head<2>().transpose() << "\n";
    // std::cout << "Y dir (camera LS)     = " << ydir_cam.head<2>().transpose() << "\n";
    // std::cout << "X dir (cam→world)     = " << xdir_cam_to_world.head<2>().transpose() << "\n";
    // std::cout << "Y dir (cam→world)     = " << ydir_cam_to_world.head<2>().transpose() << "\n";

    // std::cout << "\n--- Direction Errors (XY plane L2 norm) ---\n";
    // std::cout << "||X: cam→world - world|| = " << (xdir_cam_to_world.head<2>() - xdir_world.head<2>()).norm() << "\n";
    // std::cout << "||Y: cam→world - world|| = " << (ydir_cam_to_world.head<2>() - ydir_world.head<2>()).norm() << "\n";
    // std::cout << "||X: world→cam - cam||   = " << (xdir_world_to_cam.head<2>() - xdir_cam.head<2>()).norm() << "\n";
    // std::cout << "||Y: world→cam - cam||   = " << (ydir_world_to_cam.head<2>() - ydir_cam.head<2>()).norm() << "\n";
    // std::cout << "===============================================\n";

    // // ===========================================================================
    // // ✅ 第二部分：旋转中心一致性验证（使用 p3, p4, p5）
    // // ===========================================================================
    // std::vector<std::vector<Eigen::Vector2d>> ptsRot_pix{p3_pix, p4_pix, p5_pix};
    // std::vector<std::vector<Eigen::Vector2d>> ptsRot_cam{p3_cam, p4_cam, p5_cam};
    // std::vector<std::vector<Eigen::Vector2d>> ptsRot_world{p3_world, p4_world, p5_world};

    // Eigen::Vector2d C_pix = computeRotationCenterSequential(ptsRot_pix);
    // Eigen::Vector2d C_cam = computeRotationCenterSequential(ptsRot_cam);
    // Eigen::Vector2d C_world = computeRotationCenterSequential(ptsRot_world);

    // // pixel → camera（旋转中心）
    // Eigen::Vector2d C_pix_cam;
    // {
    //     std::vector<Eigen::Vector2d> tmp = {C_pix};
    //     Eigen::MatrixXd px = vecToMat(tmp);
    //     Eigen::MatrixXd cam = lineCalib_->pixelToCameraCoordinates(px, K_, dist_);
    //     C_pix_cam = cam.row(0).transpose();
    // }

    // // world → camera（旋转中心）
    // Eigen::Vector2d C_world_cam;
    // {
    //     Eigen::Vector3d Pw(C_world.x(), C_world.y(), 0.0);
    //     Eigen::Vector3d Pc = R_world_cam * Pw + v_trans_;
    //     C_world_cam = Pc.head<2>();
    // }

    // // camera → world（旋转中心）
    // Eigen::Vector2d C_cam_world;
    // {
    //     Eigen::MatrixXd cam_mat(1, 2);
    //     cam_mat.row(0) = C_cam.transpose();
    //     Eigen::MatrixXd world_mat = lineCalib_->cameraToWorldCoordinates(cam_mat, v_rot_, v_trans_);
    //     C_cam_world = world_mat.row(0).transpose();
    // }

    // // pixel → world（旋转中心）
    // Eigen::Vector2d C_pix_world = convertToWorld({C_pix})[0];

    // // 打印旋转中心验证
    // std::cout << "\n========== Rotation Center Verification ==========\n";
    // std::cout << "C_pixel (direct)      = " << C_pix.transpose() << std::endl;
    // std::cout << "C_camera (direct)     = " << C_cam.transpose() << std::endl;
    // std::cout << "C_world (direct)      = " << C_world.transpose() << std::endl;
    // std::cout << "C_pixel -> camera     = " << C_pix_cam.transpose() << std::endl;
    // std::cout << "C_world -> camera     = " << C_world_cam.transpose() << std::endl;
    // std::cout << "C_camera -> world     = " << C_cam_world.transpose() << std::endl;
    // std::cout << "C_pixel -> world      = " << C_pix_world.transpose() << std::endl;

    // std::cout << "-----------------------------------------------\n";
    // std::cout << "||C_cam - C_pix_cam||   = " << (C_cam - C_pix_cam).norm() << std::endl;
    // std::cout << "||C_cam - C_world_cam|| = " << (C_cam - C_world_cam).norm() << std::endl;
    // std::cout << "||C_world - C_cam||     = " << (C_world - C_cam_world).norm() << std::endl;
    // std::cout << "||C_world - C_pix||     = " << (C_world - C_pix_world).norm() << std::endl;
    // std::cout << "===============================================\n" << std::endl;

    // // ==================== 1. 世界坐标中的两个点 ====================
    // Eigen::Vector2d Pw1(10.0, 10.0);
    // Eigen::Vector2d Pw2(20.0, 20.0);

    // Eigen::Vector2d dPw = Pw2 - Pw1;
    // double d_world = dPw.norm();

    // // ==================== 2. Rodrigues → R ====================
    // cv::Mat rvec_cv3(3, 1, CV_64F);
    // for (int i = 0; i < 3; ++i) rvec_cv3.at<double>(i, 0) = v_rot_(i);

    // cv::Mat R_cv3;
    // cv::Rodrigues(rvec_cv3, R_cv3);

    // Eigen::Matrix3d R_raw;
    // cv::cv2eigen(R_cv3, R_raw);

    // // ==================== 3. 取平面旋转块 ====================
    // Eigen::Matrix2d R_xy_raw = R_raw.block<2, 2>(0, 0);

    // // ==================== 4. 极分解：R_xy_raw = Q * S ====================
    // Eigen::JacobiSVD<Eigen::Matrix2d> svd_raw(R_xy_raw, Eigen::ComputeFullU | Eigen::ComputeFullV);

    // Eigen::Matrix2d U_raw = svd_raw.matrixU();
    // Eigen::Matrix2d V_raw = svd_raw.matrixV();

    // Eigen::Matrix2d Q_raw = U_raw * V_raw.transpose();

    // if (Q_raw.determinant() < 0) {
    //     U_raw.col(1) *= -1;
    //     Q_raw = U_raw * V_raw.transpose();
    // }

    // Eigen::Matrix2d S_raw = V_raw * svd_raw.singularValues().asDiagonal() * V_raw.transpose();

    // // ==================== 5. 两种方式投影到相机 ====================
    // Eigen::Vector2d Pc1_R = R_xy_raw * Pw1 + v_trans_.head<2>();
    // Eigen::Vector2d Pc2_R = R_xy_raw * Pw2 + v_trans_.head<2>();

    // Eigen::Vector2d Pc1_Q = Q_raw * Pw1 + v_trans_.head<2>();
    // Eigen::Vector2d Pc2_Q = Q_raw * Pw2 + v_trans_.head<2>();

    // // ==================== 6. 距离计算 ====================
    // double d_cam_R = (Pc2_R - Pc1_R).norm();
    // double d_cam_Q = (Pc2_Q - Pc1_Q).norm();

    // // ==================== 7. 输出 ====================
    // std::cout << "\n========== Distance Error Test ==========\n";

    // std::cout << "World distance          = " << d_world << std::endl;
    // std::cout << "Camera distance (R_xy)  = " << d_cam_R << std::endl;
    // std::cout << "Camera distance (SO2 Q) = " << d_cam_Q << std::endl;

    // std::cout << "Distance error (abs)    = " << std::abs(d_cam_R - d_cam_Q) << std::endl;

    // std::cout << "Distance error (ratio)  = " << std::abs(d_cam_R / d_cam_Q - 1.0) << std::endl;

    // std::cout << "========================================\n";
    // -------------------- 5. 构建平台坐标系在临时世界下的旋转矩阵 --------------------
    Eigen::Vector3d zdir = xdir.cross(ydir).normalized();
    Eigen::Matrix3d R_plat_world;
    R_plat_world.col(0) = xdir.normalized();
    R_plat_world.col(1) = ydir.normalized();
    R_plat_world.col(2) = zdir;
    Eigen::Vector3d t_plat_world(C.x(), C.y(), 0.0);

    // -------------------- 6. 临时世界 -> 相机坐标系 --------------------
    Eigen::Matrix3d R_world_cam23;
    cv::Mat rvec_cv2(3, 1, CV_64F), R_cv2(3, 3, CV_64F);
    for (int i = 0; i < 3; ++i) rvec_cv2.at<double>(i, 0) = v_rot_(i);
    cv::Rodrigues(rvec_cv2, R_cv2);
    cv::cv2eigen(R_cv2, R_world_cam23);
    Eigen::Vector3d t_world_cam = v_trans_;

    // -------------------- 7. 平台坐标系 -> 相机坐标系 --------------------
    Eigen::Matrix3d R_plat_cam = R_world_cam23 * R_plat_world;
    Eigen::Vector3d t_plat_cam = R_world_cam23 * t_plat_world + t_world_cam;
    std::cout << "平台 -> 相机旋转矩阵 = \n" << R_plat_cam << "\n";
    std::cout << "平台 -> 相机平移向量 = " << t_plat_cam.transpose() << "\n";

    // -------------------- 8. 平台到相机旋转向量 --------------------
    cv::Mat R_plat_cv, rvec_plat_cv;
    cv::eigen2cv(R_plat_cam, R_plat_cv);
    cv::Rodrigues(R_plat_cv, rvec_plat_cv);
    Eigen::Vector3d rvec_plat;
    for (int i = 0; i < 3; ++i) rvec_plat(i) = rvec_plat_cv.at<double>(i, 0);

    // -------------------- 给定像素点转换到平台坐标系 --------------------
    Eigen::MatrixXd px(1, 2);
    px(0, 0) = 6937.323892;
    px(0, 1) = 2816.240515;

    // 像素 -> 相机平面坐标
    Eigen::MatrixXd cam_norm = lineCalib_->pixelToCameraCoordinates(px, K_, dist_);
    std::cout << std::fixed << std::setprecision(12);
    std::cout << "cam_norm =\n" << cam_norm << std::endl;

    // 直接用平台到相机的外参进行相机 -> 平台坐标系转换
    Eigen::MatrixXd plat_pts = lineCalib_->cameraToWorldCoordinates(cam_norm, rvec_plat, t_plat_cam);
    // 直接用平台到相机的外参进行相机 -> 平台坐标系转换
    Eigen::MatrixXd plat_pts2 = lineCalib_->cameraToWorldCoordinates(cam_norm, rvec_opt, tvec_opt);
    // 输出结果
    std::cout << "像素点在平台坐标系 = " << plat_pts(0, 0) << ", " << plat_pts(0, 1) << "\n";
    // 输出结果
    std::cout << "像素点在平台坐标系2 = " << plat_pts2(0, 0) << ", " << plat_pts2(0, 1) << "\n";
    std::cout << "\nRecovered rotation vector (platform -> camera): " << rvec_plat.transpose() << " [rad]" << std::endl;
    std::cout << "\nTrans vector (platform -> camera): " << t_plat_cam.transpose() << std::endl;
    std::cout << "\nRecovered rotation vector (platform -> camera): " << rvec_opt.transpose() << " [rad]" << std::endl;
    std::cout << "\nTrans vector (platform -> camera): " << tvec_opt.transpose() << std::endl;
}

bool TelecentricPlatformCalib::estimatePlatformPoseFromBoards(const std::vector<std::vector<Eigen::Vector2d>>& xWorlds,
                                                              const std::vector<std::vector<Eigen::Vector2d>>& yWorlds,
                                                              const std::vector<std::vector<Eigen::Vector2d>>& rotWorlds,
                                                              bool inputIsCamCoords, Eigen::Vector3d& vRotPlat,
                                                              Eigen::Vector3d& vTransPlat) {
    if (xWorlds.size() < 1 || yWorlds.size() < 1 || rotWorlds.size() < 2) return false;

    // === 1. xdir: 相邻 X 组的平均位移 ===
    Eigen::Vector2d xdir2d = Eigen::Vector2d::Zero();
    int xCnt = 0;
    for (size_t k = 1; k < xWorlds.size(); ++k) {
        double sum_dx = 0, sum_dy = 0;
        int n = std::min(xWorlds[k - 1].size(), xWorlds[k].size());
        for (int i = 0; i < n; ++i) {
            sum_dx += xWorlds[k][i].x() - xWorlds[k - 1][i].x();
            sum_dy += xWorlds[k][i].y() - xWorlds[k - 1][i].y();
        }
        xdir2d += Eigen::Vector2d(sum_dx / n, sum_dy / n);
        ++xCnt;
    }
    if (xCnt == 0) {
        std::cerr << "X 方向需要至少 2 张图" << std::endl;
        return false;
    }
    xdir2d /= xCnt;

    // === 2. ydir: 相邻 Y 组的平均位移 ===
    Eigen::Vector2d ydir2d = Eigen::Vector2d::Zero();
    int yCnt = 0;
    for (size_t k = 1; k < yWorlds.size(); ++k) {
        double sum_dx = 0, sum_dy = 0;
        int n = std::min(yWorlds[k - 1].size(), yWorlds[k].size());
        for (int i = 0; i < n; ++i) {
            sum_dx += yWorlds[k][i].x() - yWorlds[k - 1][i].x();
            sum_dy += yWorlds[k][i].y() - yWorlds[k - 1][i].y();
        }
        ydir2d += Eigen::Vector2d(sum_dx / n, sum_dy / n);
        ++yCnt;
    }
    if (yCnt == 0) {
        std::cerr << "Y 方向需要至少 2 张图" << std::endl;
        return false;
    }
    ydir2d /= yCnt;

    Eigen::Vector3d xdir(xdir2d.x(), xdir2d.y(), 0.0);
    Eigen::Vector3d ydir(ydir2d.x(), ydir2d.y(), 0.0);
    std::cout << "X方向(累计): " << xdir2d.norm() << "mm, dir=" << xdir.head<2>().normalized().transpose() << std::endl;
    std::cout << "Y方向(累计): " << ydir2d.norm() << "mm, dir=" << ydir.head<2>().normalized().transpose() << std::endl;

    if (xdir2d.norm() < 1e-6 || ydir2d.norm() < 1e-6) {
        std::cerr << "X 或 Y 方向位移为零，无法确定平台姿态" << std::endl;
        return false;
    }

    // === 3. C₀: 星型 — 全部对 rot0 求旋转中心 ===
    Eigen::Vector2d C0 = Eigen::Vector2d::Zero();
    {
        int count = 0;
        for (size_t k = 1; k < rotWorlds.size(); ++k) {
            std::vector<std::vector<Eigen::Vector2d>> pair = {rotWorlds[0], rotWorlds[k]};
            Eigen::Vector2d Ck = computeRotationCenterSequential(pair);
            if (Ck.norm() < 1e6) { C0 += Ck; ++count; }
        }
        if (count > 0) C0 /= count;
    }
    std::cout << "旋转中心 C₀ = " << C0.transpose() << std::endl;

    // === 4. 构建平台→相机位姿 ===
    Eigen::Vector3d zdir = xdir.cross(ydir).normalized();
    Eigen::Matrix3d R_plat_local;
    R_plat_local.col(0) = xdir.normalized();
    R_plat_local.col(1) = ydir.normalized();
    R_plat_local.col(2) = zdir;
    Eigen::Vector3d t_plat_local(C0.x(), C0.y(), 0.0);

    Eigen::Matrix3d R_plat_cam;
    Eigen::Vector3d t_plat_cam;

    if (inputIsCamCoords) {
        // 输入已是相机坐标，直接输出
        R_plat_cam = R_plat_local;
        t_plat_cam = t_plat_local;
    } else {
        // 输入是世界坐标，转到相机坐标
        Eigen::Matrix3d R_world_cam;
        cv::Mat rvec_cv(3, 1, CV_64F), R_cv(3, 3, CV_64F);
        for (int i = 0; i < 3; ++i) rvec_cv.at<double>(i, 0) = v_rot_(i);
        cv::Rodrigues(rvec_cv, R_cv);
        cv::cv2eigen(R_cv, R_world_cam);
        Eigen::Vector3d t_world_cam = v_trans_;

        R_plat_cam = R_world_cam * R_plat_local;
        t_plat_cam.setZero();
        t_plat_cam.head<2>() = R_world_cam.topLeftCorner<2, 2>() * t_plat_local.head<2>() + t_world_cam.head<2>();
    }

    // === 5. 转 Rodrigues 输出 ===
    cv::Mat R_plat_cv, rvec_plat_cv;
    cv::eigen2cv(R_plat_cam, R_plat_cv);
    cv::Rodrigues(R_plat_cv, rvec_plat_cv);
    for (int i = 0; i < 3; ++i) vRotPlat(i) = rvec_plat_cv.at<double>(i, 0);
    vTransPlat = t_plat_cam;

    return true;
}

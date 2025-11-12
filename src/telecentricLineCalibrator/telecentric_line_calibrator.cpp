#include "telecentric_line_calibrator.h"

#include <iomanip>

// =========================================================
// 构造函数
// =========================================================
TelecentricLineCalibrator::TelecentricLineCalibrator() {
    K_.setIdentity();
    coff_dis_ = Eigen::VectorXd::Zero(5);
}
bool TelecentricLineCalibrator::calculate_Image_Points(cv::Mat imageInput, cv::Size boardSize,
                                                       std::vector<cv::Point2d>& imagePoints) {
    // ---------- 1. 灰度化与反色 ----------
    cv::Mat gray;
    if (imageInput.channels() == 3)
        cv::cvtColor(imageInput, gray, cv::COLOR_BGR2GRAY);
    else
        gray = imageInput.clone();
    cv::bitwise_not(gray, gray);  // 若圆为黑底白点可省略

    // ---------- 2. Blob 检测参数 ----------
    cv::SimpleBlobDetector::Params params;
    params.minArea = 1e5;
    params.maxArea = 8e5;
    params.minCircularity = 0.7f;
    params.filterByCircularity = true;
    params.filterByColor = false;
    params.filterByConvexity = false;
    params.filterByInertia = false;

    cv::Ptr<cv::FeatureDetector> blobDetector = cv::SimpleBlobDetector::create(params);

    // ---------- 3. 时间戳 ----------
    std::time_t t = std::time(nullptr);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    std::string timestamp = oss.str();

    // ---------- 4. 圆阵检测 ----------
    bool found =
        cv::findCirclesGrid(gray, boardSize, imagePoints, cv::CALIB_CB_SYMMETRIC_GRID | cv::CALIB_CB_CLUSTERING, blobDetector);

    if (!found) {
        std::cout << "未检测到圆心，结果图已保存为: " << std::endl;
        // 仍按原逻辑提示，但不保存图（保持与你原代码一致的行为）
        return false;
    }

    // ---------- 5. 亚像素精确化 ----------
    cv::Size winSize(10, 10);
    cv::Size zeroZone(1, 1);
    cv::TermCriteria tc(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 100, 1e-3);
    cv::cornerSubPix(gray, imagePoints, winSize, zeroZone, tc);

    // ---------- 6. 点序规范（保持最终为：上→下、左→右，行优先） ----------
    const int W = boardSize.width;
    const int H = boardSize.height;
    if (imagePoints.size() == static_cast<size_t>(W * H)) {
        auto mean_row_y = [&](int r, const std::vector<cv::Point2d>& P) -> double {
            double s = 0.0;
            for (int c = 0; c < W; ++c) s += P[r * W + c].y;
            return s / W;
        };
        auto row_y_variance_sum = [&](const std::vector<cv::Point2d>& P) -> double {
            double total = 0.0;
            for (int r = 0; r < H; ++r) {
                double mu = mean_row_y(r, P);
                double var = 0.0;
                for (int c = 0; c < W; ++c) {
                    double dy = P[r * W + c].y - mu;
                    var += dy * dy;
                }
                total += var / W;
            }
            return total;
        };

        // 6.1 列优先 → 行优先（必要时做转置）
        double rowVar_A = row_y_variance_sum(imagePoints);
        std::vector<cv::Point2d> transposed(imagePoints.size());
        for (int r = 0; r < H; ++r)
            for (int c = 0; c < W; ++c) transposed[r * W + c] = imagePoints[c * H + r];
        double rowVar_B = row_y_variance_sum(transposed);
        if (rowVar_B + 1e-6 < rowVar_A) {
            imagePoints.swap(transposed);
            std::cout << " 检测到列优先 → 已转置为行优先\n";
        }

        // 6.2 每行保证 x 递增（左→右）
        for (int r = 0; r < H; ++r) {
            int idx = r * W;
            int inc = 0;
            for (int c = 0; c < W - 1; ++c)
                if (imagePoints[idx + c + 1].x > imagePoints[idx + c].x) ++inc;
            if (inc < (W - 1) / 2) {  // 多数递减 → 镜像一行
                for (int c = 0; c < W / 2; ++c) std::swap(imagePoints[idx + c], imagePoints[idx + (W - 1 - c)]);
                std::cout << " 行 " << r << " 发生镜像，已纠正为 x 递增\n";
            }
        }

        // 6.3 整体保证第一行为“更靠上”（上→下）
        double mean_top_y = 0.0, mean_bottom_y = 0.0;
        for (int c = 0; c < W; ++c) {
            mean_top_y += imagePoints[c].y;
            mean_bottom_y += imagePoints[(H - 1) * W + c].y;
        }
        mean_top_y /= W;
        mean_bottom_y /= W;
        if (mean_top_y > mean_bottom_y) {
            // 反转所有行的顺序（上下翻转）
            for (int r = 0; r < H / 2; ++r) {
                for (int c = 0; c < W; ++c) {
                    std::swap(imagePoints[r * W + c], imagePoints[(H - 1 - r) * W + c]);
                }
            }
            std::cout << " 整体做了上下翻转，统一为上→下\n";
        }
    }

    // ---------- 7. 绘制结果（保存路径与命名按你的原程序） ----------
    cv::Mat drawImg = imageInput.clone();
    cv::drawChessboardCorners(drawImg, boardSize, imagePoints, found);
    std::string gridFile = "./data/CISCamera_Image/img/Detected_Grid_" + timestamp + ".png";
    cv::imwrite(gridFile, drawImg);

    // ---------- 8. 日志输出（同原程序） ----------
    std::string txtFile = "./data/CISCamera_Image/img/Detected_Points_" + timestamp + ".txt";
    std::ofstream ofs(txtFile);
    if (ofs.is_open()) {
        ofs << "# Index\tX\tY\n";
        for (size_t i = 0; i < imagePoints.size(); ++i) ofs << i << "\t" << imagePoints[i].x << "\t" << imagePoints[i].y << "\n";
        ofs.close();
    }

    std::cout << "检测完成，结果文件已保存：\n"
              << "  - 圆心检测图: " << gridFile << "\n"
              << "  - 圆心坐标表: " << txtFile << std::endl;

    return true;
}

// =========================================================
// Step 1. 计算单应矩阵（DLT）
// =========================================================
Eigen::Matrix3d TelecentricLineCalibrator::computeHomography(const std::vector<Eigen::Vector2d>& worldPts,
                                                             const std::vector<Eigen::Vector2d>& imagePts) {
    // --- 检查输入 ---
    assert(worldPts.size() == imagePts.size() && worldPts.size() >= 3);
    const int n = static_cast<int>(worldPts.size());

    // --- 构造线性方程 A h = b ---
    // 其中 h = [h11, h12, h13, h21, h22, h23]^T
    // 对每个点 (X, Y) ↔ (u, v)，有：
    // u = h11*X + h12*Y + h13
    // v = h21*X + h22*Y + h23
    //
    // 于是：
    // [X Y 1 0 0 0] [h] = u
    // [0 0 0 X Y 1] [h] = v

    Eigen::MatrixXd A(2 * n, 6);
    Eigen::VectorXd b(2 * n);

    for (int i = 0; i < n; ++i) {
        double X = worldPts[i].x();
        double Y = worldPts[i].y();
        double u = imagePts[i].x();
        double v = imagePts[i].y();

        A.row(2 * i) << X, Y, 1, 0, 0, 0;
        A.row(2 * i + 1) << 0, 0, 0, X, Y, 1;
        b(2 * i) = u;
        b(2 * i + 1) = v;
    }

    // --- 最小二乘求解 h ---
    Eigen::VectorXd h = (A.transpose() * A).ldlt().solve(A.transpose() * b);

    // --- 组装 3×3 仿射单应矩阵（第三行固定 [0 0 1]）---
    Eigen::Matrix3d H;
    H << h(0), h(1), h(2), h(3), h(4), h(5), 0.0, 0.0, 1.0;
    // 打印结果
    // std::cout << "\n========= 单应矩阵 H =========\n";
    // std::cout << std::fixed << std::setprecision(6);
    // std::cout << H << "\n";
    // std::cout << "==============================\n";
    return H;
}
// =========================================================
// 辅助：按论文式(10)由 H 求单幅 m_i（取非负根）:contentReference[oaicite:4]{index=4}
// =========================================================
double TelecentricLineCalibrator::compute_m_from_H(const Eigen::Matrix3d& H, double dx, double dy) {
    const double h11 = H(0, 0), h12 = H(0, 1), h13 = H(0, 2);
    const double h21 = H(1, 0), h22 = H(1, 1), h23 = H(1, 2);

    const double num = (1.0 / (dy * dy)) * (h11 * h11 + h12 * h12) - std::pow(h11 * h22 - h12 * h21, 2.0);
    const double den = (1.0 / (dy * dy)) - (h21 * h21 + h22 * h22);

    if (std::abs(den) < 1e-12 || num / den < 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    double m = std::sqrt(num / den) * dx;  // 取非负根
    return m;
}
// =========================================================
// Step 2. 从单应矩阵提取姿态
// =========================================================
Pose TelecentricLineCalibrator::extractPoseFromHomography(const Eigen::Matrix3d& H, const Eigen::Matrix3d& K,
                                                          const std::vector<Eigen::Vector2d>& worldPts,
                                                          const std::vector<Eigen::Vector2d>& imagePts, const double m) {
    Pose pose;

    const double u0 = u0_;
    const double v0 = v0_;
    const double dx = dx_, dy = dy_;

    const double h11 = H(0, 0), h12 = H(0, 1), h13 = H(0, 2);
    const double h21 = H(1, 0), h22 = H(1, 1), h23 = H(1, 2);

    // === Step 1. 式(11)基础分量 ===
    const double r11 = h11 * (dx / m);
    const double r12 = h12 * (dx / m);
    const double r21 = h21 * dy;
    const double r22 = h22 * dy;
    const double tx = (h13 - u0) * (dx / m);
    const double ty = (h23 - v0 / m) * dy;

    const double abs_r13 = std::sqrt(std::max(0.0, 1.0 - (r11 * r11 + r21 * r21)));
    const double abs_r23 = std::sqrt(std::max(0.0, 1.0 - (r12 * r12 + r22 * r22)));

    // === Step 2. 用附录C第二公式预测符号 ===
    int sign_r13_th = +1;
    int sign_r23_th = +1;
    if (!worldPts.empty()) {
        const Eigen::Vector2d& ptW = worldPts[worldPts.size() / 2];
        const Eigen::Vector2d& ptI = imagePts[imagePts.size() / 2];

        Eigen::Vector3d Pw(ptW.x(), ptW.y(), 1.0);
        Eigen::Vector3d uv(ptI.x(), ptI.y(), 1.0);
        Eigen::Matrix3d A;
        A << r11, r12, tx, r21, r22, ty, 0.0, 0.0, 1.0;

        Eigen::Vector3d diff = K.inverse() * uv - A * Pw;

        // 取符号（仅关心正负）
        sign_r13_th = (diff(0) >= 0.0 ? +1 : -1);
        sign_r23_th = (diff(1) >= 0.0 ? +1 : -1);
    }

    // === Step 3. 枚举所有符号组合，评估正交与重投影 ===
    Eigen::Matrix3d bestR = Eigen::Matrix3d::Identity();
    Eigen::Vector3d bestt(tx, ty, 0.0);
    double bestScore = std::numeric_limits<double>::infinity();
    int best_r13_sgn = +1, best_r23_sgn = +1;

    auto evaluate_combo = [&](double r13, double r23) {
        Eigen::Vector3d r1(r11, r21, r13);
        Eigen::Vector3d r2(r12, r22, r23);
        if (r1.norm() < 1e-12 || r2.norm() < 1e-12) return std::numeric_limits<double>::infinity();
        r1.normalize();
        r2.normalize();
        Eigen::Vector3d r3 = r1.cross(r2);
        if (r3.norm() < 1e-12) return std::numeric_limits<double>::infinity();
        r3.normalize();
        if (r3.z() < 0) {
            r1 = -r1;
            r2 = -r2;
            r3 = -r3;
        }

        double orth_err = std::abs(r1.dot(r2));

        return orth_err;
    };

    for (int s1sgn : {-1, +1}) {
        for (int s2sgn : {-1, +1}) {
            double score = evaluate_combo(s1sgn * abs_r13, s2sgn * abs_r23);
            if (score < bestScore) {
                bestScore = score;
                best_r13_sgn = s1sgn;
                best_r23_sgn = s2sgn;

                Eigen::Vector3d r1(r11, r21, s1sgn * abs_r13);
                Eigen::Vector3d r2(r12, r22, s2sgn * abs_r23);
                r1.normalize();
                r2.normalize();
                Eigen::Vector3d r3 = r1.cross(r2);
                r3.normalize();
                if (r3.z() < 0) {
                    r1 = -r1;
                    r2 = -r2;
                    r3 = -r3;
                }
                bestR.col(0) = r1;
                bestR.col(1) = r2;
                bestR.col(2) = r3;
            }
        }
    }

    // // == = Step 4. 符号一致性判断 == =
    // if (best_r13_sgn == sign_r13_th && best_r23_sgn == sign_r23_th) {
    //     std::cout << "外参初始化正常（符号一致）" << std::endl;
    // } else {
    //     std::cout << "外参符号自动修正（论文符号不稳定或数据噪声）" << std::endl;
    // }
    // double score_pred = evaluate_combo(sign_r13_th * abs_r13, sign_r23_th * abs_r23);
    // double score_best = evaluate_combo(best_r13_sgn * abs_r13, best_r23_sgn * abs_r23);

    // std::cout << "---------------------------------------------\n";
    // std::cout << "符号预测 vs 最优方案对比:\n";
    // std::cout << "  预测符号 (r13, r23): (" << sign_r13_th << ", " << sign_r23_th << ")\n";
    // std::cout << "  最优符号 (r13, r23): (" << best_r13_sgn << ", " << best_r23_sgn << ")\n";
    // std::cout << "  理论 abs(r13) = " << abs_r13 << ", abs(r23) = " << abs_r23 << "\n";
    // std::cout << "  evaluate_combo(预测) = " << score_pred << "\n";
    // std::cout << "  evaluate_combo(最优) = " << score_best << "\n";
    // std::cout << "  正交性改进 = " << (score_pred - score_best) << "\n";
    // std::cout << "---------------------------------------------\n";

    // === Step 5. 通过重投影误差判断正反旋转 ===
    Pose poseA, poseB;
    poseA.R = bestR;
    poseA.t = Eigen::Vector3d(tx, ty, 0.0);

    poseB.R = -bestR;
    poseB.t = Eigen::Vector3d(tx, ty, 0.0);  // 平移不变，只翻转R

    double errA = computeReprojectionError(worldPts, imagePts, poseA, K);
    double errB = computeReprojectionError(worldPts, imagePts, poseB, K);

    if (errA <= errB)
        pose = poseA;
    else
        pose = poseB;

    return pose;
}
// =========================================================
// Step 3. 初始化内参
// =========================================================
Eigen::Matrix3d TelecentricLineCalibrator::initIntrinsic(double m, double dx, double dy, double u0, double v0) {
    Eigen::Matrix3d K;
    // === 3️ 构建初始内参矩阵 ===
    K << m / dx, 0.0, u0, 0.0, 1 / dy, v0 / m, 0.0, 0.0, 1.0;
    return K;
}
bool TelecentricLineCalibrator::estimateIntrinsicsFromHomographies(const std::vector<Eigen::Matrix3d>& Hs, double dx, double dy,
                                                                   double u0, double v0,
                                                                   const std::vector<double>& reprojErrors) {
    std::vector<double> ms;
    ms.reserve(Hs.size());
    for (const auto& H : Hs) {
        double mi = compute_m_from_H(H, dx, dy);
        if (std::isfinite(mi) && mi > 0.0)
            ms.push_back(mi);
        else
            ms.push_back(std::numeric_limits<double>::quiet_NaN());
    }
    if (ms.empty()) return false;

    // ---------- 使用误差加权平均 ----------
    // 权重 = 1 / (err + ε)，防止除零
    double eps = 1e-6;
    double weightedSum = 0.0;
    double weightTotal = 0.0;
    for (size_t i = 0; i < ms.size(); ++i) {
        if (!std::isfinite(ms[i])) continue;
        double w = 1.0 / (reprojErrors[i] + eps);
        weightedSum += ms[i] * w;
        weightTotal += w;
    }
    if (weightTotal <= 0.0) return false;

    m_ = weightedSum / weightTotal;
    dx_ = dx;
    dy_ = dy;

    K_ = initIntrinsic(m_, dx_, dy_, u0, v0);

    // std::cout << "\n--- 误差加权平均得到的放大倍率 m = " << m_ << " ---\n";
    // std::cout << "内参矩阵 K =\n" << K_ << "\n";
    return true;
}

// =========================================================
// Step 4. 计算重投影误差
// =========================================================
double TelecentricLineCalibrator::computeReprojectionError(const std::vector<Eigen::Vector2d>& worldPts,
                                                           const std::vector<Eigen::Vector2d>& imagePts, const Pose& pose,
                                                           const Eigen::Matrix3d& K, const std::string& savePath) {
    const int n = static_cast<int>(worldPts.size());
    if (n == 0) return 0.0;
    double totalErr = 0.0;

    // 取旋转矩阵前两列（平面旋转部分）
    Eigen::Matrix2d R2 = pose.R.block<2, 2>(0, 0);
    Eigen::Vector2d t2 = pose.t.head<2>();  // 仅取平面平移
    // 如果需要保存重投影结果
    std::ofstream ofs;
    if (savePath != " " && !savePath.empty()) {
        ofs.open(savePath);
        if (!ofs.is_open()) {
            std::cerr << "[Error] 无法打开保存路径: " << savePath << std::endl;
        } else {
            ofs << "# Index\tX\tY\n";  // 表头
        }
    }
    for (int i = 0; i < n; ++i) {
        // 世界坐标点（Z=0）

        // 仿射成像模型： [u v 1]^T ~ K * [R|t] * [X Y 1]^T
        Eigen::Vector3d xy1;
        xy1 << worldPts[i].x(), worldPts[i].y(), 1.0;

        Eigen::Vector3d img_affine;
        img_affine.head<2>() = R2 * xy1.head<2>() + t2;
        img_affine(2) = 1.0;

        Eigen::Vector3d uvw = K * img_affine;
        Eigen::Vector2d uv_hat(uvw(0) / uvw(2), uvw(1) / uvw(2));

        Eigen::Vector2d uv(imagePts[i].x(), imagePts[i].y());
        totalErr += std::sqrt((uv - uv_hat).squaredNorm());
        if (ofs.is_open()) {
            ofs << i << "\t" << std::fixed << std::setprecision(6) << uv_hat.x() << "\t" << uv_hat.y() << "\n";
        }
    }
    if (ofs.is_open()) ofs.close();
    return totalErr / n;
}
double TelecentricLineCalibrator::computeReprojectionErrorFinal(const std::vector<Eigen::Vector2d>& worldPts,
                                                                const std::vector<Eigen::Vector2d>& imagePts, const Pose& pose,
                                                                const Eigen::Matrix3d& K, double k) {
    const int n = static_cast<int>(worldPts.size());
    if (n == 0) {
        std::cerr << "[Error] worldPts is empty!\n";
        return 0.0;
    }
    if (imagePts.size() != worldPts.size()) {
        std::cerr << "[Error] worldPts.size() != imagePts.size(), " << worldPts.size() << " vs " << imagePts.size() << "\n";
        return 0.0;
    }

    // 检查旋转矩阵与平移向量维度
    if (pose.R.rows() < 2 || pose.R.cols() < 2 || pose.t.size() < 2) {
        std::cerr << "[Error] Invalid pose dimensions\n";
        return 0.0;
    }

    double totalErr = 0.0;

    Eigen::Matrix2d R2 = pose.R.block<2, 2>(0, 0);
    Eigen::Vector2d t2 = pose.t.head<2>();

    double v0__ = v0_;
    Eigen::VectorXd residuals(2 * n);
    residuals.setZero();

    int res_idx = 0;
    for (int i = 0; i < n; ++i) {
        const Eigen::Vector2d& Pw2d = worldPts[i];
        const Eigen::Vector2d& img_pt = imagePts[i];

        if (!Pw2d.allFinite() || !img_pt.allFinite()) {
            std::cerr << "[Warning] NaN in input points at index " << i << "\n";
            continue;
        }

        // 仿射变换：世界 -> 相机平面
        Eigen::Vector2d img_affine = R2 * Pw2d + t2;
        if (!img_affine.allFinite()) {
            std::cerr << "[Warning] img_affine NaN at " << i << "\n";
            continue;
        }

        // 齐次投影
        Eigen::Vector3d uvw = K * Eigen::Vector3d(img_affine(0), img_affine(1), 1.0);
        if (std::abs(uvw(2)) < 1e-12) {
            std::cerr << "[Warning] Division by zero at point " << i << ", uvw=" << uvw.transpose() << "\n";
            continue;
        }

        // 归一化
        double x_u = uvw(0) / uvw(2);
        double y_u = uvw(1) / uvw(2);
        if (!std::isfinite(x_u) || !std::isfinite(y_u)) {
            std::cerr << "[Warning] Invalid normalized coordinates at " << i << "\n";
            continue;
        }

        // Telecentric 专用畸变
        double r2 = x_u * x_u + std::pow(v0__ * dy_, 2);
        double delta_x = k * x_u * r2;
        double delta_y = -k * v0__ * dy_ * r2;

        double u_pred = x_u + delta_x;
        double v_pred = y_u + delta_y;

        if (res_idx + 1 >= residuals.size()) {
            std::cerr << "[Error] residuals index overflow! res_idx=" << res_idx << "\n";
            break;
        }

        residuals[res_idx++] = u_pred - img_pt.x();
        residuals[res_idx++] = v_pred - img_pt.y();
    }

    if (res_idx == 0) {
        std::cerr << "[Error] No valid residual computed!\n";
        return 0.0;
    }

    residuals.conservativeResize(res_idx);  // 去掉无效部分
    double rmse = std::sqrt(residuals.squaredNorm() / (res_idx / 2));
    if (!std::isfinite(rmse)) {
        std::cerr << "[Error] Computed RMSE is NaN\n";
        return 0.0;
    }

    return rmse;
}
// -------------------- 齐次坐标 --------------------
Eigen::MatrixXd TelecentricLineCalibrator::toHomogeneous(const Eigen::MatrixXd& points) {
    if (points.cols() != 2 && points.cols() != 3) throw std::runtime_error("齐次坐标转换失败，输入应为 Nx2 或 Nx3");

    Eigen::MatrixXd homo(points.rows(), points.cols() + 1);
    homo.leftCols(points.cols()) = points;
    homo.col(points.cols()).setOnes();
    return homo;
}

// -------------------- 正向畸变 --------------------
Eigen::MatrixXd TelecentricLineCalibrator::distort(const Eigen::Matrix<double, 1, 5>& coff_dis,
                                                   const Eigen::MatrixXd& normalized_proj) {
    double k1 = coff_dis(0), h1 = coff_dis(1), h2 = coff_dis(2), s1 = coff_dis(3), s2 = coff_dis(4);
    Eigen::VectorXd x = normalized_proj.col(0);
    Eigen::VectorXd y = normalized_proj.col(1);
    Eigen::VectorXd r = x.array().square() + y.array().square();

    Eigen::VectorXd deltaX = k1 * x.array() * r.array() + h1 * (3 * x.array().square() + y.array().square()) +
                             2 * h2 * x.array() * y.array() + s1 * r.array();
    Eigen::VectorXd deltaY = k1 * y.array() * r.array() + 2 * h1 * x.array() * y.array() +
                             h2 * (x.array().square() + 3 * y.array().square()) + s2 * r.array();

    Eigen::MatrixXd distorted(normalized_proj.rows(), 2);
    distorted.col(0) = x + deltaX;
    distorted.col(1) = y + deltaY;

    return toHomogeneous(distorted);
}

// -------------------- 迭代去畸变 --------------------
Eigen::MatrixXd TelecentricLineCalibrator::undistortPointsIter(const Eigen::MatrixXd& points_px, const Eigen::Matrix3d& K,
                                                               const Eigen::Matrix<double, 1, 5>& coff_dis, int max_iter,
                                                               double tol) {
    if (points_px.cols() != 2) throw std::runtime_error("输入点应为 Nx2");

    Eigen::MatrixXd undistorted(points_px.rows(), 2);
    double fx = K(0, 0), fy = K(1, 1), cx = K(0, 2), cy = K(1, 2);
    double ifx = 1.0 / fx, ify = 1.0 / fy;
    double k1 = coff_dis(0), h1 = coff_dis(1), h2 = coff_dis(2), s1 = coff_dis(3), s2 = coff_dis(4);

    for (int i = 0; i < points_px.rows(); ++i) {
        double u = points_px(i, 0), v = points_px(i, 1);
        double x0 = (u - cx) * ifx;
        double y0 = (v - cy) * ify;
        double x = x0, y = y0;

        double error = 1e9;
        int iter = 0;

        while (iter < max_iter && error > tol) {
            double r = x * x + y * y;
            double deltaX = k1 * x * r + h1 * (3 * x * x + y * y) + 2 * h2 * x * y + s1 * r;
            double deltaY = k1 * y * r + 2 * h1 * x * y + h2 * (x * x + 3 * y * y) + s2 * r;

            x = x0 - deltaX;
            y = y0 - deltaY;

            r = x * x + y * y;
            double xd = x + k1 * x * r + h1 * (3 * x * x + y * y) + 2 * h2 * x * y + s1 * r;
            double yd = y + k1 * y * r + 2 * h1 * x * y + h2 * (x * x + 3 * y * y) + s2 * r;

            error = std::sqrt(std::pow(xd * fx + cx - u, 2) + std::pow(yd * fy + cy - v, 2));
            iter++;
        }
        undistorted(i, 0) = x * fx + cx;
        undistorted(i, 1) = y * fy + cy;
    }

    return undistorted;
}

// -------------------- 像素坐标 → 相机坐标 --------------------
Eigen::MatrixXd TelecentricLineCalibrator::pixelToCameraCoordinates(const Eigen::MatrixXd& points_px, const Eigen::Matrix3d& K,
                                                                    const Eigen::Matrix<double, 1, 5>& coff_dis) {
    Eigen::MatrixXd processed_px = points_px;

    // 如果畸变系数存在有效值则去畸变
    if (coff_dis.norm() > 1e-15) {
        processed_px = undistortPointsIter(points_px, K, coff_dis);
    }

    Eigen::MatrixXd homo = toHomogeneous(processed_px);
    Eigen::Matrix3d K_inv = K.inverse();
    Eigen::MatrixXd cam_pts = (K_inv * homo.transpose()).transpose();

    Eigen::MatrixXd result(cam_pts.rows(), 2);
    result.col(0) = cam_pts.col(0).array() / cam_pts.col(2).array();
    result.col(1) = cam_pts.col(1).array() / cam_pts.col(2).array();
    return result;
}
Pose TelecentricLineCalibrator::estimateTelecentricPose(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& coff_dis,
                                                        const std::vector<Eigen::Vector2d>& worldPts,
                                                        const std::vector<Eigen::Vector2d>& imgPts) {
    if (worldPts.size() != imgPts.size() || worldPts.size() < 3) {
        throw std::runtime_error("点数不足或数量不匹配！");
    }

    // ---------- 1. 转为 Eigen 矩阵 ----------
    Eigen::MatrixXd pts_px(worldPts.size(), 2);
    Eigen::MatrixXd pts_w(worldPts.size(), 2);
    for (size_t i = 0; i < worldPts.size(); ++i) {
        pts_px.row(i) = imgPts[i].transpose();
        pts_w.row(i) = worldPts[i].transpose();
    }

    // ---------- 2. 像素 → 相机坐标 ----------
    Eigen::MatrixXd camPts = pixelToCameraCoordinates(pts_px, K, coff_dis);

    // ---------- 3. 构造线性方程求仿射 ----------
    Eigen::MatrixXd A(2 * worldPts.size(), 6);
    Eigen::VectorXd b(2 * worldPts.size());
    for (int i = 0; i < pts_w.rows(); ++i) {
        double X = pts_w(i, 0), Y = pts_w(i, 1);
        A.row(2 * i) << X, Y, 1, 0, 0, 0;
        A.row(2 * i + 1) << 0, 0, 0, X, Y, 1;
        b(2 * i) = camPts(i, 0);
        b(2 * i + 1) = camPts(i, 1);
    }

    // ---------- 4. 最小二乘解 ----------
    Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);

    // ---------- 5. 提取 R 和 t ----------
    Eigen::Matrix2d R2;
    Eigen::Vector2d t2;
    R2 << x(0), x(1), x(3), x(4);
    t2 << x(2), x(5);

    // ---------- 6. 正交化旋转 ----------
    Eigen::JacobiSVD<Eigen::Matrix2d> svd(R2, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix2d R_ortho = svd.matrixU() * svd.matrixV().transpose();

    // ---------- 7. 扩展为 3x3 ----------
    Pose pose;
    pose.R.setIdentity();
    pose.R.block<2, 2>(0, 0) = R_ortho;
    pose.t.setZero();
    pose.t.head<2>() = t2;

    // ---------- ✅ 新增：计算旋转向量 ----------
    cv::Mat R_cv, rvec;
    cv::eigen2cv(pose.R, R_cv);
    cv::Rodrigues(R_cv, rvec);

    // ---------- 8. 打印结果 ----------
    std::cout << "\n=== Telecentric Pose ===" << std::endl;
    std::cout << "R = \n" << pose.R << std::endl;
    std::cout << "t = \n" << pose.t.transpose() << std::endl;
    std::cout << "rvec = " << rvec.t() << std::endl;  // <--- 打印旋转向量

    // ---------- 9. 计算重投影误差 ----------
    double totalErr = 0.0;
    for (int i = 0; i < pts_w.rows(); ++i) {
        Eigen::Vector3d xy1(pts_w(i, 0), pts_w(i, 1), 1.0);
        Eigen::Vector2d proj = (R_ortho * xy1.head<2>() + t2);
        Eigen::Vector2d cam = camPts.row(i);
        double err = (proj - cam).norm();
        totalErr += err;
        std::cout << "点 " << i << "  世界: [" << pts_w(i, 0) << ", " << pts_w(i, 1) << "]  → 投影: [" << proj.x() << ", "
                  << proj.y() << "]  误差: " << err << std::endl;
    }

    std::cout << "平均重投影误差: " << totalErr / pts_w.rows() << std::endl;

    return pose;
}
void TelecentricLineCalibrator::estimatePosePnP(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& coff_dis,
                                                const std::vector<Eigen::Vector2d>& worldPts,
                                                const std::vector<Eigen::Vector2d>& imgPts) {
    if (worldPts.size() != imgPts.size() || worldPts.size() < 4) {
        std::cerr << "点数量不足或不匹配！" << std::endl;
        return;
    }

    // ---------- 1. 构造输入点 ----------
    std::vector<cv::Point3f> objPts;
    objPts.reserve(worldPts.size());
    for (const auto& p : worldPts) objPts.emplace_back(static_cast<float>(p.x()), static_cast<float>(p.y()), 0.0f);  // Z=0 平面

    std::vector<cv::Point2f> imgPts_cv;
    imgPts_cv.reserve(imgPts.size());
    for (const auto& p : imgPts) imgPts_cv.emplace_back(static_cast<float>(p.x()), static_cast<float>(p.y()));

    // ---------- 2. 转换内参 ----------
    cv::Mat cameraMatrix, distCoeffs;
    cv::eigen2cv(K, cameraMatrix);
    cv::eigen2cv(coff_dis, distCoeffs);

    // ---------- 3. 求解 PnP ----------
    cv::Mat rvec, tvec;
    bool success = cv::solvePnP(objPts, imgPts_cv, cameraMatrix, distCoeffs, rvec, tvec, false, cv::SOLVEPNP_ITERATIVE);

    if (!success) {
        std::cerr << "solvePnP 失败！" << std::endl;
        return;
    }

    // ---------- 4. 输出结果 ----------
    std::cout << "\n=== OpenCV solvePnP 结果 ===" << std::endl;
    std::cout << "rvec = " << rvec.t() << std::endl;
    std::cout << "tvec = " << tvec.t() << std::endl;

    // ---------- 5. 转换为旋转矩阵 ----------
    cv::Mat R;
    cv::Rodrigues(rvec, R);
    std::cout << "R = \n" << R << std::endl;

    // ---------- 6. 重投影 ----------
    std::vector<cv::Point2f> projectedPts;
    cv::projectPoints(objPts, rvec, tvec, cameraMatrix, distCoeffs, projectedPts);

    // ---------- 7. 计算重投影误差 ----------
    double totalErr = 0.0;
    for (size_t i = 0; i < imgPts_cv.size(); ++i) {
        cv::Point2f diff = imgPts_cv[i] - projectedPts[i];
        double err = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        totalErr += err;
        std::cout << "点 " << i << " 世界: [" << objPts[i].x << ", " << objPts[i].y << "]" << " 投影: [" << projectedPts[i].x
                  << ", " << projectedPts[i].y << "]" << " 观测: [" << imgPts_cv[i].x << ", " << imgPts_cv[i].y << "]"
                  << " 误差: " << err << std::endl;
    }

    double meanErr = totalErr / imgPts_cv.size();
    std::cout << "平均重投影误差: " << meanErr << std::endl;
}
// =========================================================
// Step 5. 主函数：标定流程（DLT 初值）
// =========================================================
bool TelecentricLineCalibrator::calibrateCameraFromPointsDemo(const std::vector<std::vector<Eigen::Vector2d>>& all_imgPts,
                                                              const std::vector<Eigen::Vector2d>& worldPts, int width, int height,
                                                              double dx, double dy, Eigen::Matrix3d K_out, double rmse_out,
                                                              std::vector<Pose> poses_out) {
    // std::cout << "检查all_imgPts顺序:\n";
    // for (size_t i = 0; i < all_imgPts.size(); ++i) {
    //     std::cout << "Image " << i << ":\n";
    //     for (size_t j = 0; j < all_imgPts[i].size(); ++j) {
    //         std::cout << "  Point " << j << ": (" << all_imgPts[i][j].x() << ", " << all_imgPts[i][j].y() << ")\n";
    //     }
    // }
    if (all_imgPts.empty()) {
        std::cerr << "输入点集为空！\n";
        return false;
    }

    dx_ = dx;
    dy_ = dy;
    u0_ = width / 2.0;
    v0_ = height / 2.0;

    std::cout << "初始化主点为图像中心 (" << u0_ << ", " << v0_ << ")\n" << std::endl;

    std::vector<Eigen::Matrix3d> homographies;
    std::vector<double> m_values, reprojErrors;

    for (size_t i = 0; i < all_imgPts.size(); ++i) {
        const auto& imgPts = all_imgPts[i];
        if (imgPts.size() != worldPts.size()) {
            std::cerr << "图像 " << i << " 点数不匹配，跳过\n";
            continue;
        }

        // Step 1. 求单应矩阵
        Eigen::Matrix3d Hi = computeHomography(worldPts, imgPts);

        // Step 2. 计算放大倍率
        double mi = compute_m_from_H(Hi, dx, dy);
        if (!std::isfinite(mi) || mi <= 0.0) {
            std::cerr << "图像 " << i << " 的放大倍率无效，跳过\n";
            continue;
        }

        // Step 3. 临时内参与重投影
        Eigen::Matrix3d K_i = initIntrinsic(mi, dx, dy, u0_, v0_);
        Pose tmpPose = extractPoseFromHomography(Hi, K_i, worldPts, imgPts, mi);
        double e_i = computeReprojectionError(worldPts, imgPts, tmpPose, K_i);

        homographies.push_back(Hi);
        m_values.push_back(mi);
        reprojErrors.push_back(e_i);

        std::cout << "图像 " << i << " : m_i = " << mi << " , e_i = " << e_i << " px\n";
    }

    if (homographies.empty()) {
        std::cerr << "无有效单应矩阵。\n";
        return false;
    }

    // Step 4. 估计内参
    if (!estimateIntrinsicsFromHomographies(homographies, dx, dy, u0_, v0_, reprojErrors)) {
        std::cerr << "内参估计失败。\n";
        return false;
    }

    // Step 5. 计算最终外参与RMSE
    poses_out.clear();
    double totalErr = 0.0;
    int count = 0;
    static int fileCounter = 0;
    std::string txtPath;
    for (size_t i = 0; i < homographies.size(); ++i) {
        txtPath = "./data/CISCamera_Image/txt1/" + ("img_" + std::to_string(++fileCounter) + "_reproj_pts.txt");
        Pose pose = extractPoseFromHomography(homographies[i], K_, worldPts, all_imgPts[i], m_);
        pose.reprojErr = computeReprojectionError(worldPts, all_imgPts[i], pose, K_, txtPath);
        poses_out.push_back(pose);
        totalErr += pose.reprojErr;
        count++;
    }
    std::cout << poses_out[3].R << std::endl;
    std::cout << poses_out[3].t << std::endl;
    rmse_out = totalErr / std::max(1, count);
    double rmse_before = rmse_out;
    K_out = K_;

    std::cout << "\n=== 点集标定完成 ===" << std::endl;
    std::cout << "K = \n" << K_ << "\n平均重投影误差 = " << rmse_out << " 像素" << std::endl;
    // -------------------------- 非线性优化前保存【初步估计内参+外参】 --------------------------
    CalibrationData calib;
    calib.m = m_;
    calib.dx = dx_;
    calib.dy = dy_;
    calib.u0 = u0_;
    calib.v0 = v0_;
    calib.K = K_;
    calib.coff_dis = coff_dis_;  // Eigen::Matrix<double, 1, 5>
    // 转换姿态
    for (const auto& pose : poses_out) {
        Eigen::Vector3d rvec = rotMatToVec(pose.R);
        calib.v_rot.push_back(rvec);
        calib.v_trans.push_back(pose.t);
    }
    if (!calib.save(calib_data_path_)) {
        PLOGE << "警告：保存初步估计参数失";
    } else {
        std::cout << "非线性优化前的初步估计参数已保存：" << calib_data_path_ << std::endl;
    }
    Pose pos1e = estimateTelecentricPose(K_, coff_dis_, worldPts, all_imgPts[3]);
    estimatePosePnP(K_, coff_dis_, worldPts, all_imgPts[3]);
    // TelecentricPYOptimizer opt;
    // PLOGD << " 开始非线性优化";
    // if (!opt.invokeTelecentricCalibration()) {
    //     PLOGE << "Python 调用失败！";
    // }
    return true;
}
Eigen::Vector3d TelecentricLineCalibrator::rotMatToVec(const Eigen::Matrix3d& R) const {
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

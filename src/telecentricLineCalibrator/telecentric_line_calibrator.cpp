#include "telecentric_line_calibrator.h"
#include <QDebug>

#include <iomanip>

// =========================================================
// 构造函数
// =========================================================
TelecentricLineCalibrator::TelecentricLineCalibrator() {
    K_.setIdentity();
    coff_dis_ = Eigen::VectorXd::Zero(5);
}
bool TelecentricLineCalibrator::calculate_Image_Points(cv::Mat imageInput, cv::Size boardSize, std::vector<cv::Point2d>& imagePoints) {
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
    bool found = cv::findCirclesGrid(gray, boardSize, imagePoints, cv::CALIB_CB_SYMMETRIC_GRID | cv::CALIB_CB_CLUSTERING, blobDetector);

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
                                                          const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imagePts,
                                                          const double m, const double u0, const double v0, const double dx, const double dy) {
    Pose pose;

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

    // 固定 Rodrigues 方向：确保 x 分量为正（复现旧结果）
    {
        Eigen::Vector3d rv = rotMatToVec(pose.R);
        if (rv.x() < 0) rv = -rv;
        pose.R = vecToRotMat(rv);
    }

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
bool TelecentricLineCalibrator::estimateIntrinsicsFromHomographies(const std::vector<Eigen::Matrix3d>& Hs, double dx, double dy, double u0, double v0,
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
double TelecentricLineCalibrator::computeReprojectionError(const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imagePts,
                                                           const Pose& pose, const Eigen::Matrix3d& K, const std::string& savePath,
                                                           const Eigen::Matrix<double, 1, 5>& coff_dis) {
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
        // ---------- 仿射成像 + 畸变 ----------
        Eigen::Vector3d xy1;
        xy1 << worldPts[i].x(), worldPts[i].y(), 1.0;

        // 1. 仿射到相机归一化坐标（平面）
        Eigen::Vector2d cam_xy = R2 * xy1.head<2>() + t2;
        // 2. 转为 Nx2 矩阵，方便调用 distort
        Eigen::MatrixXd camPt(1, 2);
        camPt.row(0) = cam_xy.transpose();

        // 3. 畸变，返回 Nx3（齐次坐标）
        Eigen::MatrixXd distortedH = distort(coff_dis, camPt);

        // 4. 乘相机内参
        Eigen::Vector3d uvw = K * distortedH.row(0).transpose();

        // 5. 像素坐标
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
double TelecentricLineCalibrator::computeReprojectionErrorDemo(const std::vector<Eigen::Vector2d>& worldPts,
                                                               const std::vector<Eigen::Vector2d>& imagePts, const Pose& pose,
                                                               const Eigen::Matrix3d& K, const std::string& savePath,
                                                               const Eigen::Matrix<double, 1, 5>& coff_dis) {
    // ======================= 打印输入 =======================
    std::cout << "\n========== [computeReprojectionError Inputs] ==========\n";

    std::cout << "\nPose.R:\n" << pose.R << "\n";
    std::cout << "\nPose.t: " << pose.t.transpose() << "\n";

    std::cout << "\nCamera Matrix K:\n" << K << "\n";

    std::cout << "\nDistortion coff_dis:\n" << coff_dis << "\n";

    std::cout << "=======================================================\n\n";

    const int n = static_cast<int>(worldPts.size());
    if (n == 0) return 0.0;
    double totalErr = 0.0;

    // --- 提取 R2 和 t2（平面部分） ---
    Eigen::Matrix2d R2 = pose.R.block<2, 2>(0, 0);
    Eigen::Vector2d t2 = pose.t.head<2>();

    std::cout << "\n--- C++ Pose ---\n";
    std::cout << "R (3x3):\n" << pose.R << "\n";
    std::cout << "R2 (2x2):\n" << R2 << "\n";
    std::cout << "t2 (2D): " << t2.transpose() << "\n";
    std::cout << "==============================\n";

    for (int i = 0; i < n; ++i) {
        std::cout << "\n------ [C++ Point " << i << "] ------\n";

        // world point
        Eigen::Vector2d xy(worldPts[i].x(), worldPts[i].y());
        std::cout << "world xy = " << xy.transpose() << "\n";

        // ----------- Step 1: 仿射到相机归一化平面 -----------
        Eigen::Vector2d cam_xy = R2 * xy + t2;
        std::cout << "cam_xy = " << cam_xy.transpose() << "\n";

        // 模拟 Python reshape Nx2
        Eigen::MatrixXd camPt(1, 2);
        camPt.row(0) = cam_xy.transpose();

        // ----------- Step 2: 畸变 -----------
        Eigen::MatrixXd distortedH = distort(coff_dis, camPt);
        std::cout << "distortedH = " << distortedH << "\n";

        // ----------- Step 3: 相机内参 K * distortedH -----------
        Eigen::Vector3d uvw = K * distortedH.row(0).transpose();
        std::cout << "uvw = " << uvw.transpose() << "\n";

        // ----------- Step 4: 归一化到像素坐标 -----------
        Eigen::Vector2d uv_hat(uvw(0) / uvw(2), uvw(1) / uvw(2));
        Eigen::Vector2d uv(imagePts[i].x(), imagePts[i].y());

        std::cout << "uv_hat = " << uv_hat.transpose() << "\n";
        std::cout << "uv (GT) = " << uv.transpose() << "\n";

        double err = (uv - uv_hat).norm();
        std::cout << "err = " << err << "\n";

        totalErr += err;
    }

    std::cout << "\n================ END C++ computeReprojError ================\n";
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
Eigen::MatrixXd TelecentricLineCalibrator::distort(const Eigen::Matrix<double, 1, 5>& coff_dis, const Eigen::MatrixXd& normalized_proj) {
    double k1 = coff_dis(0), h1 = coff_dis(1), h2 = coff_dis(2), s1 = coff_dis(3), s2 = coff_dis(4);
    Eigen::VectorXd x = normalized_proj.col(0);
    Eigen::VectorXd y = normalized_proj.col(1);
    Eigen::VectorXd r = x.array().square() + y.array().square();

    Eigen::VectorXd deltaX =
        k1 * x.array() * r.array() + h1 * (3 * x.array().square() + y.array().square()) + 2 * h2 * x.array() * y.array() + s1 * r.array();
    Eigen::VectorXd deltaY =
        k1 * y.array() * r.array() + 2 * h1 * x.array() * y.array() + h2 * (x.array().square() + 3 * y.array().square()) + s2 * r.array();

    Eigen::MatrixXd distorted(normalized_proj.rows(), 2);
    distorted.col(0) = x + deltaX;
    distorted.col(1) = y + deltaY;

    return toHomogeneous(distorted);
}

// -------------------- 迭代去畸变 --------------------
Eigen::MatrixXd TelecentricLineCalibrator::undistortPointsIter(const Eigen::MatrixXd& points_px, const Eigen::Matrix3d& K,
                                                               const Eigen::Matrix<double, 1, 5>& coff_dis, int max_iter, double tol) {
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
            // 计算畸变量
            double r = x * x + y * y;
            double deltaX = k1 * x * r + h1 * (3 * x * x + y * y) + 2 * h2 * x * y + s1 * r;
            double deltaY = k1 * y * r + 2 * h1 * x * y + h2 * (x * x + 3 * y * y) + s2 * r;

            x = x0 - deltaX;
            y = y0 - deltaY;

            // 算畸变后的坐标
            r = x * x + y * y;
            double xd = x + k1 * x * r + h1 * (3 * x * x + y * y) + 2 * h2 * x * y + s1 * r;
            double yd = y + k1 * y * r + 2 * h1 * x * y + h2 * (x * x + 3 * y * y) + s2 * r;

            // 算出来的畸变坐标和原始u，v的欧式距离，迭代的目的就是让欧式距离最小，得到精确的畸变系数
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
    qDebug() << "内参矩阵 K =" << K(0,0) << K(0,1) << K(0,2) << "\n"
             << K(1,0) << K(1,1) << K(1,2) << "\n"
             << K(2,0) << K(2,1) << K(2,2);
    qDebug() << "畸变系数 =" << coff_dis(0) << coff_dis(1) << coff_dis(2) << coff_dis(3) << coff_dis(4);

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

// -------------------- 相机坐标 → 世界坐标 --------------------
Eigen::MatrixXd TelecentricLineCalibrator::cameraToWorldCoordinates(const Eigen::MatrixXd& cam_pts, const Eigen::Vector3d& v_rot,
                                                                    const Eigen::Vector3d& v_trans) {
    // -------- 1. Rodrigues旋转向量转旋转矩阵 --------
    qDebug() << "世界旋转向量 =" << v_rot(0) << v_rot(1) << v_rot(2)
             << "世界平移向量 =" << v_trans(0) << v_trans(1) << v_trans(2);
    cv::Mat rvec(3, 1, CV_64F);
    cv::Mat R_cv(3, 3, CV_64F);
    for (int i = 0; i < 3; ++i) rvec.at<double>(i, 0) = v_rot(i);
    cv::Rodrigues(rvec, R_cv);

    // std::cout << "=====================v_rot" << v_rot << std::endl;

    Eigen::Matrix3d R;
    cv::cv2eigen(R_cv, R);
    // -------- 2. 取平面部分 --------
    Eigen::Matrix2d R2 = R.block<2, 2>(0, 0);
    Eigen::Vector2d t2 = v_trans.head<2>();
    // std::cout << "=====================" << R << std::endl;
    // std::cout << "=====================" << v_trans << std::endl;

    // -------- 3. 平面逆变换（相机 -> 世界）--------
    Eigen::MatrixXd world_pts(cam_pts.rows(), 2);
    Eigen::Matrix2d R2_inv = R2.inverse();
    for (int i = 0; i < cam_pts.rows(); ++i) {
        Eigen::Vector2d Xc = cam_pts.row(i);
        Eigen::Vector2d Xw = R2_inv * (Xc - t2);
        world_pts.row(i) = Xw.transpose();
    }
    std::cout << std::fixed << std::setprecision(15);

    // std::cout << "v_rot =\n" << v_rot << std::endl;
    // -------- 打印 R2 --------
    // std::cout << "R2 =\n" << R2 << std::endl;

    // -------- 打印 R2_inv --------
    // std::cout << "R2_inv =\n" << R2_inv << std::endl;

    // -------- 打印 t2 --------
    // std::cout << "t2 = " << t2.transpose() << std::endl;

    // -------- 打印 Xc / Xw（只打印第一个点）--------
    Eigen::Vector2d Xc = cam_pts.row(0);
    Eigen::Vector2d Xw = R2_inv * (Xc - t2);

    // std::cout << "Xc = " << Xc.transpose() << std::endl;
    // std::cout << "Xw = " << Xw.transpose() << std::endl;
    return world_pts;
}
// -------------------- 相机坐标 → 世界坐标（SO(2) 修正） --------------------
Eigen::MatrixXd TelecentricLineCalibrator::cameraToWorldCoordinatesSO2(const Eigen::MatrixXd& cam_pts, const Eigen::Vector3d& v_rot,
                                                                       const Eigen::Vector3d& v_trans) {
    // -------- 1. Rodrigues：world -> camera --------
    cv::Mat rvec(3, 1, CV_64F);
    for (int i = 0; i < 3; ++i) rvec.at<double>(i, 0) = v_rot(i);

    cv::Mat R_cv;
    cv::Rodrigues(rvec, R_cv);

    Eigen::Matrix3d R;
    cv::cv2eigen(R_cv, R);

    // -------- 2. 取 2D 平面部分 --------
    Eigen::Matrix2d R2 = R.block<2, 2>(0, 0);
    Eigen::Vector2d t2 = v_trans.head<2>();

    // -------- 3. 对 R2 做极分解：R2 = U * S --------
    Eigen::JacobiSVD<Eigen::Matrix2d> svd(R2, Eigen::ComputeFullU | Eigen::ComputeFullV);

    Eigen::Matrix2d U = svd.matrixU() * svd.matrixV().transpose();  // 最近 SO(2)

    // 防止反射（det = -1）
    if (U.determinant() < 0) {
        Eigen::Matrix2d V = svd.matrixV();
        V.col(1) *= -1;
        U = svd.matrixU() * V.transpose();
    }

    // -------- 4. 用“纯旋转”做相机 -> 世界 --------
    Eigen::MatrixXd world_pts(cam_pts.rows(), 2);
    Eigen::Matrix2d U_inv = U.transpose();  // SO(2): inverse = transpose

    for (int i = 0; i < cam_pts.rows(); ++i) {
        Eigen::Vector2d Xc = cam_pts.row(i);
        Eigen::Vector2d Xw = U_inv * (Xc - t2);
        world_pts.row(i) = Xw.transpose();
    }

    return world_pts;
}
bool TelecentricLineCalibrator::optimizeExtrinsicsWithLeastSquares(const Eigen::MatrixXd& transformedPts,
                                                                   const std::vector<Eigen::Vector2d>& trueWorldPts, const Eigen::Matrix3d& R_in,
                                                                   const Eigen::Vector3d& t_in, Eigen::Matrix3d& R_out, Eigen::Vector3d& t_out) {
    if (transformedPts.rows() != trueWorldPts.size() || transformedPts.rows() < 3) {
        std::cerr << "[optimizeExtrinsicsWithLeastSquares] 点数量不足或不匹配" << std::endl;
        return false;
    }

    std::cout << "[最小二乘优化] 开始构建仿射变换模型..." << std::endl;

    // 1. 准备数据点对
    struct PointPair {
        double transformed_x, transformed_y;
        double true_x, true_y;
    };
    std::vector<PointPair> data;
    data.reserve(trueWorldPts.size());

    for (int i = 0; i < trueWorldPts.size(); ++i) {
        PointPair pair;
        pair.transformed_x = transformedPts(i, 0);
        pair.transformed_y = transformedPts(i, 1);
        pair.true_x = trueWorldPts[i].x();
        pair.true_y = trueWorldPts[i].y();
        data.push_back(pair);
    }

    // 2. 构建最小二乘方程：world_true = A * world_transformed + b
    int n = data.size();
    Eigen::MatrixXd X(2 * n, 6);  // 扩展矩阵用于求解仿射变换
    Eigen::VectorXd Y(2 * n);     // 目标世界坐标

    for (int i = 0; i < n; ++i) {
        // X分量
        X(2 * i, 0) = data[i].transformed_x;
        X(2 * i, 1) = data[i].transformed_y;
        X(2 * i, 2) = 1;
        X(2 * i, 3) = 0;
        X(2 * i, 4) = 0;
        X(2 * i, 5) = 0;

        // Y分量
        X(2 * i + 1, 0) = 0;
        X(2 * i + 1, 1) = 0;
        X(2 * i + 1, 2) = 0;
        X(2 * i + 1, 3) = data[i].transformed_x;
        X(2 * i + 1, 4) = data[i].transformed_y;
        X(2 * i + 1, 5) = 1;

        // 目标值
        Y(2 * i) = data[i].true_x;
        Y(2 * i + 1) = data[i].true_y;
    }

    // 3. 使用SVD求解最小二乘问题
    Eigen::VectorXd params = X.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(Y);

    // 4. 提取补偿矩阵和偏移量
    Eigen::Matrix2d A;
    Eigen::Vector2d b;
    A << params[0], params[1], params[3], params[4];
    b << params[2], params[5];

    // 5. 输出最小二乘优化结果
    std::cout << "[最小二乘优化] 补偿矩阵求解结果：" << std::endl;
    std::cout << "  转换矩阵 A (2x2):" << std::endl;
    std::cout << "  " << A << std::endl;
    std::cout << "  偏移向量 b (2x1):" << std::endl;
    std::cout << "  " << b << std::endl;

    // 6. 更新外参：应用补偿变换
    // a. 计算新的旋转矩阵：R_new = A * R_old
    Eigen::Matrix2d R2_old = R_in.block<2, 2>(0, 0);
    Eigen::Matrix2d R2_new = A * R2_old;

    // b. 计算新的平移向量：t_new = A * t_old + b
    Eigen::Vector2d t2_old = t_in.head<2>();
    Eigen::Vector2d t2_new = A * t2_old + b;

    // c. 更新完整的旋转矩阵和平移向量
    R_out = R_in;
    R_out.block<2, 2>(0, 0) = R2_new;

    t_out = t_in;
    t_out.head<2>() = t2_new;
    // 5.1 计算补偿前（原始）误差
    double raw_error = 0;
    for (const auto& p : data) {
        Eigen::Vector2d raw_pred(p.transformed_x, p.transformed_y);
        Eigen::Vector2d world_true(p.true_x, p.true_y);
        double err = (raw_pred - world_true).norm();
        raw_error += err * err;
    }

    // 6. 计算补偿后误差
    double corrected_error = 0;
    std::cout << "\n=== 补偿后拟合误差分析 ===" << std::endl;
    for (const auto& p : data) {
        Eigen::Vector2d pixel(p.transformed_x, p.transformed_y);
        Eigen::Vector2d world_pred = A * pixel + b;
        Eigen::Vector2d world_true(p.true_x, p.true_y);
        double err = (world_pred - world_true).norm();
        corrected_error += err * err;

        if (&p == &data[0] || &p == &data[8] || &p == &data[16]) {
            std::cout << "像素(" << p.transformed_x << "," << p.transformed_y << ") -> 预测(" << world_pred[0] << "," << world_pred[1] << ") 真实("
                      << p.true_x << "," << p.true_y << ") 误差: " << err << std::endl;
        }
    }
    // 7. 输出补偿前/补偿后的对比结果
    std::cout << "\n=== 原始误差 vs 补偿后误差 ===" << std::endl;
    std::cout << "原始均方误差 (RMSE): " << sqrt(raw_error / n) << std::endl;
    std::cout << "补偿后均方误差 (RMSE): " << sqrt(corrected_error / n) << std::endl;

    // 7. 输出所有补偿后坐标及误差（加入补偿前误差）
    std::cout << "\n=== 所有坐标补偿结果及误差 ===" << std::endl;
    std::cout << std::left << std::setw(15) << "像素X" << std::setw(15) << "像素Y" << std::setw(18) << "补偿后世界X" << std::setw(18) << "补偿后世界Y"
              << std::setw(15) << "真实世界X" << std::setw(15) << "真实世界Y" << std::setw(15) << "点误差" << std::setw(15)
              << "原始误差"  //  新增这一列
              << std::endl;

    std::cout << std::string(130, '-') << std::endl;

    for (const auto& p : data) {
        Eigen::Vector2d pixel(p.transformed_x, p.transformed_y);
        Eigen::Vector2d world_pred = A * pixel + b;      // 补偿后世界坐标
        Eigen::Vector2d world_true(p.true_x, p.true_y);  // 真实世界坐标

        double err_after = (world_pred - world_true).norm();  // 补偿后误差
        double err_before = (pixel - world_true).norm();      // 补偿前误差

        std::cout << std::left << std::setw(15) << p.transformed_x << std::setw(15) << p.transformed_y << std::setw(18) << world_pred[0]
                  << std::setw(18) << world_pred[1] << std::setw(15) << p.true_x << std::setw(15) << p.true_y << std::setw(15) << err_after
                  << std::setw(15) << err_before  // 输出原始误差
                  << std::endl;
    }

    // 输出统计
    std::cout << "\n=== 整体误差统计 ===" << std::endl;
    std::cout << "原始均方误差 (RMSE):  " << sqrt(raw_error / n) << std::endl;
    std::cout << "补偿后均方误差 (RMSE): " << sqrt(corrected_error / n) << std::endl;
    std::cout << "误差下降比例: " << (1.0 - sqrt(corrected_error / raw_error)) * 100.0 << "%" << std::endl;

    std::cout << "[最小二乘优化] 外参优化完成" << std::endl;
    return true;
}
Pose TelecentricLineCalibrator::estimateTelecentricPose(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& coff_dis, const double m,
                                                        const double u0, const double v0, const double dx, const double dy,
                                                        const std::vector<Eigen::Vector2d>& worldPts, const std::vector<Eigen::Vector2d>& imgPts) {
    std::cout << "======== [estimateTelecentricPose] 开始求解外参 ========" << std::endl;

    if (worldPts.size() != imgPts.size() || worldPts.size() < 4) throw std::runtime_error("estimateTelecentricPose: 输入点数量不足或不匹配");

    std::cout << "Step 1. 输入点数量: " << worldPts.size() << " 对" << std::endl;
    std::cout << "Step 1. 相机内参:\n" << K << std::endl;
    std::cout << "Step 1. 畸变系数: " << coff_dis << std::endl;

    // ---------- Step 1. 像素坐标去畸变 ----------
    std::cout << "[1] 正在执行去畸变..." << std::endl;
    Eigen::MatrixXd imgPtsMat(imgPts.size(), 2);
    for (size_t i = 0; i < imgPts.size(); ++i) imgPtsMat.row(i) = imgPts[i].transpose();

    Eigen::MatrixXd undistorted = undistortPointsIter(imgPtsMat, K, coff_dis, 2000, 1e-12);
    std::cout << "[1] 去畸变完成，前3个结果：" << std::endl;
    for (int i = 0; i < std::min<int>(3, undistorted.rows()); ++i)
        std::cout << "  原: " << imgPtsMat.row(i) << " → 去畸变: " << undistorted.row(i) << std::endl;

    std::vector<Eigen::Vector2d> undistortedPts(undistorted.rows());
    for (int i = 0; i < undistorted.rows(); ++i) undistortedPts[i] = undistorted.row(i).transpose();

    // ---------- Step 2. 求单应矩阵 ----------
    std::cout << "[2] 正在计算单应矩阵..." << std::endl;
    Eigen::Matrix3d Hi = computeHomography(worldPts, undistortedPts);
    std::cout << "[2] 单应矩阵 H:\n" << Hi << std::endl;

    // ---------- Step 3. 提取外参 ----------
    std::cout << "[3] 从单应矩阵提取外参..." << std::endl;
    Pose pose = extractPoseFromHomography(Hi, K, worldPts, undistortedPts, m, u0, v0, dx, dy);
    std::cout << "[3] 旋转矩阵 R:\n" << pose.R << std::endl;
    std::cout << "[3] 平移向量 t: " << pose.t.transpose() << std::endl;
    Eigen::Vector3d rvec = rotMatToVec(pose.R);
    std::cout << "[3] 优化前Rodrigues 旋转向量 (rx, ry, rz): " << rvec.transpose() << std::endl;
    // ---------- Step 4. 计算重投影误差 ----------
    std::cout << "[4] 正在计算重投影误差..." << std::endl;
    pose.reprojErr = computeReprojectionError(worldPts, imgPts, pose, K, " ", coff_dis);
    std::cout << "[4] 平均重投影误差 = " << pose.reprojErr << std::endl;
    // ---------- Step 5. 调用非线性优化 ----------
    std::cout << "[5] 正在调用 Python 非线性外参优化..." << std::endl;

    Eigen::Vector3d opt_r = rotMatToVec(pose.R);
    Eigen::Vector3d opt_r_init = opt_r;
    Eigen::Vector3d opt_t = pose.t;
    double opt_err = 0.0;
    TelecentricPYOptimizer opt;
    bool ok = opt.optTelecentricExtrinsicParameters(K, coff_dis, imgPts, worldPts, opt_r, opt_t, opt_err);

    if (!ok) {
        std::cout << " [5] 优化失败，返回初始值。" << std::endl;
    } else {
        // 防止符号翻转（rvec → -rvec 数学等价但数值不同）
        if (opt_r.dot(opt_r_init) < 0) opt_r = -opt_r;
        std::cout << " [5] 优化成功！" << std::endl;
        std::cout << "   优化后 rvec = " << opt_r.transpose() << std::endl;
        std::cout << "   优化后 tvec = " << opt_t.transpose() << std::endl;
        std::cout << "   优化残差 = " << opt_err << std::endl;

        // 写回 pose
        pose.R = vecToRotMat(opt_r);
        pose.t = opt_t;
        pose.reprojErr = opt_err;
        std::string txtPath = "./data/CISCamera_Image/txt1/" + ("img_" + std::to_string(100) + "_reproj_pts.txt");
        std::cout << "\n====== computeReprojectionError INPUT ======\n";

        // // --- 打印 worldPts ---
        // std::cout << "worldPts (" << worldPts.size() << "):\n";
        // for (size_t i = 0; i < worldPts.size(); ++i) {
        //     std::cout << "  [" << i << "] " << worldPts[i].transpose() << "\n";
        // }

        // // --- 打印 imagePts ---
        // std::cout << "imagePts (" << imgPts.size() << "):\n";
        // for (size_t i = 0; i < imgPts.size(); ++i) {
        //     std::cout << "  [" << i << "] " << imgPts[i].transpose() << "\n";
        // }
        // double test = computeReprojectionError(worldPts, imgPts, pose, K, txtPath, coff_dis);
        // std::cout << "test err" << test << std::endl;

        // // ---------- Step 6. 调用最小二乘法进一步优化外参 ----------
        // std::cout << "[6] 正在使用最小二乘法进一步优化外参..." << std::endl;

        // // 1. 将像素坐标转换到世界坐标（使用优化后的外参）
        // Eigen::MatrixXd px = imgPtsMat;

        // // 像素坐标 → 相机归一化坐标（去畸变 + 归一化）
        // Eigen::MatrixXd cam_norm = pixelToCameraCoordinates(px, K, coff_dis);

        // // 相机坐标 → 世界坐标（使用当前优化后的外参）
        // Eigen::MatrixXd world_transformed = cameraToWorldCoordinates(cam_norm, opt_r, opt_t);

        // // 2. 调用最小二乘优化函数
        // Eigen::Matrix3d R_refined;
        // Eigen::Vector3d t_refined;
        // if (optimizeExtrinsicsWithLeastSquares(world_transformed, worldPts, pose.R, pose.t, R_refined, t_refined)) {
        //     // 3. 验证优化效果，只有当重投影误差减小时才接受新参数
        //     Pose refined_pose;
        //     refined_pose.R = R_refined;
        //     refined_pose.t = t_refined;
        //     refined_pose.reprojErr = computeReprojectionError(worldPts, imgPts, refined_pose, K, " ", coff_dis);
        //     std::cout << "refined_pose.reprojErr " << refined_pose.reprojErr << std::endl;
        //     std::cout << "pose.reprojErr " << pose.reprojErr << std::endl;
        //     if (refined_pose.reprojErr < pose.reprojErr) {
        //         pose = refined_pose;
        //         std::cout << " [6] 最小二乘优化成功，重投影误差从 " << pose.reprojErr << " 减小到 " << refined_pose.reprojErr
        //                   << std::endl;
        //     } else {
        //         std::cout << " [6] 最小二乘优化未能改善结果，保持原参数" << std::endl;
        //     }
        // } else {
        //     std::cout << " [6] 最小二乘优化失败，保持原参数" << std::endl;
        // }
    }

    std::cout << "======== [estimateTelecentricPose] 完成 ========" << std::endl;
    return pose;
}
// =========================================================
// Step 5. 主函数：标定流程（DLT 初值）
// =========================================================
bool TelecentricLineCalibrator::calibrateCameraFromPointsDemo(const std::vector<std::vector<Eigen::Vector2d>>& all_imgPts,
                                                              const std::vector<Eigen::Vector2d>& worldPts, int width, int height, double dx,
                                                              double dy, Eigen::Matrix3d K_out, double rmse_out, std::vector<Pose> poses_out) {
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
        Pose tmpPose = extractPoseFromHomography(Hi, K_i, worldPts, imgPts, mi, u0_, v0_, dx_, dy_);
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
        Pose pose = extractPoseFromHomography(homographies[i], K_, worldPts, all_imgPts[i], m_, u0_, v0_, dx_, dy_);
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
        PLOGD << "非线性优化前的初步估计参数已保存：" << calib_data_path_;
    }
    // Pose pos2e = estimateTelecentricPose(K_, coff_dis_, m_,u0_, v0_, dx_, dy_,worldPts, all_imgPts[3]);

    CalibrationData calib2;
    if (!calib2.load("./data/calibration_config/optimized_calib_data.json")) {
        throw std::runtime_error("无法加载标定文件");
    }
    // ------------读取初步标定数据沿着非线性优化求取外参是否合理----------
    // // 从标定数据中读取参数
    // Eigen::Matrix3d K1 = calib2.K;
    // Eigen::Matrix<double, 1, 5> coff_dis1 = calib2.coff_dis;
    // double m1 = calib2.m;
    // double u01 = calib2.u0;
    // double v01 = calib2.v0;
    // double dx1 = calib2.dx;
    // double dy1 = calib2.dy;

    // std::vector<Eigen::Vector3d> v_rot1 = calib2.v_rot;
    // std::vector<Eigen::Vector3d> v_trans1 = calib2.v_trans;

    // // Pose aaa;
    // // cv::Vec3d rvec_new(2.0747767, 2.05844074, -0.21906585);
    // // cv::Vec3d tvec_new(-249.01625128, -134.99191718, 1);
    // // cv::Mat R_cv;
    // // cv::Rodrigues(rvec_new, R_cv);
    // // aaa.R = Eigen::Map<Eigen::Matrix<double, 3, 3, Eigen::RowMajor>>(R_cv.ptr<double>());
    // // aaa.t = Eigen::Vector3d(tvec_new[0], tvec_new[1], tvec_new[2]);
    // // aaa.reprojErr = computeReprojectionErrorDemo(worldPts, all_imgPts[3], aaa, K1, " ", coff_dis1);
    // // std::cout << "平均重投影误差 = " << aaa.reprojErr << std::endl;
    // // 调用姿态估计函数
    // std::vector<Eigen::Vector2d> pts;
    // if (readPointsFromTxt("D:/Code/CISCamera_DALSA/data/PaltfromCalibrate/orignCor/txt/Splice_20251108_160806374.txt", pts)) {
    //     Pose pos1e = estimateTelecentricPose(K1, coff_dis1, m1, u0_, v0_, dx_, dy_, worldPts, pts);
    // }
    //------------------------------------------------------

    // PLOGD << " 开始非线性优化";
    TelecentricPYOptimizer opt;
    opt.setLogCallback(
        [this](const std::string& line) { QMetaObject::invokeMethod(this, [this, msg = QString::fromStdString(line)]() {}, Qt::QueuedConnection); });

    if (!opt.invokeTelecentricCalibration()) {
        PLOGE << "Python 调用失败！";
    }
    emit sendSignalSuccessCalib();
    return true;
}

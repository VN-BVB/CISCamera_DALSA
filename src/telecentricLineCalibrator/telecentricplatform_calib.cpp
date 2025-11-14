#include "telecentricplatform_calib.h"

TelecentricPlatformCalib::TelecentricPlatformCalib(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& dist,
                                                   const Eigen::Vector3d& rvec, const Eigen::Vector3d& tvec)
    : K_(K), dist_(dist), v_rot_(rvec), v_trans_(tvec) {
    lineCalib_ = new TelecentricLineCalibrator();
}
bool TelecentricPlatformCalib::readPointsFromTxt(const std::string& path, std::vector<Eigen::Vector2d>& pts) {
    std::ifstream fin(path);
    if (!fin.is_open()) {
        std::cerr << "无法打开文件: " << path << std::endl;
        return false;
    }

    std::string line;
    std::getline(fin, line);  // 跳过标题

    pts.clear();
    double idx, x, y;

    while (fin >> idx >> x >> y) {
        pts.emplace_back(x, y);
    }

    if (pts.empty()) {
        std::cerr << "文件 " << path << " 无有效点。\n";
        return false;
    }

    std::cout << "读取 " << path << " 成功，共 " << pts.size() << " 点\n";
    return true;
}

Eigen::MatrixXd TelecentricPlatformCalib::vecToMat(const std::vector<Eigen::Vector2d>& v) {
    Eigen::MatrixXd M(v.size(), 2);
    for (int i = 0; i < v.size(); i++) M.row(i) = v[i];
    return M;
}

std::vector<Eigen::Vector2d> TelecentricPlatformCalib::matToVec(const Eigen::MatrixXd& M) {
    std::vector<Eigen::Vector2d> v;
    v.reserve(M.rows());
    for (int i = 0; i < M.rows(); i++) v.emplace_back(M(i, 0), M(i, 1));
    return v;
}

std::vector<Eigen::Vector2d> TelecentricPlatformCalib::convertToWorld(const std::vector<Eigen::Vector2d>& pix_pts) {
    if (pix_pts.empty()) return {};

    Eigen::MatrixXd px = vecToMat(pix_pts);

    // pixel → camera (去畸变 + 归一化)
    Eigen::MatrixXd cam_norm = lineCalib_->pixelToCameraCoordinates(px, K_, dist_);

    // camera → world（逆平面变换）
    Eigen::MatrixXd world = lineCalib_->cameraToWorldCoordinates(cam_norm, v_rot_, v_trans_);

    return matToVec(world);
}

Eigen::Vector3d TelecentricPlatformCalib::computeDirectionLS(const std::vector<Eigen::Vector2d>& ptsA,
                                                             const std::vector<Eigen::Vector2d>& ptsB) {
    if (ptsA.size() != ptsB.size() || ptsA.empty()) return Eigen::Vector3d::Zero();

    Eigen::MatrixXd D(ptsA.size(), 2);
    for (int i = 0; i < ptsA.size(); i++) {
        D(i, 0) = ptsB[i].x() - ptsA[i].x();
        D(i, 1) = ptsB[i].y() - ptsA[i].y();
    }

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(D, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::Vector2d d2 = svd.matrixV().col(0);

    return Eigen::Vector3d(d2.x(), d2.y(), 0).normalized();
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

        Eigen::Matrix2d I = Eigen::Matrix2d::Identity();
        Eigen::Vector2d C = (I - R).inverse() * (meanQ - R * meanP);

        sumC += C;
        count++;

        double ang = std::atan2(R(1, 0), R(0, 0)) * 180.0 / M_PI;
        std::cout << "第 " << k << " 组旋转中心 = " << C.transpose() << " 角度 = " << ang << "°\n";
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
void TelecentricPlatformCalib::run() {
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

    std::cout << "X方向 = " << xdir.transpose() << "\n";
    std::cout << "Y方向 = " << ydir.transpose() << "\n";

    std::vector<std::vector<Eigen::Vector2d>> ptsRot{w3, w4, w5};
    Eigen::Vector2d C = computeRotationCenterSequential(ptsRot);

    std::cout << "旋转中心 = " << C.transpose() << "\n";
}

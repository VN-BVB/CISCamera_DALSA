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
TelecentricPlatformCalib::TelecentricPlatformCalib(const Eigen::Matrix3d& K, const Eigen::Matrix<double, 1, 5>& dist,
                                                   const Eigen::Vector3d& rvec, const Eigen::Vector3d& tvec)
    : K_(K), dist_(dist), v_rot_(rvec), v_trans_(tvec) {
    lineCalib_ = new TelecentricLineCalibrator();
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
void TelecentricPlatformCalib::runDemo(std::vector<std::vector<Eigen::Vector2d>> pts) {
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
    // -------------------- 5. 构建平台坐标系在临时世界下的旋转矩阵 --------------------
    Eigen::Vector3d zdir = xdir.cross(ydir).normalized();
    Eigen::Matrix3d R_plat_world;
    R_plat_world.col(0) = xdir.normalized();
    R_plat_world.col(1) = ydir.normalized();
    R_plat_world.col(2) = zdir;
    Eigen::Vector3d t_plat_world(C.x(), C.y(), 0.0);

    // -------------------- 6. 临时世界 -> 相机坐标系 --------------------
    Eigen::Matrix3d R_world_cam;
    cv::Mat rvec_cv(3, 1, CV_64F), R_cv(3, 3, CV_64F);
    for (int i = 0; i < 3; ++i) rvec_cv.at<double>(i, 0) = v_rot_(i);
    cv::Rodrigues(rvec_cv, R_cv);
    cv::cv2eigen(R_cv, R_world_cam);
    Eigen::Vector3d t_world_cam = v_trans_;

    // -------------------- 7. 平台坐标系 -> 相机坐标系 --------------------
    Eigen::Matrix3d R_plat_cam = R_world_cam * R_plat_world;
    Eigen::Vector3d t_plat_cam = R_world_cam * t_plat_world + t_world_cam;
    std::cout << "平台 -> 相机旋转矩阵 = \n" << R_plat_cam << "\n";
    std::cout << "平台 -> 相机平移向量 = " << t_plat_cam.transpose() << "\n";

    // -------------------- 8. 平台到相机旋转向量 --------------------
    cv::Mat R_plat_cv, rvec_plat_cv;
    cv::eigen2cv(R_plat_cam, R_plat_cv);
    cv::Rodrigues(R_plat_cv, rvec_plat_cv);
    Eigen::Vector3d rvec_plat;
    for (int i = 0; i < 3; ++i) rvec_plat(i) = rvec_plat_cv.at<double>(i, 0);

    // -------------------- 9. 给定像素点转换到平台坐标系 --------------------
    Eigen::MatrixXd px(1, 2);
    px(0, 0) = 6937.323892;
    px(0, 1) = 2816.240515;

    // 像素 -> 相机平面坐标
    Eigen::MatrixXd cam_norm = lineCalib_->pixelToCameraCoordinates(px, K_, dist_);

    // 直接用平台到相机的外参进行相机 -> 平台坐标系转换
    Eigen::MatrixXd plat_pts = lineCalib_->cameraToWorldCoordinates(cam_norm, rvec_plat, t_plat_cam);

    // 输出结果
    std::cout << "像素点在平台坐标系 = " << plat_pts(0, 0) << ", " << plat_pts(0, 1) << "\n";
}
bool TelecentricPlatformCalib::estimatePlatformPoseFromBoards(
    const std::vector<std::vector<std::vector<cv::Point2d>>>& onePlatformBoards, Eigen::Vector3d& vRotPlat,
    Eigen::Vector3d& vTransPlat) {
    if (onePlatformBoards.size() < 5) {
        PLOGE << "输入的图像数量不足 5 组！";
        return false;
    }

    // -------------------- 1. 取每组的第一个标定板 --------------------
    auto extractBoard = [&](int idx) {
        std::vector<Eigen::Vector2d> pts;
        const auto& cvpts = onePlatformBoards[idx][0];

        pts.reserve(cvpts.size());
        for (const auto& p : cvpts) pts.emplace_back(p.x, p.y);
        return pts;
    };

    auto p1 = extractBoard(0);
    auto p2 = extractBoard(1);
    auto p3 = extractBoard(2);
    auto p4 = extractBoard(3);
    auto p5 = extractBoard(4);

    // -------------------- 2. 像素 -> 世界 --------------------
    auto w1 = convertToWorld(p1);
    auto w2 = convertToWorld(p2);
    auto w3 = convertToWorld(p3);
    auto w4 = convertToWorld(p4);
    auto w5 = convertToWorld(p5);

    // -------------------- 3. 平台方向向量 --------------------
    Eigen::Vector3d xdir = computeDirectionLS(w3, w4);
    Eigen::Vector3d ydir = computeDirectionLS(w4, w5);

    // -------------------- 4. 旋转中心 --------------------
    Eigen::Vector2d C = computeRotationCenterSequential({w1, w2, w3});

    // -------------------- 5. 平台 -> 世界
    Eigen::Vector3d zdir = xdir.cross(ydir).normalized();

    Eigen::Matrix3d R_plat_world;
    R_plat_world.col(0) = xdir.normalized();
    R_plat_world.col(1) = ydir.normalized();
    R_plat_world.col(2) = zdir;

    Eigen::Vector3d t_plat_world(C.x(), C.y(), 0);

    // -------------------- 6. 世界 -> 相机
    Eigen::Matrix3d R_world_cam;
    cv::Mat rvec_cv(3, 1, CV_64F), R_cv(3, 3, CV_64F);
    for (int i = 0; i < 3; i++) rvec_cv.at<double>(i, 0) = v_rot_(i);
    cv::Rodrigues(rvec_cv, R_cv);
    cv::cv2eigen(R_cv, R_world_cam);

    Eigen::Vector3d t_world_cam = v_trans_;

    // -------------------- 7. 平台 -> 相机
    Eigen::Matrix3d R_plat_cam = R_world_cam * R_plat_world;
    Eigen::Vector3d t_plat_cam = R_world_cam * t_plat_world + t_world_cam;

    // -------------------- 8. 转换为旋转向量
    cv::Mat R_plat_cv, rvec_plat_cv;
    cv::eigen2cv(R_plat_cam, R_plat_cv);
    cv::Rodrigues(R_plat_cv, rvec_plat_cv);

    for (int i = 0; i < 3; ++i) vRotPlat(i) = rvec_plat_cv.at<double>(i, 0);

    vTransPlat = t_plat_cam;

    return true;
}

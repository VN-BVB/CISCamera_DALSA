// #include <plog/Init.h>
// #include <plog/Initializers/ConsoleInitializer.h>
// #include <plog/Initializers/RollingFileInitializer.h>
// #include <plog/Log.h>

// #include <QApplication>
// // clang-format off
// #include <winsock2.h>
// #include <windows.h>
// // clang-format on
// #include "src/crashHandler/CrashHandler.h"
// #include "src/test/test_tiny_spline.h"
// #include "src/ui/CISCamera_imageGrab/cis_camera_image.h"

// void initPlog();  // 初始化日志类
// int main(int argc, char *argv[]) {
//     QApplication a(argc, argv);
//     CrashHandler::Init(L"data/debug");  // 初始化Mini转储
//     initPlog();                         // 初始化日志类
//     CISWidget w;
//     w.show();

//     return a.exec();
// }
// // 初始化日志类
// void initPlog() {
//     // 日志信息分类等级: none = 0, fatal = 1, error = 2, warning = 3, info = 4, debug = 5, verbose = 6
//     // 设置初始化等级后, 等级『数值大于』设置值的日志信息就会被『忽略』
//     plog::init(plog::debug, "./data/log/log.csv", 1000000000, 100);
//     static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
//     plog::get()->addAppender(&consoleAppender);  // Also add logging to the console.
// }
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <direct.h>

#include <Eigen/Dense>
#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QObject>
#include <QString>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>

#include "src/telecentricLineCalibrator/telecentric_line_calibrator.h"
// 计算X方向向量（单位向量）
Eigen::Vector3d computeXDirection(const std::vector<Eigen::Vector2d>& pts1, const std::vector<Eigen::Vector2d>& pts2) {
    if (pts1.size() != pts2.size() || pts1.empty()) {
        std::cerr << "角点数量不一致或为空！\n";
        return Eigen::Vector3d::Zero();
    }

    Eigen::Vector3d sum(0, 0, 0);
    for (size_t i = 0; i < pts1.size(); ++i) {
        Eigen::Vector3d v(pts2[i].x() - pts1[i].x(), pts2[i].y() - pts1[i].y(), 0.0);
        sum += v;
    }
    Eigen::Vector3d dir = sum.normalized();
    std::cout << "X方向向量: [" << dir.transpose() << "]\n";
    return dir;
}

// 计算Y方向向量（单位向量）
Eigen::Vector3d computeYDirection(const std::vector<Eigen::Vector2d>& pts2, const std::vector<Eigen::Vector2d>& pts3) {
    if (pts2.size() != pts3.size() || pts2.empty()) {
        std::cerr << "角点数量不一致或为空！\n";
        return Eigen::Vector3d::Zero();
    }

    Eigen::Vector3d sum(0, 0, 0);
    for (size_t i = 0; i < pts2.size(); ++i) {
        Eigen::Vector3d v(pts3[i].x() - pts2[i].x(), pts3[i].y() - pts2[i].y(), 0.0);
        sum += v;
    }
    Eigen::Vector3d dir = sum.normalized();
    std::cout << "Y方向向量: [" << dir.transpose() << "]\n";
    return dir;
}

// 计算旋转中心（原点位置）
// 思路：旋转中心 C 满足 P3 - C = R * (P1 - C)，对每个角点最小二乘拟合
Eigen::Vector2d computeRotationCenterSequential(const std::vector<std::vector<Eigen::Vector2d>>& pts) {
    if (pts.size() < 2) {
        std::cerr << "点集数量不足！\n";
        return Eigen::Vector2d::Zero();
    }

    Eigen::Matrix2d I = Eigen::Matrix2d::Identity();
    Eigen::Vector2d sumC = Eigen::Vector2d::Zero();
    int count = 0;

    for (size_t k = 0; k < pts.size() - 1; ++k) {
        const auto& Pvec = pts[k];      // 旋转前
        const auto& Qvec = pts[k + 1];  // 旋转后

        if (Pvec.size() != Qvec.size() || Pvec.empty()) {
            std::cerr << "第 " << k << " 组点集数量不一致或为空！\n";
            continue;
        }

        size_t n = Pvec.size();
        Eigen::MatrixXd P(2, n), Q(2, n);
        for (size_t i = 0; i < n; ++i) {
            P.col(i) << Pvec[i].x(), Pvec[i].y();
            Q.col(i) << Qvec[i].x(), Qvec[i].y();
        }

        Eigen::Vector2d meanP = P.rowwise().mean();
        Eigen::Vector2d meanQ = Q.rowwise().mean();

        Eigen::MatrixXd P_centered = P.colwise() - meanP;
        Eigen::MatrixXd Q_centered = Q.colwise() - meanQ;

        Eigen::Matrix2d H = P_centered * Q_centered.transpose();
        Eigen::JacobiSVD<Eigen::Matrix2d> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);
        Eigen::Matrix2d R = svd.matrixV() * svd.matrixU().transpose();

        if (R.determinant() < 0) {
            Eigen::Matrix2d V = svd.matrixV();
            V.col(1) *= -1;
            R = V * svd.matrixU().transpose();
        }

        // 计算旋转中心
        Eigen::Vector2d Ivec = Eigen::Vector2d::Zero();  // identity placeholder
        Eigen::Vector2d C = (Eigen::Matrix2d::Identity() - R).inverse() * (meanQ - R * meanP);
        sumC += C;
        count++;

        // 计算旋转角度 (弧度) -> 转换为度
        double angle_rad = std::atan2(R(1, 0), R(0, 0));
        double angle_deg = angle_rad * 180.0 / M_PI;

        std::cout << "第 " << k << " 组旋转中心: [" << C.transpose() << "]，旋转矩阵对应角度: " << angle_deg << " 度\n";
    }

    if (count > 0)
        return sumC / count;  // 多组旋转中心取平均
    else
        return Eigen::Vector2d::Zero();
}
// -------- Taubin圆拟合单组点 --------
Eigen::Vector2d fitCircleTaubin(const std::vector<Eigen::Vector2d>& pts) {
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

// -------- 多组旋转前后点拟合旋转中心 --------
Eigen::Vector2d computeRotationCenterCircleFit(const std::vector<std::vector<Eigen::Vector2d>>& pts) {
    if (pts.size() < 2) {
        std::cerr << "至少需要两组点集！\n";
        return Eigen::Vector2d::Zero();
    }

    std::vector<Eigen::Vector2d> centers;

    for (size_t k = 0; k < pts.size() - 1; ++k) {
        // 每组旋转前后点差值向量
        std::vector<Eigen::Vector2d> delta_pts;
        const auto& P = pts[k];
        const auto& Q = pts[k + 1];

        if (P.size() != Q.size() || P.empty()) {
            std::cerr << "第 " << k << " 组点集数量不一致或为空！\n";
            continue;
        }

        for (size_t i = 0; i < P.size(); ++i) {
            // 构造旋转后点相对于旋转前点的圆弧
            Eigen::Vector2d delta = Q[i] - P[i];
            Eigen::Vector2d mid = (P[i] + Q[i]) / 2.0;    // 圆弧中点
            Eigen::Vector2d orth(-delta.y(), delta.x());  // 垂直方向
            delta_pts.push_back(mid + orth);              // 偏移一点用于拟合圆
        }

        Eigen::Vector2d c = fitCircleTaubin(delta_pts);
        std::cout << "第 " << k << " 组拟合旋转中心: [" << c.transpose() << "]\n";
        centers.push_back(c);
    }

    // 多组旋转中心取平均
    Eigen::Vector2d sum = Eigen::Vector2d::Zero();
    for (const auto& c : centers) sum += c;
    Eigen::Vector2d final_center = sum / centers.size();

    std::cout << "平均旋转中心: [" << final_center.transpose() << "]\n";
    return final_center;
}
// 单点解析旋转中心计算（旋转角度已知，角度单位：度）
Eigen::Vector2d computeRotationCenterSingle(const Eigen::Vector2d& p1, const Eigen::Vector2d& p2, double theta_deg) {
    double theta = theta_deg * M_PI / 180.0;  // 转成弧度
    double cos_t = std::cos(theta);
    double sin_t = std::sin(theta);

    double denom = cos_t - 1.0;
    if (std::abs(denom) < 1e-8) {
        std::cerr << "旋转角度过小或接近360°，无法解析求中心！\n";
        return Eigen::Vector2d::Zero();
    }

    double x_c = 0.5 * ((p2.y() - p1.y()) * sin_t + (p2.x() + p1.x()) * cos_t - (p2.x() + p1.x())) / denom;
    double y_c = 0.5 * ((p2.y() + p1.y()) * cos_t - (p2.x() - p1.x()) * sin_t - (p2.y() + p1.y())) / denom;

    return Eigen::Vector2d(x_c, y_c);
}

// 多组旋转前后点集计算旋转中心
Eigen::Vector2d computeRotationCenterMulti(const std::vector<std::vector<Eigen::Vector2d>>& pts, double theta_deg) {
    if (pts.size() < 2) {
        std::cerr << "至少需要两组点集！\n";
        return Eigen::Vector2d::Zero();
    }

    Eigen::Vector2d sum = Eigen::Vector2d::Zero();
    size_t count = 0;

    for (size_t k = 0; k < pts.size() - 1; ++k) {
        const auto& P = pts[k];
        const auto& Q = pts[k + 1];

        if (P.size() != Q.size() || P.empty()) {
            std::cerr << "第 " << k << " 组点集数量不一致或为空！\n";
            continue;
        }

        for (size_t i = 0; i < P.size(); ++i) {
            Eigen::Vector2d c = computeRotationCenterSingle(P[i], Q[i], theta_deg);
            std::cout << "第 " << k << " 组，第 " << i << " 个点旋转中心: [" << c.transpose() << "]\n";
            sum += c;
            count++;
        }
    }

    if (count == 0) return Eigen::Vector2d::Zero();
    Eigen::Vector2d avg_center = sum / count;
    std::cout << "平均旋转中心: [" << avg_center.transpose() << "]\n";
    return avg_center;
}
bool readPointsFromTxt(const std::string& path, std::vector<Eigen::Vector2d>& pts) {
    std::ifstream fin(path);
    if (!fin.is_open()) {
        std::cerr << "无法打开文件: " << path << std::endl;
        return false;
    }

    std::string line;
    pts.clear();
    // 跳过第一行标题
    std::getline(fin, line);

    double idx, x, y;
    while (fin >> idx >> x >> y) {
        pts.emplace_back(x, y);
    }

    if (pts.empty()) {
        std::cerr << "文件 " << path << " 无有效点。" << std::endl;
        return false;
    }
    std::cout << "读取 " << path << " 成功，共 " << pts.size() << " 个点\n";
    return true;
}
// int main() {
//     // -------- 1. 读取像素点文件 --------
//     std::vector<Eigen::Vector2d> pts1, pts2, pts3, pts4, pts5;
//     if (!readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform10.txt", pts1)) return -1;
//     if (!readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform21.txt", pts2)) return -1;
//     if (!readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform301.txt", pts3)) return -1;
//     if (!readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform401.txt", pts4)) return -1;
//     if (!readPointsFromTxt("./src/telecentricLineCalibrator/matlab/xysita/chessboard_platform501.txt", pts5)) return -1;

//     // -------- 2. 相机参数 --------
//     Eigen::Matrix3d K;
//     K << 47.283237490301396, -0.657929607742621, 15551.964431991371, 0.0, 47.05230788272559, 8043.186819107249, 0.0, 0.0, 1.0;

//     Eigen::Matrix<double, 1, 5> coff_dis;
//     coff_dis << -5.363602460785097e-10, -6.586873205793823e-07, -3.9297031624526706e-07, 2.075287196873092e-06,
//         -2.0074419972225162e-06;

//     Eigen::Vector3d v_rot(2.074776703520814, 2.0584407379860554, -0.21906585124567024);
//     Eigen::Vector3d v_trans(-249.01625128531074, -134.99191717289557, 0.0);
//     TelecentricLineCalibrator a;
//     // -------- 3. 像素 -> 相机 -> 世界坐标 --------
//     auto convertToWorld = [&](const std::vector<Eigen::Vector2d>& pts_px) {
//         Eigen::MatrixXd mat_px(pts_px.size(), 2);
//         for (size_t i = 0; i < pts_px.size(); ++i) mat_px.row(i) = pts_px[i];
//         Eigen::MatrixXd world = a.cameraToWorldCoordinates(mat_px, v_rot, v_trans);
//         std::vector<Eigen::Vector2d> res;
//         for (int i = 0; i < world.rows(); ++i) res.emplace_back(world(i, 0), world(i, 1));
//         return res;
//     };

//     pts1 = convertToWorld(pts1);
//     pts2 = convertToWorld(pts2);
//     pts3 = convertToWorld(pts3);
//     pts4 = convertToWorld(pts4);
//     pts5 = convertToWorld(pts5);
//     for (const auto& a : pts5) {
//         std::cout << "点: " << a.transpose() << std::endl;
//     }

//     // -------- 4. 求方向与旋转中心 --------
//     Eigen::Vector3d x_dir = computeXDirection(pts1, pts2);
//     std::cout << "X方向单位向量: [" << x_dir.transpose() << "]\n";

//     std::vector<std::vector<Eigen::Vector2d>> ptsRo{pts3, pts4, pts5};
//     Eigen::Vector2d C_est = computeRotationCenterSequential(ptsRo);
//     std::cout << "旋转中心坐标: [" << C_est.transpose() << "]\n";

//     return 0;
// }
int main() {
    TelecentricLineCalibrator calib;

    // -------------------- 相机内参 --------------------
    Eigen::Matrix3d K;
    K << 47.283237490301396, -0.657929607742621, 15551.964431991371, 0.0, 47.05230788272559, 8043.186819107249, 0.0, 0.0, 1.0;

    // -------------------- 畸变参数 --------------------
    Eigen::Matrix<double, 1, 5> coff_dis;
    coff_dis << -5.363602460785097e-10, -6.586873205793823e-07, -3.9297031624526706e-07, 2.075287196873092e-06,
        -2.0074419972225162e-06;

    // -------------------- 外参 --------------------
    Eigen::Vector3d v_rot(2.074776703520814, 2.0584407379860554, -0.21906585124567024);
    Eigen::Vector3d v_trans(-249.01625128531074, -134.99191717289557, 0.0);

    // -------------------- 原始世界平面点 (示例) --------------------
    std::vector<Eigen::Vector2d> worldPts;
    worldPts.emplace_back(66.000000, 74.000000);
    worldPts.emplace_back(66.000000, 71.000000);
    worldPts.emplace_back(66.000000, 68.000000);

    std::cout << "原始世界坐标:\n";
    for (auto& p : worldPts) std::cout << p.transpose() << "\n";

    // -------------------- 世界 -> 相机(平面仿射) --------------------
    // 先得到R_full（从v_rot），再取R2, t2
    cv::Mat rvec(3, 1, CV_64F), Rcv;
    for (int i = 0; i < 3; ++i) rvec.at<double>(i, 0) = v_rot(i);
    cv::Rodrigues(rvec, Rcv);
    Eigen::Matrix3d R_full;
    cv::cv2eigen(Rcv, R_full);
    Eigen::Matrix2d R2 = R_full.block<2, 2>(0, 0);
    Eigen::Vector2d t2 = v_trans.head<2>();

    Eigen::MatrixXd cam_norm(worldPts.size(), 2);
    for (size_t i = 0; i < worldPts.size(); ++i) {
        Eigen::Vector2d cw = R2 * worldPts[i] + t2;  // 相机归一化平面坐标
        cam_norm.row(i) = cw.transpose();
    }
    std::cout << "\n世界->相机归一化坐标:\n" << cam_norm << "\n";

    // -------------------- 相机(归一化) -> 畸变 -> 像素 --------------------
    // 1) 构建 Nx2 矩阵供 distort 使用（distort 返回 Nx3 齐次）
    Eigen::MatrixXd camPts2 = cam_norm;                             // Nx2
    Eigen::MatrixXd distortedH = calib.distort(coff_dis, camPts2);  // Nx3, 每行是 [xd, yd, 1]'
    // 2) 乘内参得到像素齐次，再归一化
    Eigen::MatrixXd pixelPts(cam_norm.rows(), 2);
    for (int i = 0; i < distortedH.rows(); ++i) {
        Eigen::Vector3d uvw = K * distortedH.row(i).transpose();  // K * [xd; yd; 1]
        pixelPts(i, 0) = uvw(0) / uvw(2);
        pixelPts(i, 1) = uvw(1) / uvw(2);
    }
    std::cout << "\n相机(归一化) -> 畸变 -> 像素坐标:\n" << pixelPts << "\n";

    // -------------------- 像素 -> 相机（去畸变+K^-1） --------------------
    Eigen::MatrixXd cam_back = calib.pixelToCameraCoordinates(pixelPts, K, coff_dis);
    std::cout << "\n像素 -> 相机(去畸变归一化):\n" << cam_back << "\n";

    // -------------------- 相机 -> 世界（平面逆） --------------------
    Eigen::MatrixXd world_back = calib.cameraToWorldCoordinates(cam_back, v_rot, v_trans);
    std::cout << "\n像素 -> 世界(反算):\n" << world_back << "\n";

    // -------------------- 误差评估 --------------------
    std::cout << "\n重建误差 (欧氏距离)：\n";
    for (int i = 0; i < world_back.rows(); ++i) {
        Eigen::Vector2d orig = worldPts[i];
        Eigen::Vector2d rec = world_back.row(i).transpose();
        double e = (orig - rec).norm();
        std::cout << "点 " << i << " 误差 = " << e << "\n";
    }

    return 0;
}

#include "py_telecentric_optimizer.h"
bool TelecentricPYOptimizer::invokeTelecentricCalibration() {
    PLOGD << "正在进行非线性优化";
    _putenv("PYTHONHOME=D:\\anaconda\\envs\\Telecentric-Calibration");
    _putenv(
        "PYTHONPATH=D:\\anaconda\\envs\\Telecentric-Calibration\\Lib;"
        "D:\\anaconda\\envs\\Telecentric-Calibration\\Lib\\site-packages;"
        ".\\src\\telecentricLineCalibrator\\python\\Telecentric-Calibration-main");

    try {
        pybind11::scoped_interpreter guard{};

        pybind11::exec(R"(
            import sys
            print('Python from:', sys.executable)
            print('sys.path =', sys.path)
        )");

        std::string scriptPath =
            R"(D:\Code\CISCamera_DALSA\src\telecentricLineCalibrator\python\Telecentric-Calibration-main\load.py)";

        pybind11::eval_file(scriptPath);
        return true;

    } catch (pybind11::error_already_set &e) {
        std::cerr << "❌ Python 执行错误:\n" << std::endl;
        return false;
    }
}

bool TelecentricPYOptimizer::optTelecentricExtrinsicParameters(const Eigen::Matrix3d &K,
                                                               const Eigen::Matrix<double, 1, 5> &coff_dis,
                                                               const std::vector<Eigen::Vector2d> imgPts, Eigen::Vector3d &v_rot,
                                                               Eigen::Vector3d &v_trans, const int W, const int H,
                                                               const double spacingMM) {
    std::vector<Eigen::Vector2d> worldPts;
    worldPts.reserve(W * H);
    for (int r = 0; r < H; ++r)
        for (int c = 0; c < W; ++c) worldPts.emplace_back(c * spacingMM, r * spacingMM);
    _putenv("PYTHONHOME=D:\\anaconda\\envs\\Telecentric-Calibration");
    _putenv(
        "PYTHONPATH=D:\\anaconda\\envs\\Telecentric-Calibration\\Lib;"
        "D:\\anaconda\\envs\\Telecentric-Calibration\\Lib\\site-packages;"
        ".\\src\\telecentricLineCalibrator\\python\\Telecentric-Calibration-main");

    try {
        pybind11::scoped_interpreter guard{};
#pragma region "定义 Python 函数" {
        // ------------------ 定义 Python 函数 ------------------
        pybind11::exec(R"(
import glob, os
import numpy as np
import cv2
from scipy.optimize import curve_fit, least_squares

def to_homogeneous(points):
    if points.shape[1] == 2 or points.shape[1] == 3:
        ones = np.ones((points.shape[0], 1))
        return np.hstack((points, ones))
    return points

def distort(k, normalized_proj):
    x, y = normalized_proj[:, 0], normalized_proj[:, 1]
    r = x ** 2 + y ** 2

    k1, h1, h2, s1, s2 = k

    deltaX = k1*x*r + h1*(3*x*x + y*y) + 2*h2*x*y + s1*r
    deltaY = k1*y*r + 2*h1*x*y + h2*(x*x + 3*y*y) + s2*r

    x_prime = x + deltaX
    y_prime = y + deltaY

    distorted_proj = np.hstack((x_prime[:, None], y_prime[:, None]))
    return to_homogeneous(distorted_proj)

def refine_params_with_distortion_external_only(points_world, points_pixel, K, coff_dis, v_rot, v_trans):
    points_pixel = np.array(points_pixel)
    points_world = np.array(points_world)
    print("\n-- K (camera matrix) --")
    print(K)
    coff_dis_opt = np.array(coff_dis)
    # convert to homogeneous (x,y,1)
    points_world = np.hstack([points_world, np.ones((points_world.shape[0], 1))])
    # add batch dimension
    points_world = points_world.reshape(1, points_world.shape[0], 3)
    points_pixel = points_pixel.reshape(1, points_pixel.shape[0], 2)
    v_rot = v_rot.reshape(1, 3)
    v_trans = np.array(v_trans)[:2].reshape(1, 2)
    n_views = len(v_rot)
    print("===== Inputs =====")
    print("-- points_world --")
    print("type:", type(points_world))
    print("shape:", points_world.shape)

    print("\n-- points_pixel --")
    print("type:", type(points_pixel))
    print("shape:", points_pixel.shape)

    print("\n-- K (camera matrix) --")
    print("type:", type(K))
    print("shape:", K.shape)

    print("\n-- coff_dis (distortion coefficients) --")
    print("type:", type(coff_dis))
    print("shape:", coff_dis.shape)

    print("\n-- v_rot --")
    print("type:", type(v_rot))
    print("shape:", v_rot.shape)

    print("\n-- v_trans --")
    print("type:", type(v_trans))
    print("shape:", v_trans.shape)
    print("==================\n")
    def compute_reproj_loss(v_rot_list, v_trans_list):
        total_err = 0.0
        total_points = 0
        for i in range(n_views):
            world_points = points_world[i].reshape(-1, 3)
            world_points[:, 2] = 1
            pixel_gt = points_pixel[i].reshape(-1, 2)

            rot_mat, _ = cv2.Rodrigues(np.array(v_rot_list[i], dtype=np.float64).reshape(3))
            R2 = rot_mat[:2, :2]
            t2 = np.array(v_trans_list[i], dtype=np.float64).reshape(2)
            for pt_idx in range(world_points.shape[0]):
                xy = world_points[pt_idx, :2]
                cam_xy = R2 @ xy + t2
                camPt = cam_xy.reshape(1, 2)
                distortedH = distort(coff_dis_opt, camPt)
                uvw = K @ distortedH[0].T
                uv_hat = np.array([uvw[0] / uvw[2], uvw[1] / uvw[2]])
                total_err += np.linalg.norm(uv_hat - pixel_gt[pt_idx])
                total_points += 1
        mean_loss = total_err / total_points
        return mean_loss
    initial_loss = compute_reproj_loss(v_rot, v_trans)
    print(f"Initial reprojection error: {initial_loss:.6f}")
    packed_params = []
    for i in range(n_views):
        packed_params.extend(list(v_rot[i]))
        packed_params.extend(list(v_trans[i]))
    packed_params = np.array(packed_params, dtype=np.float64)
    def project_external_only(x_data, *params):
        y_pre_list = []
        for i in range(n_views):
            idx = i * 5
            rt = params[idx: idx + 5]
            rot_vec = np.array(rt[:3], dtype=np.float64).reshape(3)
            trans_vec = np.array(rt[3:], dtype=np.float64).reshape(2)

            world_points = np.array(x_data[i]).reshape(-1, 3)
            world_points[:, 2] = 1

            rot_mat, _ = cv2.Rodrigues(rot_vec)
            rt_matri = np.eye(3)
            rt_matri[:2, :2] = rot_mat[:2, :2]
            rt_matri[:2, 2] = trans_vec

            y_normalized = (rt_matri @ world_points.T).T
            y_distorted = distort(coff_dis_opt, y_normalized)
            y_pixel = (K @ y_distorted.T).T
            y_pre_list.append(y_pixel[:, :2])
        return np.array(y_pre_list).reshape(-1)
    popt, _ = curve_fit(
        project_external_only,
        points_world,
        points_pixel.reshape(-1),
        p0=packed_params,
        maxfev=10000000
    )
    v_rot_refined = []
    v_trans_refined = []
    for i in range(n_views):
        v_rot_refined.append(popt[i * 5: i * 5 + 3])
        v_trans_refined.append(popt[i * 5 + 3: (i + 1) * 5])
    v_rot_refined = np.array(v_rot_refined)
    v_trans_refined = np.array(v_trans_refined)
    final_loss = compute_reproj_loss(v_rot_refined, v_trans_refined)
    print(f"Final reprojection error: {final_loss:.6f}")
    print(f"Error improvement: {initial_loss - final_loss:.6f}")
    return final_loss, v_rot_refined, v_trans_refined
)");
#pragma endregion }
        pybind11::module sys = pybind11::module::import("sys");
        pybind11::module main = pybind11::module::import("__main__");
        pybind11::object globals = main.attr("__dict__");

        // ------------------ C++ → Python 数据 ------------------
        int N = (int)worldPts.size();

        pybind11::array_t<double> py_world({N, 2});
        pybind11::array_t<double> py_pixel({N, 2});

        auto bufW = py_world.mutable_unchecked<2>();
        auto bufP = py_pixel.mutable_unchecked<2>();

        for (int i = 0; i < N; i++) {
            bufW(i, 0) = worldPts[i].x();
            bufW(i, 1) = worldPts[i].y();
            bufP(i, 0) = imgPts[i].x();
            bufP(i, 1) = imgPts[i].y();
        }

        pybind11::array_t<double> py_K({3, 3});
        auto bufK = py_K.mutable_unchecked<2>();
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++) bufK(r, c) = K(r, c);

        pybind11::array_t<double> py_coff({5});
        auto bufC = py_coff.mutable_unchecked<1>();
        for (int i = 0; i < 5; ++i) bufC(i) = coff_dis(0, i);

        pybind11::array_t<double> py_vrot({3});
        pybind11::array_t<double> py_vtrans({3});
        auto br = py_vrot.mutable_unchecked<1>();
        auto bt = py_vtrans.mutable_unchecked<1>();
        for (int i = 0; i < 3; ++i) {
            br(i) = v_rot(i);
            bt(i) = v_trans(i);
        }

        // ------------------ 调用 Python 函数 ------------------
        pybind11::object func = globals["refine_params_with_distortion_external_only"];

        pybind11::object result = func(py_world, py_pixel, py_K, py_coff, py_vrot, py_vtrans);

        double ret = result.cast<pybind11::tuple>()[0].cast<double>();

        auto v_rot_opt_py = result.cast<pybind11::tuple>()[1].cast<pybind11::array_t<double>>();
        auto v_trans_opt_py = result.cast<pybind11::tuple>()[2].cast<pybind11::array_t<double>>();

        auto rr = v_rot_opt_py.unchecked<2>();
        auto tt = v_trans_opt_py.unchecked<2>();

        Eigen::Vector3d v_rot_opt;
        Eigen::Vector2d v_trans_opt;  // 注意这里现在是 2D

        for (int i = 0; i < 3; ++i) {
            v_rot_opt(i) = rr(0, i);  // 取第 0 行
        }
        for (int i = 0; i < 2; ++i) {
            v_trans_opt(i) = tt(0, i);  // 取第 0 行
        }
        Eigen::Vector3d v_trans_opt_3d;          // 新的 3D 向量
        v_trans_opt_3d.head<2>() = v_trans_opt;  // 前两位拷贝原来的 2D
        v_trans_opt_3d(2) = 0.0;                 // 第三位设为 0
        v_rot = v_rot_opt;
        v_trans = v_trans_opt_3d;
        std::cout << "ret = " << ret << "\n";
        std::cout << "v_rot = " << v_rot.transpose() << "\n";
        std::cout << "v_trans = " << v_trans.transpose() << "\n";
    } catch (std::exception &e) {
        std::cerr << "Python 异常: " << e.what() << "\n";
        return false;
    }
    return true;
}

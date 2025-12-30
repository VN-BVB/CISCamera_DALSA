import numpy as np
import cv2

from scipy.optimize import curve_fit, least_squares
def read_points_from_txt(path):
    """
    从txt文件中读取点数据
    :param path: txt文件路径
    :return: (成功标志, 点数据)
    """
    try:
        with open(path, 'r') as fin:
            # 跳过第一行标题
            first_line = fin.readline()
            if not first_line:
                print(f"文件 {path} 为空")
                return False, None

            points = []
            for line in fin:
                # 尝试解析每一行数据
                parts = line.strip().split()
                if len(parts) >= 3:
                    try:
                        idx = int(parts[0])
                        x = float(parts[1])
                        y = float(parts[2])
                        points.append([x, y])
                    except ValueError:
                        continue

            if not points:
                print(f"文件 {path} 无有效点")
                return False, None

            print(f"读取 {path} 成功，共 {len(points)} 个点")
            return True, np.array(points, dtype=np.float32)
    except Exception as e:
        print(f"无法打开或读取文件 {path}: {str(e)}")
        return False, None

# -------------------- 迭代去畸变 --------------------
def undistort_points_iter(points_px, K, coff_dis, max_iter=20, tol=1e-6):
    fx, fy = K[0, 0], K[1, 1]
    cx, cy = K[0, 2], K[1, 2]
    ifx, ify = 1.0 / fx, 1.0 / fy

    k1, h1, h2, s1, s2 = coff_dis

    undistorted = np.zeros_like(points_px, dtype=np.float64)

    for i, (u, v) in enumerate(points_px):
        x0 = (u - cx) * ifx
        y0 = (v - cy) * ify
        x, y = x0, y0

        error = 1e9
        it = 0

        while it < max_iter and error > tol:
            r = x * x + y * y

            deltaX = (
                k1 * x * r
                + h1 * (3 * x * x + y * y)
                + 2 * h2 * x * y
                + s1 * r
            )
            deltaY = (
                k1 * y * r
                + 2 * h1 * x * y
                + h2 * (x * x + 3 * y * y)
                + s2 * r
            )

            x = x0 - deltaX
            y = y0 - deltaY

            r = x * x + y * y
            xd = (
                x
                + k1 * x * r
                + h1 * (3 * x * x + y * y)
                + 2 * h2 * x * y
                + s1 * r
            )
            yd = (
                y
                + k1 * y * r
                + 2 * h1 * x * y
                + h2 * (x * x + 3 * y * y)
                + s2 * r
            )

            u_proj = xd * fx + cx
            v_proj = yd * fy + cy

            error = np.sqrt((u_proj - u) ** 2 + (v_proj - v) ** 2)
            it += 1

        undistorted[i, 0] = x * fx + cx
        undistorted[i, 1] = y * fy + cy

    return undistorted


# -------------------- 像素 → 相机坐标 --------------------
def pixel_to_camera(points_px, K, coff_dis):
    pts = points_px.astype(np.float64)

    if np.linalg.norm(coff_dis) > 1e-15:
        pts = undistort_points_iter(pts, K, coff_dis)

    ones = np.ones((pts.shape[0], 1))
    homo = np.hstack([pts, ones])

    K_inv = np.linalg.inv(K)
    cam = (K_inv @ homo.T).T

    cam_norm = cam[:, :2] / cam[:, 2:3]
    return cam_norm


# -------------------- 相机 → 世界（平面） --------------------
def camera_to_world(cam_pts, rvec, tvec):
    R, _ = cv2.Rodrigues(rvec.reshape(3, 1))

    R2 = R[:2, :2]
    t2 = tvec[:2]

    R2_inv = np.linalg.inv(R2)

    np.set_printoptions(precision=15, suppress=False)

    world = (R2_inv @ (cam_pts - t2).T).T

    return world


# -------------------- 像素 → 世界（总入口） --------------------
def pixel_to_world(points_px, K, D, rvec, tvec):
    cam_norm = pixel_to_camera(points_px, K, D)
    world_pts = camera_to_world(cam_norm, rvec, tvec)
    return world_pts
def compute_center_angle(P, Q):
    """
    P, Q: Nx2
    return: C(2,), angle(rad)
    """
    P = np.asarray(P)
    Q = np.asarray(Q)

    meanP = P.mean(axis=0)
    meanQ = Q.mean(axis=0)

    Pc = (P - meanP).T
    Qc = (Q - meanQ).T

    H = Pc @ Qc.T
    U, _, Vt = np.linalg.svd(H)
    R = Vt.T @ U.T

    if np.linalg.det(R) < 0:
        Vt[1, :] *= -1
        R = Vt.T @ U.T

    I = np.eye(2)
    C = np.linalg.inv(I - R) @ (meanQ - R @ meanP)

    angle = np.arctan2(R[1, 0], R[0, 0])
    return C, angle
def residual_extrinsics(theta, K, D, pts, dx, dy, ang):
    """
    LM 非线性优化残差函数

    theta: [rx, ry, rz, tx, ty] 旋转向量和平移向量（只用前两项）
    pts: [p1, p2, p3, p4, p5] 五组像素点列表，每组 Nx2
    dx, dy: 平移约束（单位 mm）
    ang: 旋转约束（单位 rad）
    """
    rvec = theta[:3]
    tvec = theta[3:5]

    p1, p2, p3, p4, p5 = pts

    # 将像素点转换到世界坐标系
    w1 = pixel_to_world(p1, K, D, rvec, np.append(tvec, 0.0))
    w2 = pixel_to_world(p2, K, D, rvec, np.append(tvec, 0.0))
    w3 = pixel_to_world(p3, K, D, rvec, np.append(tvec, 0.0))
    w4 = pixel_to_world(p4, K, D, rvec, np.append(tvec, 0.0))
    w5 = pixel_to_world(p5, K, D, rvec, np.append(tvec, 0.0))

    res = []

    # ---------- 平移约束 ----------
    for i in range(len(w1)):
        res.extend(w2[i] - w1[i] - np.array([dx, 0.0]))
        res.extend(w3[i] - w2[i] - np.array([0.0, dy]))

    # ---------- 旋转约束 ----------
    C34, ang34 = compute_center_angle(w3, w4)
    C45, ang45 = compute_center_angle(w4, w5)

    res.append(ang34 - ang)
    res.append(ang45 - ang)

    # 旋转中心约束（假设在原点）
    res.extend(C34)
    res.extend(C45)

    return np.array(res)


def refine_extrinsics_lm(K, D, pts, rvec_init, tvec_init, dx, dy, ang_deg):
    """
    利用 Levenberg-Marquardt 方法优化外参（旋转向量和平移向量）

    K: 内参矩阵 3x3
    D: 畸变系数 1x5
    pts: 五组像素点列表 [p1, p2, p3, p4, p5]
    rvec_init: 初始旋转向量 [rx, ry, rz]
    tvec_init: 初始平移向量 [tx, ty]
    dx, dy: 平移量约束
    ang_deg: 旋转角约束（单位 deg）
    """
    ang_rad = np.deg2rad(ang_deg)

    # 打包初始外参
    init_params = np.hstack([rvec_init, tvec_init[:2]])

    result = least_squares(
        residual_extrinsics,
        init_params,
        method="trf",
        args=(K, D, pts, dx, dy, ang_rad),
        verbose=2,
        max_nfev=2000
    )

    rvec_opt = result.x[:3]
    tvec_opt = result.x[3:5]
    final_rms = np.sqrt(np.mean(result.fun ** 2))
    print("rvec:", rvec_opt)
    print("tvec:", tvec_opt)
    print("RMS:", final_rms)
    return rvec_opt, tvec_opt, final_rms


# ==================== main 测试 ====================
if __name__ == "__main__":
    K = np.array([
        [47.283237490301396, -0.657929607742621, 15551.964431991371],
        [0.0, 47.05230788272559, 8043.186819107249],
        [0.0, 0.0, 1.0]
    ])

    D = np.array([
        -5.363602460785097e-10,
        -6.586873205793823e-07,
        -3.9297031624526706e-07,
         2.075287196873092e-06,
        -2.0074419972225162e-06
    ])

    # 初始旋转向量和平移向量（平面只用前两项）
    rvec_init = np.array([ 0.165329144509388,
 0.234669704479263,
-1.569108596273157])
    tvec_init = np.array([-208.387787176078490,-86.695384037745484, 0.0])

    # 读取五组点
    paths = [
        "D:/Code/CISCamera_DALSA/src/telecentricLineCalibrator/matlab/xysita/chessboard_platform10.txt",
        "D:/Code/CISCamera_DALSA/src/telecentricLineCalibrator/matlab/xysita/chessboard_platform21.txt",
        "D:/Code/CISCamera_DALSA/src/telecentricLineCalibrator/matlab/xysita/chessboard_platform30d.txt",
        "D:/Code/CISCamera_DALSA/src/telecentricLineCalibrator/matlab/xysita/chessboard_platform40d.txt",
        "D:/Code/CISCamera_DALSA/src/telecentricLineCalibrator/matlab/xysita/chessboard_platform50d.txt",
    ]
    pts = [read_points_from_txt(p)[1] for p in paths]

    rvec_opt, tvec_opt, rms = refine_extrinsics_lm(K, D, pts, rvec_init, tvec_init,50.0,50.0,45.0)

    # ====================== 测试像素点 ======================
    px = np.array([[6937.323892, 2816.240515]], dtype=np.float64)

    # ====================== 参考世界坐标 ======================
    world_ref = np.array([[25.0, 25.0]])

    # ====================== 初始外参 ======================
    world_init = pixel_to_world(
        px,
        K,
        D,
        rvec_init,  # 初始 rvec
        tvec_init  # 初始 tvec
    )

    # ====================== LM 优化后外参 ======================
    world_opt = pixel_to_world(
        px,
        K,
        D,
        rvec_opt,
        tvec_opt
    )

    # ====================== 误差计算 ======================
    err_init = world_init - world_ref
    err_opt = world_opt - world_ref

    # ====================== 打印 ======================
    print("========== 像素输入 ==========")
    print(px)

    print("\n========== 参考世界坐标 ==========")
    print(world_ref)

    print("\n========== 初始外参 → 世界坐标 ==========")
    print(world_init)
    print("误差 (init - ref):", err_init)
    print("欧氏误差:", np.linalg.norm(err_init, axis=1))

    print("\n==========  优化后 → 世界坐标 ==========")
    print(world_opt)
    print("误差 (opt - ref):", err_opt)
    print("欧氏误差:", np.linalg.norm(err_opt, axis=1))
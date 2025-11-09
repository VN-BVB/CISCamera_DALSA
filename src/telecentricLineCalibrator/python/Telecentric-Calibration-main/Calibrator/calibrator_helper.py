import glob,os
import numpy as np
import cv2
from nuitka.build.inline_copy.clcache.clcache.caching import cache
from scipy.optimize import curve_fit, least_squares
from numba import njit


# 得到给定文件夹下所有图片路径，按名称排序。
def get_sorted_image_paths(folder_path):
    """
    得到给定文件夹下所有图片路径，按名称排序。
    :param folder_path: 文件夹路径
    :return: 图片路径列表
    """
    # 获取所有图片文件路径
    image_paths = glob.glob(os.path.join(folder_path, '*'))

    # 过滤出图片文件（根据扩展名）
    image_paths = [path for path in image_paths if
                   path.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif', '.tiff'))]
    if len(image_paths) == 0:
        ret = 0
    else:
        ret = 1
        # 按文件名排序
        image_paths.sort(key=lambda x: os.path.basename(x))
    return ret, image_paths


# 计算坐标数组的齐次坐标
def to_homogeneous(points):
    """
    计算nx2或nx3 numpy 坐标数组的齐次坐标
    Args:
        points: nx2或nx3坐标数组
    Returns:
        nx3或nx4齐次坐标数组
    """
    # Check if input is a Nx2 array or Nx3 array
    if points.shape[1] == 2 or points.shape[1] == 3:
        ones = np.ones((points.shape[0], 1))
        homogeneous_points = np.hstack((points, ones))
    else:
        raise ValueError("齐次坐标转换失败，输入应该是nx2或nx3坐标数组")
    return homogeneous_points


# 投影点加畸变，考虑5个畸变系数(k1,k2,p1,p2,k3)
# def distort(k, normalized_proj):
#     """
#     投影点加畸变
#     Args:
#         k: 畸变系数--(k1,k2,p1,p2,k3)
#         normalized_proj: 投影坐标点(nx3)：成像平面齐次坐标。(x,y,1)
#     Returns:
#         带畸变的投影点数组
#     """
#
#     x, y = normalized_proj[:, 0], normalized_proj[:, 1]
#
#     # Calculate radii
#     r = np.sqrt(x ** 2 + y ** 2)
#
#     k1, k2, p1, p2, k3 = k
#
#     # Calculate distortion effects
#     D = k1 * r ** 2 + k2 * r ** 4 + k3 * r ** 6
#     deltaX = 2 * p1 * x * y + p2 * (r**2 + 2 * x ** 2)
#     deltaY = p1 * (r**2 + 2 * y ** 2) + 2 * p2 * x * y
#
#     # Calculate distorted normalized projection values
#     x_prime = x * (1. + D) + deltaX
#     y_prime = y * (1. + D) + deltaY
#
#     distorted_proj = np.hstack((x_prime[:, np.newaxis], y_prime[:, np.newaxis]))
#     distorted_proj = to_homogeneous(distorted_proj)
#     return distorted_proj
def distort(k, normalized_proj):
    """
    投影点加畸变
    Args:
        k: 畸变系数--(k1,h1,h2,s1,s2)
        normalized_proj: 投影坐标点(nx3)：成像平面齐次坐标。(x,y,1)
    Returns:
        带畸变的投影点数组
    """
    x, y = normalized_proj[:, 0], normalized_proj[:, 1]
    # Calculate radii
    r = x ** 2 + y ** 2
    k1,h1,h2,s1,s2 = k
    # Calculate distortion effects
    deltaX = k1*x*r+h1*(3* x ** 2 + y ** 2)+2*h2*x*y+s1*r
    deltaY = k1*y*r+2*h1*x*y+h2*(x**2+3*y**2)+s2*r

    # Calculate distorted normalized projection values
    x_prime = x  + deltaX
    y_prime = y  + deltaY

    distorted_proj = np.hstack((x_prime[:, np.newaxis], y_prime[:, np.newaxis]))
    distorted_proj = to_homogeneous(distorted_proj)
    return distorted_proj


# 优化内参和所有外参（不带畸变）
def refine_params_without_distortion(points_world, points_pixel, mat_intri, v_rot, v_trans):
    """
    优化内参和所有外参
    Args:
        points_world: 控制点世界坐标
        points_pixel: 控制点像素坐标
        mat_intri: 相机内参3x3
        v_rot: 旋转向量nx3
        v_trans: 位移向量nx2

    Returns:
        重投影误差，优化后的内参，所有外参。
    """
    points_pixel = np.array(points_pixel)
    # print(points_pixel,points_pixel.reshape(-1))
    points_world = np.array(points_world)
    # 打包所有参数
    packed_params = []
    # 5个内参
    alpha, beta, gamma, u_c, v_c = mat_intri[0, 0], mat_intri[1, 1], mat_intri[0, 1], mat_intri[0, 2], mat_intri[1, 2]
    packed_params.extend([alpha, beta, gamma, u_c, v_c])
    # 打包所有外参
    for i in range(len(v_rot)):
        rho_x, rho_y, rho_z = v_rot[i]
        t_x, t_y = v_trans[i]
        e = [rho_x, rho_y, rho_z, t_x, t_y]
        packed_params.extend(e)
    # 设置边界约束
    min_bounds = [-np.inf] * len(packed_params)
    max_bounds = [np.inf] * len(packed_params)
    min_bounds[3], max_bounds[3] = u_c - 2, u_c + 2
    min_bounds[4], max_bounds[4] = v_c - 2, v_c + 2
    bounds = (min_bounds, max_bounds)

    # 对参数进行优化
    def project(x_data, *params):
        # 不带畸变的投影
        K = np.eye(3)
        K[0, 0], K[1, 1], K[0, 1], K[0, 2], K[1, 2] = params[:5]
        v_RT = params[5:]
        y_pre_list = []
        for i in range(len(x_data)):
            world = np.array(x_data[i]).reshape(-1, 3)
            # 齐次坐标
            world[:, 2] = 1
            rt = v_RT[i * 5:(i + 1) * 5]
            # 使用Rodrigues公式将旋转向量转换为旋转矩阵
            rotation_matrix, _ = cv2.Rodrigues(rt[:3])
            rt_matri = np.eye(3)
            rt_matri[:2, :2] = rotation_matrix[:2, :2]
            rt_matri[:2, 2] = rt[3:5]
            # 投影三维空间点到图像平面上
            y_pre = (K @ rt_matri @ world.T).T
            y_pre = y_pre[:, :2]
            y_pre_list.append(y_pre)
        y_pre_list = np.array(y_pre_list).reshape(-1)
        return y_pre_list

    popt, pcov = curve_fit(project, points_world, points_pixel.reshape(-1), packed_params, bounds=bounds, maxfev=2000000)
    # 解包所有参数
    params_refined = popt
    intrinsics = params_refined[:5]
    # 相机内参K
    alpha, beta, gamma, u_c, v_c = intrinsics
    K = np.array([[alpha, gamma, u_c],
                  [0., beta, v_c],
                  [0., 0., 1.]])
    # 所有外参
    rt_v = params_refined[5:]
    m = int(len(rt_v) / 5)
    v_rot = []
    v_trans = []
    for i in range(m):
        v_rot.append(rt_v[i * 5:i * 5 + 3])
        v_trans.append(rt_v[i * 5 + 3:(i + 1) * 5])
    v_rot = np.array(v_rot)
    v_trans = np.array(v_trans)

    # 计算重投影误差
    loss_list = []
    # 定义保存路径（使用原始字符串避免转义问题）
    save_dir = r"D:\Code\CISCamera_DALSA\data\CISCamera_Image\txt2"
    # 确保保存目录存在
    os.makedirs(save_dir, exist_ok=True)
    for i in range(len(points_world)):
        world = points_world[i].reshape(-1, 3)
        world[:, 2] = 1
        pixel = points_pixel[i].reshape(-1, 2)
        r_m, _ = cv2.Rodrigues(v_rot[i])
        rt_matri = np.eye(3)
        rt_matri[:2, :2] = r_m[:2, :2]
        rt_matri[:2, 2] = v_trans[i].T
        # 投影三维空间点到图像平面上
        y_pre = (K @ rt_matri @ world.T).T
        txt_path = os.path.join(save_dir, f"img_{i + 1}_reproj_pts.txt")
        with open(txt_path, "w", encoding="utf-8") as f:
            f.write("# Index	X	Y\n")  # 表头
            for pt_idx in range(len(y_pre)):
                x_reproj = y_pre[pt_idx, 0]
                y_reproj = y_pre[pt_idx, 1]
                f.write(f"{pt_idx}\t{x_reproj:.6f}\t{y_reproj:.6f}\n")  # Tab分隔，保留6位小数
        loss = np.linalg.norm(y_pre[:, :2] - pixel, axis=1)
        loss_list.append(np.mean(loss))
    ret = np.mean(loss_list)
    return ret, K, v_rot, v_trans


# 优化内参和所有外参（带畸变）
def refine_params_with_distortion(points_world, points_pixel, mat_intri, coff_dis, v_rot, v_trans):
    """
    优化内参，畸变系数，所有外参
    Args:
        points_world: 控制点世界坐标
        points_pixel: 控制点像素坐标
        mat_intri: 相机内参3x3
        coff_dis: 畸变系数，5个
        v_rot: 旋转向量nx3
        v_trans: 位移向量nx2
    """
    points_pixel = np.array(points_pixel)
    # print(points_pixel,points_pixel.reshape(-1))
    points_world = np.array(points_world)
    # 打包所有参数
    packed_params = []
    # 5个内参
    alpha, beta, gamma, u_c, v_c = mat_intri[0, 0], mat_intri[1, 1], mat_intri[0, 1], mat_intri[0, 2], mat_intri[1, 2]
    # 5个畸变系数
    k1, k2, p1, p2, k3 = coff_dis
    packed_params.extend([alpha, beta, gamma, u_c, v_c, k1, k2, p1, p2, k3])
    # 打包外参
    for i in range(len(v_rot)):
        rho_x, rho_y, rho_z = v_rot[i]
        t_x, t_y = v_trans[i]
        e = [rho_x, rho_y, rho_z, t_x, t_y]
        packed_params.extend(e)
    # 设置边界约束
    min_bounds = [-np.inf] * len(packed_params)
    max_bounds = [np.inf] * len(packed_params)
    min_bounds[3], max_bounds[3] = u_c - 2, u_c + 2
    min_bounds[4], max_bounds[4] = v_c - 2, v_c + 2
    # min_bounds[5], max_bounds[5] = u_c - 10, u_c + 10
    # min_bounds[6], max_bounds[6] = v_c - 1, v_c + 1
    # min_bounds[7], max_bounds[7] = u_c - 1, u_c + 1
    # min_bounds[8], max_bounds[8] = v_c - 1, v_c + 1
    # min_bounds[9], max_bounds[9] = u_c - 1, u_c + 1

    bounds = (min_bounds, max_bounds)

    def project(x_data, *params):
        K = np.eye(3)
        K[0, 0], K[1, 1], K[0, 1], K[0, 2], K[1, 2], k1, k2, p1, p2, k3 = params[:10]
        coff_dis = np.array([k1, k2,p1,p2,k3])
        v_RT = params[10:]
        y_pre_list = []
        for i in range(len(x_data)):
            world = np.array(x_data[i]).reshape(-1, 3)
            # 齐次坐标
            world[:, 2] = 1
            rt = v_RT[i * 5:(i + 1) * 5]
            # 使用Rodrigues公式将旋转向量转换为旋转矩阵
            rotation_matrix, _ = cv2.Rodrigues(rt[:3])
            rt_matri = np.eye(3)
            rt_matri[:2, :2] = rotation_matrix[:2, :2]
            rt_matri[:2, 2] = rt[3:5]
            # 投影三维空间点到成像平面上
            y_pre = (rt_matri @ world.T).T
            y_dis = distort(coff_dis, y_pre)
            y_pre = (K @ y_dis.T).T
            y_pre = y_pre[:, :2]
            y_pre_list.append(y_pre)
        y_pre_list = np.array(y_pre_list).reshape(-1)
        return y_pre_list

    popt, pcov = curve_fit(project, points_world, points_pixel.reshape(-1), packed_params, bounds=bounds, maxfev=200000)
    # 解包所有参数
    params_refined = popt
    intrinsics = params_refined[:5]
    # 相机内参K
    alpha, beta, gamma, u_c, v_c = intrinsics
    K = np.array([[alpha, gamma, u_c],
                  [0., beta, v_c],
                  [0., 0., 1.]])
    k1, k2, p1, p2, k3 = params_refined[5:10]
    # 所有外参
    rt_v = params_refined[10:]
    m = int(len(rt_v) / 5)
    v_rot = []
    v_trans = []
    for i in range(m):
        v_rot.append(rt_v[i * 5:i * 5 + 3])
        v_trans.append(rt_v[i * 5 + 3:(i + 1) * 5])
    v_rot = np.array(v_rot)
    v_trans = np.array(v_trans)
    # 计算重投影误差
    loss_list = []
    # 定义保存路径（使用原始字符串避免转义问题）
    save_dir = r"D:\Code\CISCamera_DALSA\data\CISCamera_Image\txt3"
    # 确保保存目录存在
    os.makedirs(save_dir, exist_ok=True)
    for i in range(len(points_world)):
        world = points_world[i].reshape(-1, 3)
        world[:, 2] = 1
        pixel = points_pixel[i].reshape(-1, 2)
        r_m, _ = cv2.Rodrigues(v_rot[i])
        rt_matri = np.eye(3)
        rt_matri[:2, :2] = r_m[:2, :2]
        rt_matri[:2, 2] = v_trans[i].T
        # 投影三维空间点到成像平面上
        y_pre = (rt_matri @ world.T).T
        y_dis = distort([k1, k2, p1, p2, k3], y_pre)
        y_pre = (K @ y_dis.T).T
        y_pre = y_pre[:, :2]
        txt_path = os.path.join(save_dir, f"img_{i + 1}_reproj_pts.txt")
        with open(txt_path, "w", encoding="utf-8") as f:
            f.write("# Index	X	Y\n")  # 表头
            for pt_idx in range(len(y_pre)):
                x_reproj = y_pre[pt_idx, 0]
                y_reproj = y_pre[pt_idx, 1]
                f.write(f"{pt_idx}\t{x_reproj:.6f}\t{y_reproj:.6f}\n")  # Tab分隔，保留6位小数
        loss = np.linalg.norm(y_pre[:, :2] - pixel, axis=1)
        loss_list.append(np.mean(loss))
    ret = np.mean(loss_list)
    return ret, K, [k1, k2,p1, p2, k3], v_rot, v_trans

def refine_params_with_distortion_lm(points_world, points_pixel, mat_intri, coff_dis, v_rot, v_trans):
    # 你的前半部分不变：打包参数等
    points_pixel = np.array(points_pixel, dtype=np.float64)
    points_world = np.array(points_world, dtype=np.float64)

    packed_params = []
    alpha, beta, gamma, u_c, v_c = mat_intri[0, 0], mat_intri[1, 1], mat_intri[0, 1], mat_intri[0, 2], mat_intri[1, 2]
    k1, k2, p1, p2, k3 = coff_dis
    packed_params.extend([alpha, beta, gamma, u_c, v_c, k1, k2, p1, p2, k3])
    for i in range(len(v_rot)):
        rho_x, rho_y, rho_z = v_rot[i]
        t_x, t_y = v_trans[i]
        packed_params.extend([rho_x, rho_y, rho_z, t_x, t_y])

    x0 = np.array(packed_params, dtype=np.float64)

    # bounds（确保长度正确）
    min_bounds = np.full_like(x0, -np.inf, dtype=np.float64)
    max_bounds = np.full_like(x0, np.inf, dtype=np.float64)
    # 限制 u_c, v_c（注意索引：0..）
    min_bounds[3], max_bounds[3] = u_c - 2.0, u_c + 2.0
    min_bounds[4], max_bounds[4] = v_c - 2.0, v_c + 2.0
    # 建议限制畸变范围（可选，避免发散）
    min_bounds[5:10] = [-0.5, -0.5, -0.05, -0.05, -0.5]
    max_bounds[5:10] = [0.5, 0.5, 0.05, 0.05, 0.5]

    bounds = (min_bounds, max_bounds)

    # ---------- 定义残差函数，返回一维残差向量 ----------
    def residuals(params):
        # 防护：参数中出现 NaN 或 inf 直接返回大残差
        if np.any(np.isnan(params)) or np.any(np.isinf(params)):
            return np.ones(points_pixel.size) * 1e6

        alpha, beta, gamma, u_c, v_c = params[:5]
        k1, k2, p1, p2, k3 = params[5:10]
        coff = np.array([k1, k2, p1, p2, k3], dtype=np.float64)
        v_RT = params[10:]
        res_list = []

        # 逐视角投影并计算残差
        for i in range(len(points_world)):
            world = np.array(points_world[i], dtype=np.float64).reshape(-1, 3)
            world[:, 2] = 1.0
            rt = v_RT[i * 5:(i + 1) * 5]
            # 防护：若 rt 有 NaN/inf 跳出
            if np.any(np.isnan(rt)) or np.any(np.isinf(rt)):
                return np.ones(points_pixel.size) * 1e6
            R, _ = cv2.Rodrigues(rt[:3])
            RT_mat = np.eye(3, dtype=np.float64)
            RT_mat[:2, :2] = R[:2, :2]
            RT_mat[:2, 2] = rt[3:5]
            y_pre = (RT_mat @ world.T).T  # Nx3
            y_dis = distort(coff, y_pre)  # 你的 distort 函数必须返回 Nx3 (齐次)
            K = np.array([[alpha, gamma, u_c],
                          [0., beta, v_c],
                          [0., 0., 1.]], dtype=np.float64)
            y_proj = (K @ y_dis.T).T  # Nx3
            y2d = y_proj[:, :2]
            # 保护：若出现 NaN/inf，返回大残差
            if np.any(np.isnan(y2d)) or np.any(np.isinf(y2d)):
                return np.ones(points_pixel.size) * 1e6
            res = (y2d - points_pixel[i]).reshape(-1)
            res_list.append(res)

        return np.concatenate(res_list)

    # ---------- 每次迭代回调打印 & 写文件 ----------
    iter_state = {'idx': 0}
    loss_log_path = r"D:\Code\CISCamera_DALSA\data\CISCamera_Image\logs\lm_iter_loss.txt"
    # 清空文件
    import os
    os.makedirs(os.path.dirname(loss_log_path), exist_ok=True)
    with open(loss_log_path, 'w') as ff:
        ff.write("iter,mse\n")

    def ls_callback(xk, *args, **kwargs):
        iter_state['idx'] += 1
        res = residuals(xk)
        mse = np.mean(res ** 2)
        msg = f"Iter {iter_state['idx']:04d} | MSE={mse:.6e}"
        print(msg)
        with open(loss_log_path, 'a') as ff:
            ff.write(f"{iter_state['idx']},{mse:.12e}\n")

    # ---------- 调用 least_squares（trf 更稳健且支持 bounds & callback） ----------
    result = least_squares(residuals, x0, bounds=bounds, method='trf',
                           max_nfev=200000, verbose=0, xtol=1e-12, ftol=1e-12,
                           gtol=1e-12, callback=ls_callback)

    if not result.success:
        print("⚠️ least_squares 没有成功收敛：", result.message)

    params_refined = result.x

    # ---------- 后处理（与你原来一致） ----------
    intrinsics = params_refined[:5]
    alpha, beta, gamma, u_c, v_c = intrinsics
    K = np.array([[alpha, gamma, u_c],
                  [0., beta, v_c],
                  [0., 0., 1.]])
    k1, k2, p1, p2, k3 = params_refined[5:10]
    rt_v = params_refined[10:]
    m = int(len(rt_v) / 5)
    v_rot = []
    v_trans = []
    for i in range(m):
        v_rot.append(rt_v[i * 5:i * 5 + 3])
        v_trans.append(rt_v[i * 5 + 3:(i + 1) * 5])
    v_rot = np.array(v_rot)
    v_trans = np.array(v_trans)

    # 计算重投影误差并保存点（保持你原逻辑）
    loss_list = []
    save_dir = r"D:\Code\CISCamera_DALSA\data\CISCamera_Image\txt3"
    os.makedirs(save_dir, exist_ok=True)
    for i in range(len(points_world)):
        world = points_world[i].reshape(-1, 3)
        world[:, 2] = 1
        pixel = points_pixel[i].reshape(-1, 2)
        r_m, _ = cv2.Rodrigues(v_rot[i])
        rt_matri = np.eye(3)
        rt_matri[:2, :2] = r_m[:2, :2]
        rt_matri[:2, 2] = v_trans[i].T
        y_pre = (rt_matri @ world.T).T
        y_dis = distort([k1, k2, p1, p2, k3], y_pre)
        y_pre = (K @ y_dis.T).T
        y_pre = y_pre[:, :2]
        txt_path = os.path.join(save_dir, f"img_{i + 1}_reproj_pts.txt")
        with open(txt_path, "w", encoding="utf-8") as f:
            f.write("# Index\tX\tY\n")
            for pt_idx in range(len(y_pre)):
                x_reproj = y_pre[pt_idx, 0]
                y_reproj = y_pre[pt_idx, 1]
                f.write(f"{pt_idx}\t{x_reproj:.6f}\t{y_reproj:.6f}\n")
        loss = np.linalg.norm(y_pre - pixel, axis=1)
        loss_list.append(np.mean(loss))

    ret = np.mean(loss_list)
    return ret, K, [k1, k2, p1, p2, k3], v_rot, v_trans
# 迭代优化法去畸变
def undistort_points(points, K, D, criteria=None):
    """
    对点集去畸变
    Args:
        points: nx2的图像坐标数组
        K: 相机内参
        D: 畸变系数

    Returns:
        去畸变后的点集
    """
    # 验证输入参数
    if criteria is None:
        criteria = [20, 10e-10]

    assert isinstance(points, np.ndarray), "Input points must be a numpy array"
    assert points.ndim == 2 and points.shape[1] == 2, "Points should have shape (N, 2)"
    assert K.shape == (3, 3), "Camera matrix must be 3x3"
    # Convert camera matrix to numpy array
    A = K.astype(np.float64)
    fx, fy, cx, cy = A[0, 0], A[1, 1], A[0, 2], A[1, 2]
    ifx, ify = 1.0 / fx, 1.0 / fy

    # 畸变系数
    k1,h1,h2,s1,s2=D

    # Apply undistortion
    undistorted_points = []

    for point in points:
        x, y = point[0], point[1]
        u, v = x, y  # Initial values for error calculation
        x = (x - cx) * ifx  # Convert to normalized image coordinates
        y = (y - cy) * ify

        x0, y0 = x, y  # Store initial undistorted coordinates

        error = float('inf')  # Set error to maximum initially
        iteration = 0

        while iteration < criteria[0] and error > criteria[1]:
            r = x * x + y * y
            # Calculate distortion effects
            deltaX = k1 * x * r + h1 * (3 * x ** 2 + y ** 2) + 2 * h2 * x * y + s1 * r
            deltaY = k1 * y * r + 2 * h1 * x * y + h2 * (x ** 2 + 3 * y ** 2) + s2 * r

            x = x0 - deltaX
            y = y0 - deltaY

            # Compute error
            r = x * x + y * y
            # Calculate distortion effects
            deltaX = k1 * x * r + h1 * (3 * x ** 2 + y ** 2) + 2 * h2 * x * y + s1 * r
            deltaY = k1 * y * r + 2 * h1 * x * y + h2 * (x ** 2 + 3 * y ** 2) + s2 * r
            xd0 = x + deltaX
            yd0 = y + deltaY
            # Apply tilt compensation if necessary

            error = np.sqrt((xd0 * fx + cx - u) ** 2 + (yd0 * fy + cy - v) ** 2)
            iteration += 1
            # print(error)

        undistorted_points.append([x*fx+cx, y*fx+cy])

    return np.array(undistorted_points)


def refine_params_with_distortion_basic(
        points_world, points_pixel,
        m, dx, dy, theta, u0, v0,
        coff_dis, v_rot, v_trans
):
    points_pixel = np.array(points_pixel)
    points_world = np.array(points_world)

    # ---------------------- 打包参数（直接使用输入的基础参数作为初始值） ----------------------
    packed_params = []
    # 1. 6个内参基础参数（优化目标，初始值为输入值）
    packed_params.extend([m, dx, dy, theta, u0, v0])
    # 2. 5个畸变系数（优化目标）
    k1, k2, p1, p2, k3 = coff_dis
    packed_params.extend([k1, k2, p1, p2, k3])
    # 3. 外参（每个视角：3旋转向量 + 2平移向量）
    for i in range(len(v_rot)):
        rho_x, rho_y, rho_z = v_rot[i]
        t_x, t_y = v_trans[i]
        packed_params.extend([rho_x, rho_y, rho_z, t_x, t_y])

    # ---------------------- 设置参数边界（根据物理意义约束） ----------------------
    min_bounds = [-np.inf] * len(packed_params)
    max_bounds = [np.inf] * len(packed_params)
    # 内参基础参数边界
    print(m, dx, dy, theta, u0, v0)
    min_bounds[4], max_bounds[4] = u0 - 2, u0 + 2
    min_bounds[5], max_bounds[5] = v0 - 5, v0 + 5
    # min_bounds[6], max_bounds[6] = v_c - 1, v_c + 1
    # min_bounds[7], max_bounds[7] = u_c - 1, u_c + 1
    # min_bounds[8], max_bounds[8] = v_c - 1, v_c + 1
    # min_bounds[9], max_bounds[9] = u_c - 1, u_c + 1
    bounds = (min_bounds, max_bounds)

    # ---------------------- 投影函数（基础参数推导内参K） ----------------------
    def project(x_data, *params):
        # 解包参数
        m_opt, dx_opt, dy_opt, theta_opt, u0_opt, v0_opt = params[:6]  # 优化后的基础参数
        k1_opt, k2_opt, p1_opt, p2_opt, k3_opt = params[6:11]  # 优化后的畸变系数
        v_RT_opt = params[11:]  # 优化后的外参

        # 由基础参数推导内参矩阵K（严格对应公式）
        K = np.eye(3)
        K[0, 0] = m_opt / dx_opt  # α = m/dx
        K[0, 1] = -m_opt * np.tan(theta_opt) / dx_opt  # γ = -m·tanθ/dx
        K[0, 2] = u0_opt  # u₀
        K[1, 1] = 1 / (dy_opt * np.cos(theta_opt))  # β = 1/(dy·cosθ)
        K[1, 2] = v0_opt / m_opt  # v₀/m（公式对应项）
        coff_dis_opt = np.array([k1_opt, k2_opt, p1_opt, p2_opt, k3_opt])

        y_pre_list = []
        for i in range(len(x_data)):
            # 世界坐标（齐次化）
            world = np.array(x_data[i]).reshape(-1, 3)
            world[:, 2] = 1  # 齐次坐标z=1
            # 当前视角外参
            rt = v_RT_opt[i * 5: (i + 1) * 5]  # [rho_x, rho_y, rho_z, t_x, t_y]
            # 旋转向量→旋转矩阵
            rotation_matrix, _ = cv2.Rodrigues(rt[:3])
            # 外参矩阵（3x3）
            rt_matri = np.eye(3)
            rt_matri[:2, :2] = rotation_matrix[:2, :2]  # 旋转部分
            rt_matri[:2, 2] = rt[3:5]  # 平移部分（t_x, t_y）

            # 投影计算：世界坐标→归一化平面→畸变→像素坐标
            y_normalized = (rt_matri @ world.T).T  # 世界坐标→归一化平面（无畸变）
            y_distorted = distort(coff_dis_opt, y_normalized)  # 畸变校正
            y_pixel = (K @ y_distorted.T).T  # 归一化平面→像素坐标
            y_pre_list.append(y_pixel[:, :2])  # 取前两列（u, v）

        return np.array(y_pre_list).reshape(-1)  # 展平为1D数组（匹配curve_fit要求）

    # ---------------------- 执行优化 ----------------------
    popt, pcov = curve_fit(project, points_world, points_pixel.reshape(-1), packed_params, bounds=bounds,
                           maxfev=10000000)

    # ---------------------- 解包优化结果 ----------------------
    # 1. 内参基础参数
    m_refined, dx_refined, dy_refined, theta_refined, u0_refined, v0_refined = popt[:6]
    print(
        f"Refined params: m={m_refined:.6f}, dx={dx_refined:.6f}, dy={dy_refined:.6f}, theta={theta_refined:.6f}, u0={u0_refined:.6f}, v0={v0_refined:.6f}")
    # 2. 由基础参数推导内参矩阵K
    K_refined = np.array([
        [m_refined / dx_refined, -m_refined * np.tan(theta_refined) / dx_refined, u0_refined],
        [0., 1 / (dy_refined * np.cos(theta_refined)), v0_refined / m_refined],
        [0., 0., 1.]
    ])
    print(f"Refined K:\n{K_refined}")
    # 3. 畸变系数
    k1_refined, k2_refined, p1_refined, p2_refined, k3_refined = popt[6:11]
    # 4. 外参
    rt_refined = popt[11:]
    n_views = len(v_rot)
    v_rot_refined = []
    v_trans_refined = []
    for i in range(n_views):
        v_rot_refined.append(rt_refined[i * 5: i * 5 + 3])  # 旋转向量（3个）
        v_trans_refined.append(rt_refined[i * 5 + 3: (i + 1) * 5])  # 平移向量（2个）
    v_rot_refined = np.array(v_rot_refined)
    v_trans_refined = np.array(v_trans_refined)

    # ---------------------- 计算重投影误差并保存结果 ----------------------
    loss_list = []
    save_dir = r"D:\Code\CISCamera_DALSA\data\CISCamera_Image\txt3"
    os.makedirs(save_dir, exist_ok=True)

    for i in range(n_views):
        # 世界坐标和像素坐标
        world_points = points_world[i].reshape(-1, 3)
        world_points[:, 2] = 1  # 齐次化
        pixel_gt = points_pixel[i].reshape(-1, 2)
        # 外参矩阵
        rot_mat, _ = cv2.Rodrigues(v_rot_refined[i])
        rt_matri = np.eye(3)
        rt_matri[:2, :2] = rot_mat[:2, :2]
        rt_matri[:2, 2] = v_trans_refined[i].T
        # 重投影
        y_normalized = (rt_matri @ world_points.T).T
        y_distorted = distort([k1_refined, k2_refined, p1_refined, p2_refined, k3_refined], y_normalized)
        y_reproj = (K_refined @ y_distorted.T).T[:, :2]
        # 保存重投影点
        txt_path = os.path.join(save_dir, f"img_{i + 1}_reproj_pts.txt")
        with open(txt_path, "w", encoding="utf-8") as f:
            f.write("# Index\tX_reproj\tY_reproj\n")
            for idx, (x, y) in enumerate(y_reproj):
                f.write(f"{idx}\t{x:.6f}\t{y:.6f}\n")
        # 计算单视角误差
        loss = np.linalg.norm(y_reproj - pixel_gt, axis=1).mean()
        loss_list.append(loss)

    mean_loss = np.mean(loss_list)  # 平均重投影误差

    # 返回优化结果（包含基础参数和推导的内参矩阵）
    return (
        mean_loss,
        K_refined,  # 推导的内参矩阵
        [k1_refined, k2_refined, p1_refined, p2_refined, k3_refined],  # 畸变系数
        v_rot_refined,  # 旋转向量
        v_trans_refined,  # 平移向量
        # 额外返回优化后的基础参数（方便查看）
        (m_refined, dx_refined, dy_refined, theta_refined, u0_refined, v0_refined)
    )
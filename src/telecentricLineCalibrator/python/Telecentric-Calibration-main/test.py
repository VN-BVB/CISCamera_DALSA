import glob,os
import numpy as np
import cv2
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
def undistort_points(points, K, D, criteria=None):
    """
    对点集进行去畸变（迭代反解模型）

    参数说明:
        points : ndarray (N×2)
            图像像素坐标点集 (u, v)，通常为畸变后的像素坐标。
        K : ndarray (3×3)
            相机内参矩阵 [[fx, 0, cx], [0, fy, cy], [0, 0, 1]]。
        D : list or tuple (k1, h1, h2, s1, s2)
            畸变参数：
                k1 - 径向畸变系数
                h1, h2 - 切向畸变系数
                s1, s2 - 薄棱镜畸变（或非对称畸变）系数
        criteria : list [max_iter, tol]
            迭代停止条件：
                max_iter - 最大迭代次数
                tol - 收敛误差阈值

    返回:
        undistorted_points : ndarray (N×2)
            去畸变后的像素坐标点集 (u', v')
    """

    # ------------------ 参数验证与初始化 ------------------
    if criteria is None:
        criteria = [200, 10e-10]  # 默认最大迭代 20 次，误差阈值 1e-9

    assert isinstance(points, np.ndarray), "输入点集必须是 numpy 数组"
    assert points.ndim == 2 and points.shape[1] == 2, "输入点集应为 (N, 2)"
    assert K.shape == (3, 3), "相机内参矩阵应为 3×3"

    # 提取内参
    A = K.astype(np.float64)
    fx, fy, cx, cy = A[0, 0], A[1, 1], A[0, 2], A[1, 2]
    ifx, ify = 1.0 / fx, 1.0 / fy

    # 提取畸变参数
    k1, h1, h2, s1, s2 = D

    undistorted_points = []  # 保存结果

    # ------------------ 遍历每个点进行迭代反解 ------------------
    for point in points:
        # 原始像素坐标
        u, v = point[0], point[1]

        # 初始归一化坐标 (带畸变的起点)
        x = (u - cx) * ifx
        y = (v - cy) * ify

        # 保存初始值用于固定参考
        x0, y0 = x, y

        # 初始化误差与迭代次数
        error = float('inf')
        iteration = 0

        # ------------------ 迭代去畸变过程 ------------------
        while iteration < criteria[0] and error > criteria[1]:
            # 计算当前半径平方
            r = x * x + y * y

            # 根据畸变模型计算当前畸变量 Δx, Δy
            deltaX = k1 * x * r + h1 * (3 * x ** 2 + y ** 2) + 2 * h2 * x * y + s1 * r
            deltaY = k1 * y * r + 2 * h1 * x * y + h2 * (x ** 2 + 3 * y ** 2) + s2 * r

            # 根据当前估计反解理想归一化坐标
            x = x0 - deltaX
            y = y0 - deltaY

            # ---------- 计算迭代误差 ----------
            r = x * x + y * y
            deltaX = k1 * x * r + h1 * (3 * x ** 2 + y ** 2) + 2 * h2 * x * y + s1 * r
            deltaY = k1 * y * r + 2 * h1 * x * y + h2 * (x ** 2 + 3 * y ** 2) + s2 * r

            xd0 = x + deltaX
            yd0 = y + deltaY

            # 将重新畸变后的坐标转回像素空间，计算与原始像素点的误差
            error = np.sqrt((xd0 * fx + cx - u) ** 2 + (yd0 * fy + cy - v) ** 2)
            iteration += 1
            # print(f"迭代 {iteration} 次, 误差={error}")

        # 迭代结束后，将理想归一化坐标乘回内参得到去畸变像素坐标
        undistorted_points.append([x * fx + cx, y * fy + cy])

    # ------------------ 返回结果 ------------------
    return np.array(undistorted_points)
def undistort_points2(points_px, K, D):
    """使用 OpenCV 去畸变并返回归一化坐标"""
    pts = points_px.reshape(-1, 1, 2)
    undistorted = cv2.undistortPoints(pts, K, D, P=None)
    return undistorted.reshape(-1, 2)
if __name__=="__main__":
    # 假设相机内参和畸变参数
    K = np.array([[47.27, -0.5924188327343539, 15344.18],
                  [0, 46.97, 8060.47],
                  [0, 0, 1]], dtype=float)
    D = np.array([-7.3e-10, -7.0e-07, -3.1e-07, 2.1e-06, -2.0e-06])

    # 构造理想点
    ideal = np.array([[0.1, 0.1, 1.0],
                      [0.05, -0.05, 1.0],
                      [0.0, 0.0, 1.0]])

    # 正向畸变
    distorted = distort(D, ideal)

    # 转换为像素坐标
    points_px = (K @ distorted.T).T[:, :2]

    # 去畸变
    undistorted_px = undistort_points(points_px, K, D)

    print("原始像素:", (K @ ideal.T).T[:, :2])
    print("畸变后:", points_px)
    print("去畸变后:", undistorted_px)

    # 计算内参矩阵的逆矩阵
    K_inv = np.linalg.inv(K)
    print("\n内参矩阵K:\n", K)
    print("内参矩阵的逆K_inv:\n", K_inv)

    # 将去畸变后的像素坐标转换为齐次坐标
    undistorted_homogeneous = to_homogeneous(undistorted_px)
    
    # 左乘内参的逆矩阵，得到归一化坐标
    normalized_after_undistortion = (K_inv @ undistorted_homogeneous.T).T
    
    # 仅保留前两维（x, y）
    normalized_after_undistortion_2d = normalized_after_undistortion[:, :2]
    
    # 原始理想归一化坐标（前三行作为比较）
    original_ideal_2d = ideal[:, :2]
    
    # 显示比较结果
    print("\n===== 去畸变后归一化坐标与原始理想归一化坐标比较 =====")
    print("原始理想归一化坐标 (x, y):")
    print(original_ideal_2d)
    print("\n去畸变后并左乘内参逆矩阵的归一化坐标 (x, y):")
    print(normalized_after_undistortion_2d)
    
    # 计算每个点的误差
    errors = np.sqrt(np.sum((normalized_after_undistortion_2d - original_ideal_2d) ** 2, axis=1))
    print("\n每个点的误差:")
    for i, error in enumerate(errors):
        print(f"点 {i}: {error:.8f}")
    
    # 计算平均误差
    mean_error = np.mean(errors)
    print(f"\n平均误差: {mean_error:.8f}")
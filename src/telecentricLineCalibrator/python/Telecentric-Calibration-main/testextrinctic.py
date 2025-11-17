import os
import time
import json
import numpy as np
from Calibrator.calibrator import Calibrator
from Calibrator import calibrator_helper
from corner_detector import PatternInfo
import cv2
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
def load_calibration_from_cereal_json(path):
    with open(path, "r") as f:
        data = json.load(f)

    # 假设顶层 key 是 "value0"
    data = data["value0"]

    # 内参标量
    m = data["m"]
    dx = data["dx"]
    dy = data["dy"]
    u0 = data["u0"]
    v0 = data["v0"]

    # K 矩阵
    K_dict = data["K"]
    try:
        K_val = K_dict["val"]  # 如果是列表
        K = np.array(K_val).reshape((3, 3))
    except TypeError:
        # 按 value0..value8 读取
        K_val = [K_dict[f"value{i}"] for i in range(9)]
        K = np.array(K_val).reshape((3, 3))

    # 畸变系数
    coff_dis_dict = data["coff_dis"]
    try:
        coff_dis = np.array(coff_dis_dict["val"]).reshape((1, 5))
    except TypeError:
        coff_dis = np.array([coff_dis_dict[f"value{i}"] for i in range(5)]).reshape((1, 5))

    # 外参旋转向量
    v_rot_list = []
    for vec in data["v_rot"]:
        v_rot_list.append([vec["value0"], vec["value1"], vec["value2"]])
    v_rot = np.array(v_rot_list)

    # 外参平移向量
    v_trans_list = []
    for vec in data["v_trans"]:
        v_trans_list.append([vec["value0"], vec["value1"]])
    v_trans = np.array(v_trans_list)

    return m, dx, dy, u0, v0, K, coff_dis, v_rot, v_trans
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
def compute_reproj_loss(points_world, points_pixel, K, coff_dis, v_rot, v_trans):
    # ------------------ 打印输入信息 ------------------
    print("====== [compute_reproj_loss Inputs] ======")

    print("\n-- points_world --")
    for i, pw in enumerate(points_world):
        print(f"[{i}] shape={pw.shape}")
        print(pw)

    print("\n-- points_pixel --")
    for i, pp in enumerate(points_pixel):
        print(f"[{i}] shape={pp.shape}")
        print(pp)

    print("\n-- K (camera matrix) --")
    print(K)

    print("\n-- coff_dis (distortion params) --")
    print(coff_dis)

    print("\n-- v_rot --")
    for i, r in enumerate(v_rot):
        print(f"[{i}] {r}")

    print("\n-- v_trans --")
    for i, t in enumerate(v_trans):
        print(f"[{i}] {t}")

    print("========== Python compute_reproj_loss ==========\n")

    total_err = 0.0
    total_points = 0

    for i in range(len(points_world)):

        world_points = points_world[i].reshape(-1, 3)
        world_points[:, 2] = 1.0
        pixel_gt = points_pixel[i].reshape(-1, 2)

        # ---- Rodrigues 转换 ----
        rvec = np.array(v_rot[i], dtype=np.float64).reshape(3)
        rot_mat, _ = cv2.Rodrigues(rvec)

        R2 = rot_mat[:2, :2]
        t2 = np.array(v_trans[i], dtype=np.float64).reshape(2)

        print(f"\n------ [Frame {i}] ------")
        print(f"Python rvec[{i}]: {rvec}")
        print(f"Python R[{i}]:\n{rot_mat}")
        print(f"Python R2[{i}]:\n{R2}")
        print(f"Python t2[{i}]: {t2}\n")

        N = min(world_points.shape[0], pixel_gt.shape[0])

        for pt_idx in range(N):
            xy = world_points[pt_idx, :2]

            # 仿射：cam_xy
            cam_xy = R2 @ xy + t2

            print(f"[Python] Pt {pt_idx}")
            print(f"  world xy   = {xy}")
            print(f"  cam_xy     = {cam_xy}")

            camPt = cam_xy.reshape(1, 2)
            distortedH = distort(coff_dis, camPt)

            print(f"  distortedH = {distortedH}")

            uvw = K @ distortedH[0].T

            print(f"  uvw        = {uvw}")

            uv_hat = np.array([uvw[0] / uvw[2], uvw[1] / uvw[2]])
            uv = pixel_gt[pt_idx]

            print(f"  uv_hat     = {uv_hat}")
            print(f"  uv (GT)    = {uv}")
            print(f"  err        = {np.linalg.norm(uv_hat - uv)}\n")

            total_err += np.linalg.norm(uv_hat - uv)
            total_points += 1

    print("================================================\n")
    return total_err / total_points

if __name__=="__main__":
    # 用法示例
    m, dx, dy, u0, v0, K, coff_dis, v_rot1, v_trans1 = load_calibration_from_cereal_json("D:/Code/CISCamera_DALSA/data/calibration_config/optimized_calib_data.json")
    m1, dx1, dy1, u01, v01, K1, coff_dis1, v_rot0, v_trans0 = load_calibration_from_cereal_json("D:/Code/CISCamera_DALSA/data/calibration_config/before_optimization_calib_data.json")

    # 打印类型和内容
    print("=== v_rot0 ===")
    print("type:", type(v_rot0))
    print("内容:", v_rot0)
    print("长度:", len(v_rot0))
    print("第一个元素类型:", type(v_rot0[0]))
    print("第一个元素内容:", v_rot0[0])

    print("\n=== v_trans0 ===")
    print("type:", type(v_trans0))
    print("内容:", v_trans0)
    print("长度:", len(v_trans0))
    print("第一个元素类型:", type(v_trans0[0]))
    print("第一个元素内容:", v_trans0[0])
    v_rot = v_rot0[3:4]   # [[2.0879366, 2.08700265, -0.26445935]]
    v_trans = v_trans0[3:4]   # [[-247.38128843, -135.3955088]]
    pattern_info = PatternInfo(0, (8,11), 10, (dx*1000,dy*1000),0)
    # 生成标定板的世界坐标
    w, h = pattern_info.shape
    cp_int = np.zeros((w * h, 3), np.float32)
    cp_int[:, :2] = np.mgrid[0:w, 0:h].T.reshape(-1, 2)
    cp_world = cp_int * pattern_info.distance

    # 标定板图片路径
    txt_dir = "D:/Code/CISCamera_DALSA/data/CISCamera_Image/test"
    if not os.path.exists(txt_dir):
        raise ValueError(f"指定的txt目录不存在: {txt_dir}")

    points_world = []  # the points in world space
    points_pixel = []  # the points in pixel space (relevant to points_world)

    # 修改：从txt文件中读取点数据
    for filename in os.listdir(txt_dir):
        if filename.endswith('.txt'):
            txt_path = os.path.join(txt_dir, filename)
            ret, cp_img2 = read_points_from_txt(txt_path)
            if ret:
                points_world.append(cp_world)
                points_pixel.append(cp_img2)
    if not points_pixel:
        print("没有成功读取任何点数据")
    else:
        print(f"成功读取 {len(points_pixel)} 个点数据")
        # 打印每个点集的第一个点
        print("=== First point of each points_pixel set ===")
        for i, view in enumerate(points_pixel):
            if len(view) > 0:
                print(f"Set {i}: {view[0]}")
            else:
                print(f"Set {i}: empty")
    coff_dis = coff_dis[0]
    theta = 0.0
    print("正在进行非线性优化...")
    s=time.time()
    # # 进行非线性优化
    ret,v_rot_opt, v_trans_opt = calibrator_helper.refine_params_with_distortion_external_only(
        points_world, points_pixel,
        K,coff_dis, v_rot, v_trans)
    # ret, K_opt, coff_dis_opt, v_rot_opt, v_trans_opt = calibrator_helper.refine_params_with_distortion(
    #     points_world, points_pixel, K, coff_dis, v_rot, v_trans
    # )
    # ret, K_opt, v_rot_opt, v_trans_opt = calibrator_helper.refine_params_without_distortion(
    #     points_world, points_pixel, K, v_rot, v_trans
    # )
    if ret:
        print("非线性优化成功，重投影误差:", ret)
    else:
        print("非线性优化失败")
    print("=== Python 打印 v_rot_refined ===")
    print(v_rot_opt)

    print("=== Python 打印 v_trans_refined ===")
    print(v_trans_opt)
    # mean = compute_reproj_loss( points_world, points_pixel,K,coff_dis, v_rot_opt, v_trans_opt)
    # print(f"测试: {mean:.6f}")
    e=time.time()
    print(f"耗费时间：{e-s}s")


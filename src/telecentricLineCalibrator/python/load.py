import os
import json
import numpy as np
import calibrator_helper
from Pattern_Info import PatternInfo


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

if __name__=="__main__":
    # 用法示例
    m, dx, dy, u0, v0, K, coff_dis, v_rot, v_trans = load_calibration_from_cereal_json("D:/Code/CISCamera_DALSA/data/calibration_config/before_optimization_calib_data.json")

    # PatternInfo 示例，假设PatternInfo是你自定义的类
    pattern_info = PatternInfo(0, (8, 11), 10, (dx * 1000 , dy * 1000), 0)

    # 生成标定板的世界坐标
    w, h = pattern_info.shape
    cp_int = np.zeros((w * h, 3), np.float32)
    cp_int[:, :2] = np.mgrid[0:w, 0:h].T.reshape(-1, 2)
    cp_world = cp_int * pattern_info.distance

    # 标定板图片路径
    txt_dir = "D:/Code/CISCamera_DALSA/data/CISCamera_Image/txt"
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
    coff_dis = coff_dis[0]
    theta =0.0
    print("正在进行非线性优化...")
    # # 进行非线性优化
    ret, K_opt, coff_dis_opt, v_rot_opt, v_trans_opt, base_params = calibrator_helper.refine_params_with_distortion_basic(points_world, points_pixel,
                                                                                                              m, dx, dy, theta, u0, v0,
                                                                                                              coff_dis, v_rot, v_trans)
    # ret, K_opt, coff_dis_opt, v_rot_opt, v_trans_opt = calibrator_helper.refine_params_with_distortion(
    #     points_world, points_pixel, K, coff_dis, v_rot, v_trans
    # )
    # ret, K_opt, v_rot_opt, v_trans_opt = calibrator_helper.refine_params_without_distortion(
    #     points_world, points_pixel, K, v_rot, v_trans
    # )
    m = K_opt[0, 0] * dx
    points_pixel = points_pixel
    theta = np.arctan(-K_opt[0, 1] / K_opt[0, 0])
    dy = 1.0 / (K_opt[1, 1] * np.cos(theta))
    u0 = K_opt[0, 2]
    v0 = m * K_opt[1, 2]
    base_params = [m, dx, dy, theta, u0, v0]

    if ret:
        print("非线性优化成功，重投影误差:", ret)
    else:
        print("非线性优化失败")

    # 将优化后的数据保存为新的 JSON 文件
    optimized_data = {
        "value0": {
            "m": base_params[0],
            "dx": base_params[1],
            "dy": base_params[2],
            "u0": base_params[4],
            "v0": base_params[5],
            "K": {
                "rows": 3,
                "cols": 3,
                "val": K_opt.flatten().tolist()
            },
            "coff_dis": {
                "rows": 1,
                "cols": 5,
                "val": coff_dis_opt
            },
            "v_rot": [
                {"value0": rot[0], "value1": rot[1], "value2": rot[2]}
                for rot in v_rot_opt
            ],
            "v_trans": [
                {"value0": trans[0], "value1": trans[1], "value2": 0.0}
                for trans in v_trans_opt
            ]
        }
    }

    optimized_json_file_path = "D:/Code/CISCamera_DALSA/data/calibration_config/optimized_calib_data.json"
    with open(optimized_json_file_path, 'w') as f:
        json.dump(optimized_data, f, indent=4)
    print(f"优化后的角度:{ base_params[3]}")
    print(f"优化后的结果已保存到 {optimized_json_file_path}")

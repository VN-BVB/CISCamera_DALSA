import time
import json
import numpy as np
from Calibrator.calibrator import Calibrator
from corner_detector import PatternInfo

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
    img_dir = "image/chessboard"
    m, dx, dy, u0, v0, K, coff_dis, v_rot, v_trans = load_calibration_from_cereal_json(
        "D:/Code/CISCamera_DALSA/data/calibration_config/before_optimization_calib_data.json")
    pattern_type = PatternInfo(0, (8,11), 10, (dx*1000,dy*1000),0)
    print("Telecentric-Calibration Start!")
    # s=time.time()
    # camera = Calibrator(img_dir,pattern_type,m,visualization=False)
    # camera.calibrate_camera()
    # e=time.time()
    # print(f"耗费时间：{e-s}s")


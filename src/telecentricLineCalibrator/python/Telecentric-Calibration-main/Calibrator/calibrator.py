import cv2
import json
import numpy as np
import os

from numba import none

from corner_detector import PatternInfo
from Calibrator import calibrator_helper


class Calibrator:
    def __init__(self, img_dir, pattern_info:PatternInfo, m, visualization=False):
        """
        :param img_dir: 标定图片文件夹
        :param pattern_info: 标定板信息
        :param m: 远心镜头放大倍数
        :param visualization: 是否可视化
        """
        self.pattern_info = pattern_info
        self.m = m
        self.dx = None
        self.dy = None
        self.theta = None
        self.u0 = None
        self.v0 = None
        self.visualization = visualization
        self.mat_intri = None  # intrinsic matrix
        self.coff_dis = None  # coefficients of distortion
        self.v_rot = None  # 旋转向量
        self.v_trans = None  # 位移向量
        self.json_data = None
        # 控制点的像素坐标
        self.points_pixel = None
        # 生成标定板的世界坐标
        w, h = pattern_info.shape
        # cp_int: corner point in int form, save the coordinate of corner points in world sapce in 'int' form
        # like (0,0,0), (1,0,0), (2,0,0) ...., (10,7,0)
        cp_int = np.zeros((w * h, 3), np.float32)
        cp_int[:, :2] = np.mgrid[0:w, 0:h].T.reshape(-1, 2)
        # cp_world: corner point in world space, save the coordinate of corner points in world space
        self.cp_world = cp_int * pattern_info.distance
        # 标定板图片路径
        # 修改：从 txt 文件目录获取点数据
        self.txt_dir = "D:/Code/CISCamera_DALSA/data/CISCamera_Image/mattxt"
        # 检查目录是否存在
        if not os.path.exists(self.txt_dir):
            raise ValueError(f"指定的txt目录不存在: {self.txt_dir}")
        
    def read_points_from_txt(self, path):
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
                    print(f"file {path} no valid point")
                    return False, None
                
                print(f"读取 {path} 成功，共 {len(points)} 个点")
                return True, np.array(points, dtype=np.float32)
        except Exception as e:
            print(f"无法打开文件 {path}: {str(e)}")
            return False, None

    # 标定相机
    def calibrate_camera(self):
        points_world = []  # the points in world space
        points_pixel = []  # the points in pixel space (relevant to points_world)
        
        # 修改：从txt文件中读取点数据
        for filename in os.listdir(self.txt_dir):
            if filename.endswith('.txt'):
                txt_path = os.path.join(self.txt_dir, filename)
                ret, cp_img2 = self.read_points_from_txt(txt_path)
                if ret:
                    points_world.append(self.cp_world)
                    points_pixel.append(cp_img2)
        
        if not points_pixel:
            print("无有效像素信息")
            return
        
        # 针孔相机标定得到外参
        # 由于我们没有实际图像，使用一个假设的图像尺寸
        # 实际应用中可能需要根据实际情况调整
        image_size = (15344 * 2, 8050 * 2 )  # 假设的图像尺寸
        ret, mat_intri, coff_dis, v_rot, v_trans = cv2.calibrateCamera(points_world, points_pixel, image_size,
                                                                       None, None)
        
        if ret:
            print("针孔模型标定成功，以获取内参，重投影误差为：", ret)
            self.v_rot = np.array(v_rot).reshape(-1, 3)
            # 远心成像缺少t_z
            self.v_trans = np.array(v_rot).reshape(-1, 3)[:, :2]
        else:
            print("针孔模型标定失败")
            return
        
        # 远心成像模型内参初始化
        dx, dy = self.pattern_info.pixel_size
        dx = dx / 1000
        dy = dy / 1000
        u0, v0 = 15344, 8050
        self.mat_intri = np.array([[self.m / dx, 0, u0],
                                   [0, self.m / dy, v0],
                                   [0, 0, 1]])
        
        # 优化不带畸变的远心成像模型参数
        ret, mat_intri, v_rot, v_trans = calibrator_helper.refine_params_without_distortion(points_world, points_pixel,
                                                                                            self.mat_intri, self.v_rot,
                                                                                            self.v_trans)
        if ret:
            print("无畸变优化成功，重投影误差为:", ret)
        else:
            print("无畸变参数优化失败")
        # ========== ✅ 在此处写入无畸变结果 ==========
        # 解析参数

        self.dx = dx
        self.m = mat_intri[0, 0] * self.dx
        self.points_pixel = points_pixel
        self.mat_intri = mat_intri
        self.theta = np.arctan(-mat_intri[0, 1] / mat_intri[0, 0])
        self.dy = 1.0 / (mat_intri[1, 1] * np.cos(self.theta))
        self.u0 = mat_intri[0, 2]
        self.v0 = self.m * mat_intri[1, 2]
        self.coff_dis = [0, 0, 0, 0, 0]  # 占位
        self.v_rot = v_rot
        self.v_trans = v_trans
        base_params = [self.m, self.dx, self.dy, self.theta, self.u0, self.v0]
        # === 构造JSON结构 ===
        optimized_data = {
            "value0": {
                "m": base_params[0],
                "dx": base_params[1],
                "dy": base_params[2],
                "theta": base_params[3],
                "u0": base_params[4],
                "v0": base_params[5],
                "K": {
                    "rows": 3,
                    "cols": 3,
                    "val": mat_intri.flatten().tolist()
                },
                "coff_dis": {
                    "rows": 1,
                    "cols": 5,
                    "val": self.coff_dis
                },
                "v_rot": [
                    {"value0": rot[0], "value1": rot[1], "value2": rot[2]}
                    for rot in v_rot
                ],
                "v_trans": [
                    {"value0": trans[0], "value1": trans[1], "value2": 0.0}
                    for trans in v_trans
                ]
            }
        }

        # === 写入JSON ===
        optimized_json_file_path = (
            "D:/Code/CISCamera_DALSA/data/calibration_config/"
            "optimized_calib_data_no_distortion.json"
        )
        with open(optimized_json_file_path, "w") as f:
            json.dump(optimized_data, f, indent=4)
        print(f"标定成功 (无畸变参数) -> {optimized_json_file_path}")
        # 优化带畸变的远心成像模型参数
        # 初始化畸变系数(k1,k2,p1,p2,k3)
        coff_dis = coff_dis[0]
        ret, mat_intri, coff_dis, v_rot, v_trans = calibrator_helper.refine_params_with_distortion(points_world,
                                                                                                   points_pixel,
                                                                                                   mat_intri, coff_dis,
                                                                                                   v_rot,
                                                                                                   v_trans)
        if ret:
            print("带有畸变参数的标定参数优化成功，重投影误差为:", ret)
        else:
            print("带有畸变参数的标定参数优化失败")

        self.dx = dx
        self.m = mat_intri[0, 0] * self.dx
        self.points_pixel = points_pixel
        self.mat_intri = mat_intri
        self.theta = np.arctan(-mat_intri[0, 1] / mat_intri[0, 0])
        self.dy = 1.0 / (mat_intri[1, 1] * np.cos(self.theta))
        self.u0 = mat_intri[0, 2]
        self.v0 = self.m * mat_intri[1, 2]
        self.coff_dis = coff_dis # 占位
        self.v_rot = v_rot
        self.v_trans = v_trans
        base_params = [self.m, self.dx, self.dy, self.theta, self.u0, self.v0]
        # === 构造JSON结构 ===
        optimized_data_distorted = {
            "value0": {
                "m": base_params[0],
                "dx": base_params[1],
                "dy": base_params[2],
                "theta": base_params[3],
                "u0": base_params[4],
                "v0": base_params[5],
                "K": {
                    "rows": 3,
                    "cols": 3,
                    "val": self.mat_intri.flatten().tolist()
                },
                "coff_dis": {
                    "rows": 1,
                    "cols": 5,
                    "val": self.coff_dis
                },
                "v_rot": [
                    {"value0": rot[0], "value1": rot[1], "value2": rot[2]}
                    for rot in self.v_rot
                ],
                "v_trans": [
                    {"value0": trans[0], "value1": trans[1], "value2": 0.0}
                    for trans in self.v_trans
                ]
            }
        }

        # === 写入JSON ===
        optimized_json_file_path = (
            "D:/Code/CISCamera_DALSA/data/calibration_config/"
            "optimized_calib_data_with_distortion.json"
        )
        with open(optimized_json_file_path, "w") as f:
            json.dump(optimized_data_distorted, f, indent=4)
        print(f"标定成功 (畸变参数) -> {optimized_json_file_path}")

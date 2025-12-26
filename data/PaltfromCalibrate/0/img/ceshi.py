import cv2
import numpy as np
import os

# 获取当前脚本所在目录
base_dir = os.path.dirname(os.path.abspath(__file__))
img_path = os.path.join(base_dir, "04.bmp")

# 读取图片
img = cv2.imread(img_path, cv2.IMREAD_UNCHANGED)

if img is None:
    raise FileNotFoundError(f"无法读取图像: {img_path}")

h, w = img.shape[:2]

# 平移矩阵：向右 200 像素
M = np.float32([
    [1, 0, -2000],
    [0, 1,   0]
])

# 仿射变换
shifted = cv2.warpAffine(img, M, (w + 200, h))

# 保存结果
out_path = os.path.join(base_dir, "06.bmp")
cv2.imwrite(out_path, shifted)

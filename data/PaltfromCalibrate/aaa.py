import cv2
import glob
import os

# =============================
# 配置区（根据需要修改）
# =============================
angle = 60                     # 顺时针旋转角度
output_dir = "rotated_output"  # 保存目录
os.makedirs(output_dir, exist_ok=True)

print("======== 开始处理 BMP 图像 ========")

# 搜索当前目录下所有 BMP
bmp_files = glob.glob("01.bmp")

if not bmp_files:
    print("未找到任何 BMP 文件")
    exit()

for img_name in bmp_files:
    print(f"\n读取图像：{img_name}")

    # 强制灰度（避免3通道导致大文件问题）
    img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)

    if img is None:
        print("❌ 读取失败，跳过")
        continue

    h, w = img.shape
    center = (w // 2, h // 2)

    # 顺时针 -> 使用 -angle
    M = cv2.getRotationMatrix2D(center, -angle, 1.0)
    rotated = cv2.warpAffine(img, M, (w, h))

    # 保存文件
    save_path = os.path.join(output_dir, f"rotated_{img_name}")
    cv2.imwrite(save_path, rotated)

    print(f"✅ 已保存：{save_path}")

print("\n======== 全部处理完成 ========")

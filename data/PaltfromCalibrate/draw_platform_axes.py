"""
在原图上画出对位平台坐标系（旋转中心 + X/Y 轴）
用法: python draw_platform_axes.py [--platform 5] [--image path/to/origin.bmp]
"""

import argparse, json, struct, sys, os
import numpy as np
import cv2

# 覆盖 OpenCV 像素限制
os.environ["OPENCV_IO_MAX_IMAGE_PIXELS"] = "5000000000"

# ── 读 BMP 绕过 OpenCV 限制 ──
def read_large_bmp(filepath):
    with open(filepath, "rb") as f:
        if struct.unpack("<H", f.read(2))[0] != 0x4D42:
            raise ValueError("不是 BMP")
        f.read(8)   # bfSize(4) + bfReserved1(2) + bfReserved2(2)
        off_bits = struct.unpack("<I", f.read(4))[0]
        f.read(4)   # biSize
        w = abs(struct.unpack("<i", f.read(4))[0])
        h = abs(struct.unpack("<i", f.read(4))[0])
        f.read(2)   # biPlanes
        bit_count = struct.unpack("<H", f.read(2))[0]
        compression = struct.unpack("<I", f.read(4))[0]
        if bit_count != 8 or compression != 0:
            raise ValueError(f"只支持 8-bit 未压缩 BMP, 当前 bit={bit_count} type={compression}")
        stride = ((w + 3) // 4) * 4
        f.seek(off_bits, 0)
        raw = f.read(stride * h)
        img = np.frombuffer(raw, dtype=np.uint8).reshape((h, stride))[:, :w]
        return np.flipud(img)

# ── 旋转向量 → 旋转矩阵 ──
def rodrigues(rvec):
    theta = np.linalg.norm(rvec)
    if theta < 1e-12:
        return np.eye(3)
    k = rvec / theta
    K = np.array([[0, -k[2], k[1]], [k[2], 0, -k[0]], [-k[1], k[0], 0]])
    return np.eye(3) + np.sin(theta) * K + (1 - np.cos(theta)) * (K @ K)

# ── 主逻辑 ──
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--platform", type=int, default=5)
    parser.add_argument("--image", type=str, default=None)
    parser.add_argument("--scale", type=float, default=0.05)
    parser.add_argument("--axis_len_mm", type=float, default=50.0, help="坐标轴长度(mm)")
    args = parser.parse_args()

    base = os.path.dirname(os.path.abspath(__file__))
    json_path = os.path.join(os.path.dirname(base), "calibration_config", "platform_pose.json")
    calib_path = os.path.join(os.path.dirname(base), "calibration_config", "optimized_calib_data.json")

    # ── 加载数据 ──
    pose = json.load(open(json_path))["PlatformPoseData"]
    calib = json.load(open(calib_path))["value0"]
    K_mat = calib["K"]
    if isinstance(K_mat, dict):
        K = np.array(K_mat["val"]).reshape(K_mat["rows"], K_mat["cols"])
    else:
        K = np.array(K_mat)
    rvecs = []
    for v in pose["allRotVecs"]:
        if isinstance(v, dict):
            rvecs.append(np.array([v["value0"], v["value1"], v["value2"]]))
        else:
            rvecs.append(np.array(v))
    tvecs = []
    for v in pose["allTransVecs"]:
        if isinstance(v, dict):
            tvecs.append(np.array([v["value0"], v["value1"], v["value2"]]))
        else:
            tvecs.append(np.array(v))

    p = args.platform
    if p >= len(rvecs) or np.allclose(tvecs[p], 0):
        print(f"平台 {p} 无效")
        return

    rvec = rvecs[p]        # 平台→相机 Rodrigues
    tvec = tvecs[p]        # 平台原点(旋转中心) 在相机坐标 (mm)

    R = rodrigues(rvec)    # 3×3 平台→相机
    R2 = R[:2, :2]         # 2×2 平台 XY → 相机 XY
    tx, ty = tvec[0], tvec[1]

    # 相机 → 像素 (telecentric: K(1,0)=0)
    def cam_to_pixel(xc, yc):
        u = K[0,0]*xc + K[0,1]*yc + K[0,2]
        v = K[1,1]*yc + K[1,2]
        return (int(u), int(v))

    center_px = cam_to_pixel(tx, ty)

    # X/Y 轴方向在相机坐标 (R2 的两列)
    x_dir_cam = R2[:, 0]   # (R2(0,0), R2(1,0))
    y_dir_cam = R2[:, 1]   # (R2(0,1), R2(1,1))

    L = args.axis_len_mm
    x_tip_px = cam_to_pixel(tx + L*x_dir_cam[0], ty + L*x_dir_cam[1])
    y_tip_px = cam_to_pixel(tx + L*y_dir_cam[0], ty + L*y_dir_cam[1])

    # ── 读图 ──
    if args.image:
        img_path = os.path.abspath(args.image)
    else:
        origin_dir = os.path.join(base, str(p), "origin")
        files = [f for f in os.listdir(origin_dir) if f.lower().endswith(('.bmp','.png','.jpg'))]
        if not files:
            raise RuntimeError(f"origin 目录无图片: {origin_dir}")
        img_path = os.path.join(origin_dir, files[0])

    print(f"读图: {img_path}")
    if img_path.lower().endswith(".bmp"):
        img = read_large_bmp(img_path)
    else:
        img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise RuntimeError(f"无法读取 {img_path}")

    # 降采样
    small = cv2.resize(img, None, fx=args.scale, fy=args.scale, interpolation=cv2.INTER_AREA)
    if len(small.shape) == 2:
        small = cv2.cvtColor(small, cv2.COLOR_GRAY2BGR)

    s = args.scale
    center_s = (int(center_px[0]*s), int(center_px[1]*s))
    x_tip_s  = (int(x_tip_px[0]*s), int(x_tip_px[1]*s))
    y_tip_s  = (int(y_tip_px[0]*s), int(y_tip_px[1]*s))

    # ── 画轴 ──
    # 旋转中心: 实心圆点
    cv2.circle(small, center_s, 6, (255, 255, 255), -1, cv2.LINE_AA)
    cv2.circle(small, center_s, 8, (0, 0, 0), 2, cv2.LINE_AA)

    # X 轴 (红)
    cv2.arrowedLine(small, center_s, x_tip_s, (50, 50, 255), 3, cv2.LINE_AA, tipLength=0.12)
    # Y 轴 (绿)
    cv2.arrowedLine(small, center_s, y_tip_s, (50, 200, 50), 3, cv2.LINE_AA, tipLength=0.12)

    # X/Y 标签 — 放在箭头外侧
    def put_label(img, text, pos, color):
        (tw, th), _ = cv2.getTextSize(text, cv2.FONT_HERSHEY_DUPLEX, 0.9, 2)
        cv2.putText(img, text, (pos[0] - tw//2, pos[1] + th//2),
                    cv2.FONT_HERSHEY_DUPLEX, 0.9, color, 2, cv2.LINE_AA)

    put_label(small, "X", x_tip_s, (50, 50, 255))
    put_label(small, "Y", y_tip_s, (50, 200, 50))

    # 平台号 — 原点旁边
    cv2.putText(small, f"P{p}", (center_s[0]+12, center_s[1]-10),
                cv2.FONT_HERSHEY_DUPLEX, 0.7, (255, 255, 255), 2, cv2.LINE_AA)

    # 底部信息栏
    info = f"  tx={tx:.1f}  ty={ty:.1f} mm  |  theta={np.arctan2(R2[1,0],R2[0,0])*180/np.pi:.1f}deg  |  pixel ({center_px[0]:.0f}, {center_px[1]:.0f})"
    h_bar = 30
    cv2.rectangle(small, (0, small.shape[0]-h_bar), (small.shape[1], small.shape[0]), (0,0,0), -1)
    cv2.putText(small, info, (8, small.shape[0]-8),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1, cv2.LINE_AA)

    out = os.path.join(base, f"platform{p}_axes.png")
    cv2.imwrite(out, small)
    print(f"已保存: {out}")
    cv2.namedWindow("platform_axes", cv2.WINDOW_NORMAL)
    cv2.imshow("platform_axes", small)
    cv2.waitKey(0)

if __name__ == "__main__":
    main()

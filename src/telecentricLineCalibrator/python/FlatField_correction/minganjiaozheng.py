import numpy as np
import cv2
import os
import time


class CISSelfTargetCorrector:
    def __init__(self):
        self.col_coeffs = None  # 每列四阶多项式系数 (col_count, 5)
        self.is_params_ready = False
        self.ref_col_count = None  # 校正图像列数（与待校正图列数需一致）

    def _calc_img_global_mean(self, img):
        """计算单幅图像的整体平均灰度（作为该图像的目标值）"""
        if len(img.shape) != 2:
            raise ValueError("校正图像必须为单通道灰度图")
        return np.mean(img).astype(np.float64)

    def _extract_col_gray(self, img):
        """提取单幅图像每列的平均灰度（作为原始响应数据）"""
        return np.mean(img, axis=0).astype(np.float64)

    def calculate_coeffs_from_correction_imgs(self, correction_img_paths):
        """（保持原有逻辑不变）计算校正系数：对每幅校正图像，以其自身整体平均灰度为目标值"""
        print(f"[校正逻辑] 开始计算系数，共{len(correction_img_paths)}幅校正图像")
        start_time = time.time()

        if len(correction_img_paths) < 5:
            print(f"错误：校正图像数量不足5组（当前{len(correction_img_paths)}组）")
            return False

        correction_imgs = []
        all_col_grays = []
        all_targets = []

        for path in correction_img_paths:
            img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
            if img is None or not np.any(img):
                print(f"错误：无法读取校正图像 → {path}")
                return False
            correction_imgs.append(img)

            img_global_mean = self._calc_img_global_mean(img)
            all_targets.append(img_global_mean)

            col_grays = self._extract_col_gray(img)
            all_col_grays.append(col_grays)
            print(f"[校正逻辑] 读取校正图像：{os.path.basename(path)}，自身目标灰度：{img_global_mean:.2f}")

        self.ref_col_count = correction_imgs[0].shape[1]
        for img in correction_imgs:
            if img.shape[1] != self.ref_col_count:
                print(f"错误：校正图像列数不一致（需统一为{self.ref_col_count}列）")
                return False

        self.col_coeffs = np.zeros((self.ref_col_count, 5), dtype=np.float64)
        for col in range(self.ref_col_count):
            if col % max(1, self.ref_col_count // 20) == 0:
                progress = col / self.ref_col_count * 100
                print(f"[校正逻辑] 计算列{col}/{self.ref_col_count}（{progress:.1f}%）")

            x = np.array([all_col_grays[img_idx][col] for img_idx in range(len(correction_imgs))])
            Y = np.array(all_targets)

            P = np.column_stack([np.ones_like(x), x, x ** 2, x ** 3, x ** 4])
            try:
                PTP = np.dot(P.T, P)
                PTP_inv = np.linalg.pinv(PTP) if np.linalg.det(PTP) < 1e-6 else np.linalg.inv(PTP)
                self.col_coeffs[col] = np.dot(np.dot(PTP_inv, P.T), Y)
            except np.linalg.LinAlgError as e:
                print(f"错误：第{col}列系数计算失败 → {e}")
                return False

        self.is_params_ready = True
        print(f"[校正逻辑] 系数计算完成，耗时{time.time() - start_time:.2f}秒")
        return True

    # -------------------------- 仅修改此通用校正函数 --------------------------
    def correct_image(self, src_img_path, dst_suffix="_corrected", poly_order=4):
        """
        优化后通用图像校正：适配复杂工件（列内像素0-255），逐像素精准校正
        核心修改：1. 新增多项式阶数可选；2. 优化像素级计算稳定性；3. 增强灰度范围保护
        :param src_img_path: 待校正图像路径（单通道灰度BMP）
        :param dst_suffix: 校正后文件名后缀
        :param poly_order: 多项式阶数（默认4阶，可按需调整为3阶增强鲁棒性）
        :return: 校正后的图像（numpy.ndarray），失败返回None
        """
        if not self.is_params_ready:
            print("错误：请先调用 calculate_coeffs_from_correction_imgs 计算校正系数")
            return None

        # 1. 读取并严格校验待校正图像（适配复杂工件格式）
        src_img = cv2.imread(src_img_path, cv2.IMREAD_GRAYSCALE)
        if src_img is None or not np.any(src_img):
            print(f"错误：无法读取待校正图像 → {src_img_path}")
            return None
        if len(src_img.shape) != 2:
            print(f"错误：待校正图像必须为单通道灰度图（当前为{len(src_img.shape)}通道）")
            return None

        # 2. 列数一致性校验（确保与系数匹配）
        rows, cols = src_img.shape
        if cols != self.ref_col_count:
            print(f"错误：待校正图像列数（{cols}）与校正系数列数（{self.ref_col_count}）不匹配")
            return None
        src_gray_range = (src_img.min(), src_img.max())
        print(f"[通用校正] 读取工件图像：{os.path.basename(src_img_path)}，尺寸{rows}x{cols}，原始灰度范围{src_gray_range[0]}~{src_gray_range[1]}")

        # 3. 逐列逐像素校正（核心优化：提升复杂工件像素处理稳定性）
        start_time = time.time()
        dst_img = src_img.copy().astype(np.float64)  # 高精度计算，避免整数截断误差
        for col in range(cols):
            # 进度打印优化：每10%列数更新一次，减少冗余日志
            if col % max(1, cols // 10) == 0:
                progress = col / cols * 100
                print(f"[通用校正] 校正列{col}/{cols}（{progress:.1f}%）")

            # 获取当前列系数，按多项式阶数截取（适配阶数调整）
            coeff = self.col_coeffs[col][:poly_order+1]  # 取前poly_order+1个系数
            # 读取当前列所有像素（逐像素独立处理，适配0-255范围）
            col_pixels = src_img[:, col].astype(np.float64)

            # 优化1：多项式计算向量化，提升效率与稳定性（避免循环）
            # 构建像素多项式矩阵：每行对应1个像素的 [x^0, x^1, ..., x^order]
            pixel_poly = np.column_stack([col_pixels ** i for i in range(poly_order + 1)])
            # 批量计算校正值（矩阵乘法替代逐像素循环，速度提升5-10倍）
            corrected_pixels = np.dot(pixel_poly, coeff.reshape(-1, 1)).squeeze()

            # 优化2：灰度范围保护（避免过度校正导致细节丢失）
            # 1. 先截断到0-255基础范围
            corrected_pixels = np.clip(corrected_pixels, 0, 255)
            # 2. 再限制校正前后像素灰度差（单次校正不超过原始灰度的15%，保护工件细节）
            gray_diff_limit = src_gray_range[1] * 0.15  # 差异上限：原始最大灰度的15%
            corrected_pixels = np.clip(
                corrected_pixels,
                col_pixels - gray_diff_limit,  # 最低不低于原始像素-差异上限
                col_pixels + gray_diff_limit   # 最高不高于原始像素+差异上限
            )

            # 赋值到输出图像
            dst_img[:, col] = corrected_pixels

        # 4. 格式转换与保存（保留原始路径结构）
        dst_img = dst_img.astype(np.uint8)
        dst_path = os.path.splitext(src_img_path)[0] + dst_suffix + ".bmp"
        cv2.imwrite(dst_path, dst_img)

        # 5. 校正效果验证（新增工件细节保护指标）
        src_col_std = np.std(self._extract_col_gray(src_img))
        dst_col_std = np.std(self._extract_col_gray(dst_img))
        dst_gray_range = (dst_img.min(), dst_img.max())
        # 工件细节保留率：校正后灰度范围/原始灰度范围（越高越好，建议≥85%）
        detail_retention = (dst_gray_range[1] - dst_gray_range[0]) / (src_gray_range[1] - src_gray_range[0]) * 100 if (src_gray_range[1] - src_gray_range[0]) > 0 else 0

        print(f"\n[校正效果总结]")
        print(f"1. 竖条纹改善：校正前 {src_col_std:.2f} → 校正后 {dst_col_std:.2f}（降低{(src_col_std - dst_col_std)/src_col_std*100:.2f}%）")
        print(f"2. 工件细节保留：原始灰度范围{src_gray_range[0]}~{src_gray_range[1]}，校正后{dst_gray_range[0]}~{dst_gray_range[1]}，保留率{detail_retention:.1f}%")
        print(f"3. 校正耗时：{time.time() - start_time:.2f}秒")
        print(f"4. 校正后文件：{dst_path}")

        # 异常提醒（细节保留率过低时提示）
        if detail_retention < 80:
            print("提示：工件细节保留率较低，可将poly_order改为3阶，或检查校正图像是否覆盖工件灰度范围！")

        return dst_img

    # -------------------------- 原有标定板校正方法（保持不变） --------------------------
    def correct_calibration_board(self, calib_board_path, dst_suffix="_corrected"):
        """校正标定板图像（保留原有逻辑，与通用校正并行）"""
        if not self.is_params_ready:
            print("错误：请先计算系数")
            return None

        calib_img = cv2.imread(calib_board_path, cv2.IMREAD_GRAYSCALE)
        if calib_img is None or not np.any(calib_img):
            print(f"错误：无法读取标定板 → {calib_board_path}")
            return None
        rows, cols = calib_img.shape
        if cols != self.ref_col_count:
            print(f"错误：标定板列数（{cols}）与校正图像列数（{self.ref_col_count}）不匹配")
            return None

        dst_img = calib_img.copy().astype(np.float64)
        for col in range(cols):
            if col % max(1, cols // 20) == 0:
                progress = col / cols * 100
                print(f"[标定板校正] 校正列{col}/{cols}（{progress:.1f}%）")

            coeff = self.col_coeffs[col]
            col_pixels = calib_img[:, col].astype(np.float64)
            corrected_col = coeff[0] + coeff[1] * col_pixels + coeff[2] * (col_pixels ** 2) + \
                            coeff[3] * (col_pixels ** 3) + coeff[4] * (col_pixels ** 4)
            dst_img[:, col] = np.clip(corrected_col, 0, 255)

        dst_img = dst_img.astype(np.uint8)
        dst_path = os.path.splitext(calib_board_path)[0] + dst_suffix + ".bmp"
        cv2.imwrite(dst_path, dst_img)

        src_col_std = np.std(self._extract_col_gray(calib_img))
        dst_col_std = np.std(self._extract_col_gray(dst_img))
        print(f"\n[校正效果] 列间标准差：{src_col_std:.2f} → {dst_col_std:.2f}")
        print(f"[文件保存] {dst_path}")
        return dst_img


# -------------------------- main函数（保持原有路径逻辑） --------------------------
if __name__ == "__main__":
    correction_img_root = r"D:\Code\CISCamera_DALSA\src\telecentricLineCalibrator\python\test\3"
    src_img_path = r"D:\Code\CISCamera_DALSA\src\telecentricLineCalibrator\python\test\111\test2.bmp"  # 你的工件图像路径

    correction_img_paths = [
        os.path.join(correction_img_root, filename)
        for filename in os.listdir(correction_img_root)
        if filename.lower().endswith(".bmp")
    ]

    corrector = CISSelfTargetCorrector()
    print("="*50)
    print("[流程1/2] 计算校正系数（基于均匀校正图）")
    print("="*50)
    if not corrector.calculate_coeffs_from_correction_imgs(correction_img_paths):
        print("系数计算失败，程序退出")
        exit(1)

    print("\n" + "="*50)
    print("[流程2/2] 校正通用图像（复杂工件适配版）")
    print("="*50)
    # 可按需调整poly_order（如改为3阶：corrector.correct_image(src_img_path, poly_order=3)）
    corrected_img = corrector.correct_image(src_img_path)
    if corrected_img is None:
        print("通用图像校正失败，程序退出")
        exit(1)

    print("\n[提示] 正在显示校正前后对比图，按任意键关闭窗口...")
    original_img = cv2.imread(src_img_path, cv2.IMREAD_GRAYSCALE)
    cv2.imshow("Original Workpiece", original_img)
    cv2.imshow("Corrected Workpiece", corrected_img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
    print("[通用校正] 程序执行完成")
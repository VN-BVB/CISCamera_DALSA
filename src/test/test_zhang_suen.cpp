// test_zhang_suen.cpp
// Zhang-Suen 骨架化参考实现（纯 OpenCV core+imgproc）
// 用途：作为 canny_zernike_detector.cpp 中 calculateCenterLineWithZhangSuen 的基准对照
// 注意：项目 opencv_world440 未链接 ximgproc，因此无法直接调用 cv::ximgproc::thinning
//       此处实现的是教科书版 Zhang-Suen，使用行指针 + 256 项 LUT 优化

#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

namespace {

constexpr int kMaxIterations = 1000;

// 预计算 8 邻域查找表：
// 将 8 邻域的二进制编码 (P2 P3 P4 P5 P6 P7 P8 P9) 映射为是否可删除
// idx 的位序：bit0=P2, bit1=P3, ..., bit7=P9 (顺时针，从正上开始)
struct ThinningTable {
    bool sub1[256];  // 子迭代 1 删除条件
    bool sub2[256];  // 子迭代 2 删除条件

    ThinningTable() {
        for (int n = 0; n < 256; ++n) {
            int p[8];
            for (int k = 0; k < 8; ++k) p[k] = (n >> k) & 1;

            // B(P1): 8 邻域中前景像素数
            int B = p[0] + p[1] + p[2] + p[3] + p[4] + p[5] + p[6] + p[7];

            // A(P1): 顺时针序列 P2->P3->...->P9->P2 中 0->1 的次数
            int A = 0;
            for (int k = 0; k < 8; ++k) {
                if (p[k] == 0 && p[(k + 1) % 8] == 1) ++A;
            }

            bool ok = (B >= 2 && B <= 6 && A == 1);
            // 子迭代 1: P2*P4*P6=0 且 P4*P6*P8=0
            // 在我们的位序中 P2=p0, P4=p2, P6=p4, P8=p6
            sub1[n] = ok && (p[0] & p[2] & p[4]) == 0 && (p[2] & p[4] & p[6]) == 0;
            // 子迭代 2: P2*P4*P8=0 且 P2*P6*P8=0
            sub2[n] = ok && (p[0] & p[2] & p[6]) == 0 && (p[0] & p[4] & p[6]) == 0;
        }
    }
};

const ThinningTable& getTable() {
    static ThinningTable t;
    return t;
}

// 在二值图（0/255 CV_8UC1）上做一次子迭代
// table 选择 sub1 或 sub2
// 返回本次删除的像素数
int thinningSubIteration(cv::Mat& binary, const bool* table) {
    const int rows = binary.rows;
    const int cols = binary.cols;

    // 收集待删除像素坐标，避免在迭代中修改影响判定
    std::vector<cv::Point> toRemove;
    toRemove.reserve(8192);

    for (int y = 1; y < rows - 1; ++y) {
        const uchar* rowUp   = binary.ptr<uchar>(y - 1);
        const uchar* rowMid  = binary.ptr<uchar>(y);
        const uchar* rowDown = binary.ptr<uchar>(y + 1);
        for (int x = 1; x < cols - 1; ++x) {
            if (rowMid[x] == 0) continue;  // 仅处理前景像素（这里前景=0）

            // 按位序编码 8 邻域
            // bit0=P2(上), bit1=P3(右上), bit2=P4(右), bit3=P5(右下),
            // bit4=P6(下), bit5=P7(左下), bit6=P8(左), bit7=P9(左上)
            int idx = 0;
            if (rowUp[x])     idx |= 1;       // P2
            if (rowUp[x+1])   idx |= 2;       // P3
            if (rowMid[x+1])  idx |= 4;       // P4
            if (rowDown[x+1]) idx |= 8;       // P5
            if (rowDown[x])   idx |= 16;      // P6
            if (rowDown[x-1]) idx |= 32;      // P7
            if (rowMid[x-1])  idx |= 64;      // P8
            if (rowUp[x-1])   idx |= 128;     // P9

            if (table[idx]) toRemove.emplace_back(x, y);
        }
    }

    for (const auto& pt : toRemove) {
        binary.at<uchar>(pt) = 0;
    }
    return static_cast<int>(toRemove.size());
}

// 完整 Zhang-Suen 骨架化（in-place，前景像素需为 0，背景为 255）
void zhangSuenThinning(cv::Mat& binary_fg_zero) {
    const ThinningTable& t = getTable();
    int iter = 0;
    while (iter < kMaxIterations) {
        int removed = 0;
        removed += thinningSubIteration(binary_fg_zero, t.sub1);
        removed += thinningSubIteration(binary_fg_zero, t.sub2);
        if (removed == 0) break;
        ++iter;
    }
    std::cout << "[Zhang-Suen] total iterations: " << iter << std::endl;
}

}  // namespace

// ====== 测试入口 ======
// 修改 kImagePath 为你的实际图像路径
void testZhangSuenThinning() {
    // 硬编码图像路径（参考项目内既有路径风格）
    const std::string kImagePath ="E:/work/Car_door_ring_splicing/image/背面打光/260716/croped/863_14455.bmp";
    const std::string kOutSkeleton = "zhang_suen_skeleton.bmp";
    const std::string kOutOverlay  = "zhang_suen_overlay.bmp";

    // 1. 读取图像
    cv::Mat raw = cv::imread(kImagePath, cv::IMREAD_COLOR);
    if (raw.empty()) {
        std::cerr << "[ERROR] 无法读取图像: " << kImagePath << std::endl;
        return;
    }
    std::cout << "[INFO] 图像尺寸: " << raw.cols << " x " << raw.rows << std::endl;

    // 2. 灰度化 + 二值化（与项目里 calculateCenterLineWithZhangSuen 保持一致）
    cv::Mat gray, binary;
    cv::cvtColor(raw, gray, cv::COLOR_BGR2GRAY);

    auto t0 = std::chrono::steady_clock::now();

    // Otsu 阈值得到 0/255 二值图，再做反相，使前景=0、背景=255
    cv::threshold(gray, binary, 0, 255, cv::THRESH_OTSU);

    // 3. 形态学开运算去毛刺（细化前推荐预处理）
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

    auto t1 = std::chrono::steady_clock::now();

    // 4. Zhang-Suen 骨架化
    zhangSuenThinning(binary);   // in-place，前景=0

    auto t2 = std::chrono::steady_clock::now();

    // 5. 统计骨架像素数
    int skeletonPixels = cv::countNonZero(255 - binary);

    // 6. 输出可视化
    cv::imwrite(kOutSkeleton, binary);

    cv::Mat overlay;
    cv::cvtColor(raw, overlay, cv::COLOR_BGR2GRAY);
    cv::cvtColor(overlay, overlay, cv::COLOR_GRAY2BGR);
    // 骨架用红色叠加显示
    cv::Mat skeletonColor = cv::Mat::zeros(binary.size(), CV_8UC3);
    skeletonColor.setTo(cv::Scalar(0, 0, 255), binary == 0);
    cv::addWeighted(overlay, 0.5, skeletonColor, 1.0, 0, overlay);
    cv::imwrite(kOutOverlay, overlay);

    // 7. 时间统计
    double msPre = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double msThin = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double msTotal = std::chrono::duration<double, std::milli>(t2 - t0).count();

    std::cout << "================ Zhang-Suen 骨架化报告 ================" << std::endl;
    std::cout << "图像路径        : " << kImagePath << std::endl;
    std::cout << "预处理耗时      : " << msPre  << " ms" << std::endl;
    std::cout << "细化耗时        : " << msThin << " ms" << std::endl;
    std::cout << "总耗时          : " << msTotal << " ms" << std::endl;
    std::cout << "骨架像素数      : " << skeletonPixels << std::endl;
    std::cout << "骨架图保存至    : " << kOutSkeleton << std::endl;
    std::cout << "叠加可视化保存至: " << kOutOverlay << std::endl;
    std::cout << "======================================================" << std::endl;
}

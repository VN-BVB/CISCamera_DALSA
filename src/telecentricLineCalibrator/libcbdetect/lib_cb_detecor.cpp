#include "lib_cb_detecor.h"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <chrono>
#include <ctime>
#include <iostream>

int LibCBDetector::file_counter_ = 0;  // 初始化静态变量

LibCBDetector::LibCBDetector() {
    // 初始化函数可以留空
}

std::vector<std::vector<cv::Point2d>> LibCBDetector::detect(const cv::Mat& img, cbdetect::CornerType corner_type) {
    cbdetect::Corner corners;
    std::vector<cbdetect::Board> boards;
    cbdetect::Params params;
    std::vector<std::vector<cv::Point2d>> board_points_sorted;
    params.corner_type = corner_type;

    auto t1 = std::chrono::high_resolution_clock::now();
    cbdetect::find_corners(img, corners, params);
    auto t2 = std::chrono::high_resolution_clock::now();

    auto t3 = std::chrono::high_resolution_clock::now();
    cbdetect::boards_from_corners(img, corners, boards, params);
    auto t4 = std::chrono::high_resolution_clock::now();

    std::cout << "Find corners took: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0 << " ms\n";
    std::cout << "Find boards took: " << std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / 1000.0 << " ms\n";
    std::cout << "Total took: "
              << (std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0 +
                  std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / 1000.0)
              << " ms\n";

    cbdetect::plot_board_points(img, corners, boards, board_points_sorted);
    return board_points_sorted;
}

std::vector<std::vector<cv::Point2d>> LibCBDetector::processSingleImage(const cv::Mat& img, int file_index) {
    int coarse_count_total = 0;
    int refine_count_total = 0;
    std::vector<std::vector<cv::Point2d>> allRefinedBoards;  // 最终返回值：原图坐标系

    // 1. 降采样
    cv::Mat downsampled_img;
    double downsample_scale = 300.0 / 1200.0;
    cv::resize(img, downsampled_img, cv::Size(), downsample_scale, downsample_scale, cv::INTER_LINEAR);

    // 2. 粗定位
    std::vector<std::vector<cv::Point2d>> coarse_boards = detect(downsampled_img, cbdetect::SaddlePoint);
    coarse_count_total += coarse_boards.size();
    PLOGD << "粗定位检测到棋盘格数量 = " << coarse_boards.size();

    if (coarse_boards.empty()) {
        std::cerr << "[警告] 未检测到棋盘格，跳过..." << std::endl;
        return allRefinedBoards;
    }

    // 3. 对每个粗定位进行精定位
    for (const auto& coarse_points : coarse_boards) {
        // --- 恢复到原图尺寸 ---
        std::vector<cv::Point2d> coarse_pts_original;
        coarse_pts_original.reserve(coarse_points.size());
        for (auto& p : coarse_points) coarse_pts_original.emplace_back(p * (1.0 / downsample_scale));

        // --- 外接矩形 ---
        std::vector<cv::Point2f> coarse_pts_f;
        for (auto& p : coarse_pts_original) coarse_pts_f.emplace_back(p.x, p.y);
        cv::Rect roi = cv::boundingRect(coarse_pts_f);
        int expand_w = roi.width * 0.2;
        int expand_h = roi.height * 0.2;
        roi.x = std::max(roi.x - expand_w, 0);
        roi.y = std::max(roi.y - expand_h, 0);
        roi.width = std::min(roi.width + 2 * expand_w, img.cols - roi.x);
        roi.height = std::min(roi.height + 2 * expand_h, img.rows - roi.y);

        cv::Mat cropped = img(roi);

        // --- 精定位 ---
        cv::Mat gray = cropped;
        if (cropped.channels() == 3) cv::cvtColor(cropped, gray, cv::COLOR_BGR2GRAY);
        if (cropped.channels() == 4) cv::cvtColor(cropped, gray, cv::COLOR_BGRA2GRAY);

        auto refined_boards = detect(gray, cbdetect::SaddlePoint);
        refine_count_total += refined_boards.size();
        PLOGD << "精定位检测到棋盘格 = " << refined_boards.size();

        // --- 转换为原图坐标并加入返回值 ---
        for (auto& rb : refined_boards) {
            std::vector<cv::Point2d> refined_original;
            refined_original.reserve(rb.size());
            for (auto& p : rb) refined_original.emplace_back(p.x + roi.x, p.y + roi.y);

            allRefinedBoards.push_back(refined_original);  // 保存到返回值
        }
    }

    PLOGD << "最终统计：图像 " << file_index << " 粗定位总数 = " << coarse_count_total << ", 精定位总数 = " << refine_count_total;

    return allRefinedBoards;
}

// 手动读取 8-bit 灰度 BMP，绕过 OpenCV 的 CV_IO_MAX_IMAGE_PIXELS 限制
static cv::Mat readLargeBMP(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return cv::Mat();

    // BITMAPFILEHEADER (14 bytes)
    uint16_t bfType;
    uint32_t bfOffBits;
    file.read(reinterpret_cast<char*>(&bfType), 2);
    if (bfType != 0x4D42) return cv::Mat();  // 不是 'BM'
    file.seekg(8, std::ios::cur);            // 跳过 fileSize(4) + reserved(4)
    file.read(reinterpret_cast<char*>(&bfOffBits), 4);

    // BITMAPINFOHEADER (40 bytes)
    uint32_t biWidth, biHeight;
    uint16_t biBitCount;
    uint32_t biCompression;
    file.seekg(4, std::ios::cur);  // 跳过 headerSize(4)
    file.read(reinterpret_cast<char*>(&biWidth), 4);
    file.read(reinterpret_cast<char*>(&biHeight), 4);
    file.seekg(2, std::ios::cur);  // 跳过 planes(2)
    file.read(reinterpret_cast<char*>(&biBitCount), 2);
    file.read(reinterpret_cast<char*>(&biCompression), 4);

    // 只支持 8-bit 未压缩灰度图
    if (biBitCount != 8 || biCompression != 0) return cv::Mat();

    // BMP 每行补齐到 4 字节边界
    int stride = ((biWidth + 3) / 4) * 4;
    std::vector<uint8_t> rawData(stride * biHeight);
    file.seekg(bfOffBits, std::ios::beg);
    file.read(reinterpret_cast<char*>(rawData.data()), stride * biHeight);

    // BMP 是 bottom-up 存储，需要翻转
    cv::Mat img(biHeight, biWidth, CV_8UC1);
    for (uint32_t r = 0; r < biHeight; r++) {
        memcpy(img.ptr(r), rawData.data() + (biHeight - 1 - r) * stride, biWidth);
    }
    return img;
}

void LibCBDetector::processImagesInDirectoryFilePath(const std::string& dir_path,
                                                     std::vector<std::vector<std::vector<cv::Point2d>>>& allImagesBoardsPts) {
    file_counter_ = 0;
    allImagesBoardsPts.clear();

    parent_path_ = std::filesystem::path(dir_path).parent_path().string();
    std::cout << "parent" << parent_path_ << std::endl;
    QDir dir(QString::fromStdString(dir_path));
    QFileInfoList files = dir.entryInfoList(QDir::Files);

    PLOGD << "正在检测棋盘格 ";

    for (const QFileInfo& fileInfo : files) {
        std::string file_path = fileInfo.absoluteFilePath().toStdString();
        std::string file_name = fileInfo.fileName().toStdString();

        // 生成对应 txt 名
        std::filesystem::path p(file_name);
        std::string txtFile = parent_path_ + "/txt/" + p.stem().string() + ".txt";

        std::vector<std::vector<cv::Point2d>> boardsPtsXY;

        // ==========================
        //  1. 如果 txt 已存在 → 直接读取（Eigen -> cv 转换）
        // ==========================
        if (std::filesystem::exists(txtFile)) {
            std::vector<Eigen::Vector2d> ptsEigen;
            if (readPointsFromTxt(txtFile, ptsEigen)) {
                std::vector<cv::Point2d> ptsCV;
                ptsCV.reserve(ptsEigen.size());
                for (const auto& pe : ptsEigen) ptsCV.emplace_back(pe.x(), pe.y());

                // 将读取到的第一个棋盘格（来自 txt）作为该图的唯一棋盘格
                boardsPtsXY.clear();
                boardsPtsXY.push_back(ptsCV);

                allImagesBoardsPts.push_back(boardsPtsXY);

                std::cout << "[跳过检测] 已找到对应 TXT：" << txtFile << std::endl;

                file_counter_++;
                continue;
            }
        }

        // ==========================
        //  2. 否则 → 正常读取 + 棋盘格检测
        // ==========================
        cv::Mat img;
        // BMP 文件可能超过 OpenCV 的 10.7亿像素限制，用手动读取绕过
        if (p.extension() == ".bmp" || p.extension() == ".BMP") {
            img = readLargeBMP(file_path);
        } else {
            img = cv::imread(file_path, cv::IMREAD_GRAYSCALE);
        }
        PLOGD << "正在检测第 " << file_counter_ << " 图像: " << file_path;

        boardsPtsXY = processSingleImage(img, file_counter_);

        if (boardsPtsXY.empty()) {
            std::cerr << "[警告] 在裁剪区域内未检测到棋盘格" << std::endl;
        }

        // ==========================
        //  3. 保存 TXT（只保存第一个检测到的棋盘格，使用原文件名）
        // ==========================
        if (!boardsPtsXY.empty() && !boardsPtsXY[0].empty()) {
            // 取第一个棋盘格（第0个棋盘格）的角点
            const std::vector<cv::Point2d>& firstBoard = boardsPtsXY[0];
            saveBoardPointsFile(firstBoard, file_name);
        } else {
            // 没有检测到棋盘格，不创建 txt
            std::cout << "[信息] 图像 " << file_name << " 未检测到可保存的第一个棋盘格。" << std::endl;
        }

        allImagesBoardsPts.push_back(boardsPtsXY);
        file_counter_++;
    }
}
void LibCBDetector::processImagesFromMats(const std::vector<cv::Mat>& images,
                                          std::vector<std::vector<std::vector<cv::Point2d>>>& allImagesBoardsPts) {
    file_counter_ = 0;
    allImagesBoardsPts.clear();
    allImagesBoardsPts.reserve(images.size());
    PLOGD << "正在检测棋盘格（来自内存图像）";

    for (size_t i = 0; i < images.size(); ++i) {
        const cv::Mat& img = images[i];
        if (img.empty()) {
            PLOGE << "第 " << i << " 张图像为空，跳过";
            allImagesBoardsPts.push_back({});
            continue;
        }

        PLOGD << "正在检测第 " << file_counter_ << " 张图像（内存 Mat）";

        // 单张图像多个标定板的角点
        std::vector<std::vector<cv::Point2d>> boardsPtsXY = processSingleImage(img, file_counter_);

        if (boardsPtsXY.empty()) {
            std::cerr << "[警告] 在裁剪区域内未检测到棋盘格" << std::endl;
        }
        allImagesBoardsPts.push_back(boardsPtsXY);

        file_counter_++;
    }
}
void LibCBDetector::test() { PLOGD << "当前标定线程"; }

void LibCBDetector::saveBoardPoints(const std::vector<cv::Point2d>& points) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char timestamp[64];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);
    std::string txtFile = parent_path_ + "/txt/" + std::string(timestamp) + ".txt";

    std::ofstream ofs(txtFile);
    if (ofs.is_open()) {
        ofs << "# Index\tX\tY\n";
        for (size_t i = 0; i < points.size(); ++i) {
            ofs << i << "\t" << points[i].x << "\t" << points[i].y << "\n";
        }
        ofs.close();
        std::cout << "[保存完成] 棋盘格1角点已写入：" << txtFile << std::endl;
    } else {
        std::cerr << "[错误] 无法创建输出文件：" << txtFile << std::endl;
    }
}
void LibCBDetector::saveBoardPointsFile(const std::vector<cv::Point2d>& points, const std::string& imageName) {
    // 将 bmp/jpg/png 改成 txt
    std::filesystem::path p(imageName);
    std::string base = p.stem().string();  // 不带后缀的文件名
    std::string txtFile = parent_path_ + "/txt/" + base + ".txt";

    std::ofstream ofs(txtFile);
    if (ofs.is_open()) {
        ofs << "# Index\tX\tY\n";
        for (size_t i = 0; i < points.size(); ++i) {
            ofs << i << "\t" << points[i].x << "\t" << points[i].y << "\n";
        }
        ofs.close();
        std::cout << "[保存完成] 棋盘格角点已写入：" << txtFile << std::endl;
    } else {
        std::cerr << "[错误] 无法创建输出文件：" << txtFile << std::endl;
    }
}

bool TelecentricLineCalibrator::calculate_Image_Points(cv::Mat imageInput, cv::Size boardSize,
                                                       std::vector<cv::Point2d> &imagePoints)
{
    // ---------- 1. 灰度化与反色 ----------
    cv::Mat gray;
    if (imageInput.channels() == 3)
        cv::cvtColor(imageInput, gray, cv::COLOR_BGR2GRAY);
    else
        gray = imageInput.clone();
    cv::bitwise_not(gray, gray); // 若圆为黑底白点可省略

    // ---------- 2. Blob 检测参数 ----------
    cv::SimpleBlobDetector::Params params;
    params.minArea = 1e5;
    params.maxArea = 8e5;
    params.minCircularity = 0.7f;
    params.filterByCircularity = true;
    params.filterByColor = false;
    params.filterByConvexity = false;
    params.filterByInertia = false;

    cv::Ptr<cv::FeatureDetector> blobDetector = cv::SimpleBlobDetector::create(params);

    // ---------- 3. 时间戳 ----------
    std::time_t t = std::time(nullptr);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    std::string timestamp = oss.str();

    // ---------- 4. 圆阵检测 ----------
    bool found =
        cv::findCirclesGrid(gray, boardSize, imagePoints, cv::CALIB_CB_SYMMETRIC_GRID | cv::CALIB_CB_CLUSTERING, blobDetector);

    if (!found)
    {
        std::cout << "未检测到圆心，结果图已保存为: " << std::endl;
        // 仍按原逻辑提示，但不保存图（保持与你原代码一致的行为）
        return false;
    }

    // ---------- 5. 亚像素精确化 ----------
    cv::Size winSize(10, 10);
    cv::Size zeroZone(1, 1);
    cv::TermCriteria tc(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 100, 1e-3);
    cv::cornerSubPix(gray, imagePoints, winSize, zeroZone, tc);

    // ---------- 6. 点序规范（保持最终为：上→下、左→右，行优先） ----------
    const int W = boardSize.width;
    const int H = boardSize.height;
    if (imagePoints.size() == static_cast<size_t>(W * H))
    {
        auto mean_row_y = [&](int r, const std::vector<cv::Point2d> &P) -> double
        {
            double s = 0.0;
            for (int c = 0; c < W; ++c)
                s += P[r * W + c].y;
            return s / W;
        };
        auto row_y_variance_sum = [&](const std::vector<cv::Point2d> &P) -> double
        {
            double total = 0.0;
            for (int r = 0; r < H; ++r)
            {
                double mu = mean_row_y(r, P);
                double var = 0.0;
                for (int c = 0; c < W; ++c)
                {
                    double dy = P[r * W + c].y - mu;
                    var += dy * dy;
                }
                total += var / W;
            }
            return total;
        };

        // 6.1 列优先 → 行优先（必要时做转置）
        double rowVar_A = row_y_variance_sum(imagePoints);
        std::vector<cv::Point2d> transposed(imagePoints.size());
        for (int r = 0; r < H; ++r)
            for (int c = 0; c < W; ++c)
                transposed[r * W + c] = imagePoints[c * H + r];
        double rowVar_B = row_y_variance_sum(transposed);
        if (rowVar_B + 1e-6 < rowVar_A)
        {
            imagePoints.swap(transposed);
            std::cout << " 检测到列优先 → 已转置为行优先\n";
        }

        // 6.2 每行保证 x 递增（左→右）
        for (int r = 0; r < H; ++r)
        {
            int idx = r * W;
            int inc = 0;
            for (int c = 0; c < W - 1; ++c)
                if (imagePoints[idx + c + 1].x > imagePoints[idx + c].x)
                    ++inc;
            if (inc < (W - 1) / 2)
            { // 多数递减 → 镜像一行
                for (int c = 0; c < W / 2; ++c)
                    std::swap(imagePoints[idx + c], imagePoints[idx + (W - 1 - c)]);
                std::cout << " 行 " << r << " 发生镜像，已纠正为 x 递增\n";
            }
        }

        // 6.3 整体保证第一行为“更靠上”（上→下）
        double mean_top_y = 0.0, mean_bottom_y = 0.0;
        for (int c = 0; c < W; ++c)
        {
            mean_top_y += imagePoints[c].y;
            mean_bottom_y += imagePoints[(H - 1) * W + c].y;
        }
        mean_top_y /= W;
        mean_bottom_y /= W;
        if (mean_top_y > mean_bottom_y)
        {
            // 反转所有行的顺序（上下翻转）
            for (int r = 0; r < H / 2; ++r)
            {
                for (int c = 0; c < W; ++c)
                {
                    std::swap(imagePoints[r * W + c], imagePoints[(H - 1 - r) * W + c]);
                }
            }
            std::cout << " 整体做了上下翻转，统一为上→下\n";
        }
    }

    // ---------- 7. 绘制结果（保存路径与命名按你的原程序） ----------
    cv::Mat drawImg = imageInput.clone();
    cv::drawChessboardCorners(drawImg, boardSize, imagePoints, found);
    std::string gridFile = "./data/CISCamera_Image/img/Detected_Grid_" + timestamp + ".png";
    cv::imwrite(gridFile, drawImg);

    // ---------- 8. 日志输出（同原程序） ----------
    std::string txtFile = "./data/CISCamera_Image/img/Detected_Points_" + timestamp + ".txt";
    std::ofstream ofs(txtFile);
    if (ofs.is_open())
    {
        ofs << "# Index\tX\tY\n";
        for (size_t i = 0; i < imagePoints.size(); ++i)
            ofs << i << "\t" << imagePoints[i].x << "\t" << imagePoints[i].y << "\n";
        ofs.close();
    }

    std::cout << "检测完成，结果文件已保存：\n"
              << "  - 圆心检测图: " << gridFile << "\n"
              << "  - 圆心坐标表: " << txtFile << std::endl;

    return true;
}
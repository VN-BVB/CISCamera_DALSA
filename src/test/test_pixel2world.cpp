#include "test_pixel2world.h"
#include "src/utils/geometry_utils.h"
#include <plog/Log.h>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <limits>
#include <algorithm>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <chrono>
#include <iomanip>

TestPixel2World::TestPixel2World() {}

void TestPixel2World::runBlob()
{
    // 指定图像路径和圆阵尺寸
    std::string imagePath = "E:/work/车门门环拼接/image/0123/5580_2378.bmp";
    cv::Size boardSize(37, 37);

    // 读取图像
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    PLOG_INFO << "开始检测圆心，图像: " << imagePath
              << ", 圆阵尺寸: " << boardSize.width << "x" << boardSize.height;

    // 调用 calculateImagePoints 进行圆心检测
    std::vector<cv::Point2d> imagePoints;
    bool success = calculateImagePoints(image, boardSize, imagePoints);

    if (success) {
        PLOG_INFO << "圆心检测成功，共检测到 " << imagePoints.size() << " 个圆心";
    } else {
        PLOG_ERROR << "圆心检测失败";
    }
}

void TestPixel2World::testPixel2World()
{
    // 读取像素坐标文件
    std::string filePath = "E:/all_circle_centers.txt";
    std::ifstream inFile(filePath);

    if (!inFile.is_open()) {
        PLOG_ERROR << "Failed to open file: " << filePath;
        return;
    }

    // 存储像素坐标点
    std::vector<cv::Point2f> pixelPoints;
    std::string line;

    // 逐行读取文件
    int lineNum = 0;
    while (std::getline(inFile, line)) {
        ++lineNum;
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        // 跳过表头行
        if (lineNum == 1) {
            continue;
        }

        // 解析每行的4列数据：Index Row(Y) Column(X) Radius
        std::istringstream iss(line);
        int index;
        float rowY, columnX, radius;
        if (iss >> index >> rowY >> columnX) {
            // Column(X) 是横坐标，Row(Y) 是纵坐标
            // pixelPoints.push_back(cv::Point2f(columnX + 2378, rowY + 5580));    // 裁剪后图像在原图上的坐标
            pixelPoints.push_back(cv::Point2f(columnX , rowY ));
        } else {
            PLOG_WARNING << "Invalid line format at line " << lineNum << ": " << line;
        }
    }
    inFile.close();

    if (pixelPoints.empty()) {
        PLOG_ERROR << "No valid pixel points found in file";
        return;
    }

    PLOG_INFO << "Read " << pixelPoints.size() << " pixel points from " << filePath;

    // 调用 pixel2World 将像素坐标转换为世界坐标
    std::vector<Eigen::Vector2d> worldPoints = GeometryUtils::pixel2World(pixelPoints);

    // 输出转换结果
    PLOG_INFO << "Pixel to World conversion completed:";
    for (size_t i = 0; i < pixelPoints.size() && i < worldPoints.size(); ++i) {
        const cv::Point2f& pixPt = pixelPoints[i];
        const Eigen::Vector2d& worldPt = worldPoints[i];
        PLOG_INFO << "Point " << i << ": Pixel(" << pixPt.x << ", " << pixPt.y
                  << ") -> World(" << worldPt.x() << ", " << worldPt.y() << ")";
    }

    // 可选：将世界坐标保存到文件
    std::string outputPath = "./src/test/world_points.txt";
    std::ofstream outFile(outputPath);
    if (outFile.is_open()) {
        for (const auto& point : worldPoints) {
            outFile << point.x() << " " << point.y() << std::endl;
        }
        outFile.close();
        PLOG_INFO << "World points saved to: " << outputPath;
    } else {
        PLOG_ERROR << "Failed to open output file: " << outputPath;
    }

    // 旋转校正并可视化（索引0和10的连线变水平）
    rotateAndVisualize(worldPoints, 0, 10);

    std::vector<Eigen::Vector2d> twoWorldPoints;
    twoWorldPoints.push_back(worldPoints[0]);
    twoWorldPoints.push_back(worldPoints[1368]);
    calculateTwoPointDistance(twoWorldPoints);
    // 计算每个点与离它最近点的距离
    // calculateNearestNeighborDistance(worldPoints);
}

void TestPixel2World::rotateAndVisualize(const std::vector<Eigen::Vector2d> &worldPoints,
                                         size_t idx0, size_t idx1)
{
    if (worldPoints.size() <= std::max(idx0, idx1))
    {
        PLOG_ERROR << "Not enough points (need at least " << (std::max(idx0, idx1) + 1)
                   << ", got " << worldPoints.size() << ")";
        return;
    }

    // 1. 计算两点连线与X轴的夹角
    double dx = worldPoints[idx1].x() - worldPoints[idx0].x();
    double dy = worldPoints[idx1].y() - worldPoints[idx0].y();
    double angleRad = std::atan2(dy, dx);
    double angleDeg = angleRad * 180.0 / CV_PI;

    PLOG_INFO << "========== Rotation Correction ==========";
    PLOG_INFO << "Point " << idx0 << ": (" << worldPoints[idx0].x() << ", " << worldPoints[idx0].y() << ")";
    PLOG_INFO << "Point " << idx1 << ": (" << worldPoints[idx1].x() << ", " << worldPoints[idx1].y() << ")";
    PLOG_INFO << "Angle between line(" << idx0 << "->" << idx1 << ") and X-axis: " << angleDeg << " degrees";

    // 2. 以 idx0 为旋转中心，旋转 -angle 使连线变水平
    double cosA = std::cos(-angleRad);
    double sinA = std::sin(-angleRad);
    double cx = worldPoints[idx0].x();
    double cy = worldPoints[idx0].y();

    PLOG_INFO << "Rotation center: (" << cx << ", " << cy << ")";
    PLOG_INFO << "Rotation angle: " << -angleDeg << " degrees";

    // 3. 对所有点应用旋转变换
    std::vector<Eigen::Vector2d> rotatedPoints;
    rotatedPoints.reserve(worldPoints.size());
    for (const auto &pt : worldPoints)
    {
        double tx = pt.x() - cx;
        double ty = pt.y() - cy;
        double rx = cosA * tx - sinA * ty + cx;
        double ry = sinA * tx + cosA * ty + cy;
        rotatedPoints.push_back(Eigen::Vector2d(rx, ry));
    }

    // 4. 验证旋转结果
    PLOG_INFO << "After rotation:";
    PLOG_INFO << "  Point " << idx0 << ": (" << rotatedPoints[idx0].x() << ", " << rotatedPoints[idx0].y() << ")";
    PLOG_INFO << "  Point " << idx1 << ": (" << rotatedPoints[idx1].x() << ", " << rotatedPoints[idx1].y() << ")";
    double dyAfter = rotatedPoints[idx1].y() - rotatedPoints[idx0].y();
    PLOG_INFO << "  Y difference (should be ~0): " << dyAfter;

    // 5. 保存旋转后的点
    std::string rotatedOutputPath = "./src/test/rotated_world_points.txt";
    std::ofstream rotOutFile(rotatedOutputPath);
    if (rotOutFile.is_open())
    {
        for (const auto &point : rotatedPoints)
        {
            rotOutFile << point.x() << " " << point.y() << std::endl;
        }
        rotOutFile.close();
        PLOG_INFO << "Rotated world points saved to: " << rotatedOutputPath;
    }

    // 6. 可视化
    displayRotatedPoints(worldPoints, rotatedPoints, idx0, idx1);

    // 7. 排序后按每37个点分组着色显示
    sortAndDisplayGrouped(rotatedPoints, 37);
}

void TestPixel2World::displayRotatedPoints(const std::vector<Eigen::Vector2d> &originalPoints,
                                           const std::vector<Eigen::Vector2d> &rotatedPoints,
                                           size_t idx0, size_t idx1)
{
    // 计算原始点范围
    double origMinX = std::numeric_limits<double>::max(), origMaxX = std::numeric_limits<double>::lowest();
    double origMinY = std::numeric_limits<double>::max(), origMaxY = std::numeric_limits<double>::lowest();
    for (const auto &pt : originalPoints)
    {
        origMinX = std::min(origMinX, pt.x());
        origMaxX = std::max(origMaxX, pt.x());
        origMinY = std::min(origMinY, pt.y());
        origMaxY = std::max(origMaxY, pt.y());
    }

    // 计算旋转后点范围
    double rotMinX = std::numeric_limits<double>::max(), rotMaxX = std::numeric_limits<double>::lowest();
    double rotMinY = std::numeric_limits<double>::max(), rotMaxY = std::numeric_limits<double>::lowest();
    for (const auto &pt : rotatedPoints)
    {
        rotMinX = std::min(rotMinX, pt.x());
        rotMaxX = std::max(rotMaxX, pt.x());
        rotMinY = std::min(rotMinY, pt.y());
        rotMaxY = std::max(rotMaxY, pt.y());
    }

    // 画布参数
    int margin = 50;
    int canvasWidth = 1400;
    int canvasHeight = 800;
    int halfWidth = canvasWidth / 2;
    int drawAreaW = halfWidth - 2 * margin;
    int drawAreaH = canvasHeight - 2 * margin;

    cv::Mat canvas(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(255, 255, 255));

    // 标题
    cv::putText(canvas, "Original Points", cv::Point(halfWidth / 2 - 80, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
    cv::putText(canvas, "Rotated Points (Corrected)", cv::Point(halfWidth + halfWidth / 2 - 120, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);

    // 分割线
    cv::line(canvas, cv::Point(halfWidth, 0), cv::Point(halfWidth, canvasHeight),
             cv::Scalar(128, 128, 128), 2);

    // ---- 辅助 lambda：绘制点集 ----
    auto drawPoints = [&](const std::vector<Eigen::Vector2d> &points,
                          double minX, double maxX, double minY, double maxY,
                          int offsetX)
    {
        double rangeX = maxX - minX;
        double rangeY = maxY - minY;
        if (rangeX < 1e-9)
            rangeX = 1.0;
        if (rangeY < 1e-9)
            rangeY = 1.0;
        double scale = std::min(drawAreaW / rangeX, drawAreaH / rangeY);

        // 绘制所有点
        for (size_t i = 0; i < points.size(); ++i)
        {
            int px = offsetX + margin + static_cast<int>((points[i].x() - minX) * scale);
            int py = margin + 20 + static_cast<int>((points[i].y() - minY) * scale);

            cv::Scalar color(200, 100, 50); // 默认蓝色
            int radius = 3;

            if (i == idx0)
            {
                color = cv::Scalar(0, 0, 255); // 红色
                radius = 7;
            }
            else if (i == idx1)
            {
                color = cv::Scalar(0, 200, 0); // 绿色
                radius = 7;
            }
            cv::circle(canvas, cv::Point(px, py), radius, color, -1);
        }

        // 绘制 idx0 -> idx1 连线
        int px0 = offsetX + margin + static_cast<int>((points[idx0].x() - minX) * scale);
        int py0 = margin + 20 + static_cast<int>((points[idx0].y() - minY) * scale);
        int px1 = offsetX + margin + static_cast<int>((points[idx1].x() - minX) * scale);
        int py1 = margin + 20 + static_cast<int>((points[idx1].y() - minY) * scale);
        cv::line(canvas, cv::Point(px0, py0), cv::Point(px1, py1),
                 cv::Scalar(0, 165, 255), 2);

        // 标注点序号
        cv::putText(canvas, std::to_string(idx0), cv::Point(px0 + 8, py0 - 8),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 255), 1);
        cv::putText(canvas, std::to_string(idx1), cv::Point(px1 + 8, py1 - 8),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 200, 0), 1);
    };

    // 绘制左侧原始点
    drawPoints(originalPoints, origMinX, origMaxX, origMinY, origMaxY, 0);
    // 绘制右侧旋转后的点
    drawPoints(rotatedPoints, rotMinX, rotMaxX, rotMinY, rotMaxY, halfWidth);

    // 图例
    int legendY = canvasHeight - 25;
    cv::circle(canvas, cv::Point(20, legendY), 5, cv::Scalar(0, 0, 255), -1);
    cv::putText(canvas, ("Point " + std::to_string(idx0)), cv::Point(30, legendY + 5),
                cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 0), 1);
    cv::circle(canvas, cv::Point(150, legendY), 5, cv::Scalar(0, 200, 0), -1);
    cv::putText(canvas, ("Point " + std::to_string(idx1)), cv::Point(160, legendY + 5),
                cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 0), 1);
    cv::line(canvas, cv::Point(280, legendY), cv::Point(320, legendY),
             cv::Scalar(0, 165, 255), 2);
    cv::putText(canvas, "Connection Line", cv::Point(325, legendY + 5),
                cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 0), 1);

    // 显示
    cv::namedWindow("Rotation Correction", cv::WINDOW_NORMAL);
    cv::resizeWindow("Rotation Correction", 1400, 800);
    cv::imshow("Rotation Correction", canvas);

    // 保存
    std::string savePath = "./src/test/rotation_visualization.png";
    cv::imwrite(savePath, canvas);
    PLOG_INFO << "Visualization saved to: " << savePath;

    cv::waitKey(0);
    cv::destroyAllWindows();
}

void TestPixel2World::calculateTwoPointDistance(const std::vector<Eigen::Vector2d>& worldPoints)
{
    double dx = worldPoints[0].x() - worldPoints[1].x();
    double dy = worldPoints[0].y() - worldPoints[1].y();
    double distance = std::sqrt(dx * dx + dy * dy);

    std::cout << "======distance: " << distance << std::endl;
}

void TestPixel2World::calculateNearestNeighborDistance(const std::vector<Eigen::Vector2d>& worldPoints)
{
    if (worldPoints.size() < 2) {
        PLOG_ERROR << "Need at least 2 points to calculate nearest neighbor distance";
        return;
    }

    std::vector<double> nearestDistances;
    nearestDistances.reserve(worldPoints.size());

    PLOG_INFO << "========== Nearest Neighbor Distance Analysis ==========";

    // 计算每个点到其他点的最短距离
    for (size_t i = 0; i < worldPoints.size(); ++i) {
        double minDistance = std::numeric_limits<double>::max();
        size_t nearestIndex = 0;

        for (size_t j = 0; j < worldPoints.size(); ++j) {
            if (i == j) {
                continue; // 跳过自己
            }

            // 计算两点之间的欧氏距离
            double dx = worldPoints[i].x() - worldPoints[j].x();
            double dy = worldPoints[i].y() - worldPoints[j].y();
            double distance = std::sqrt(dx * dx + dy * dy);

            if (distance < minDistance) {
                minDistance = distance;
                nearestIndex = j;
            }
        }

        nearestDistances.push_back(minDistance);
        PLOG_INFO << "Point " << i << " (" << worldPoints[i].x() << ", " << worldPoints[i].y()
                  << ") -> Nearest: Point " << nearestIndex << " at distance " << minDistance;
    }

    // 计算平均最短距离
    double sum = 0.0;
    for (const auto& dist : nearestDistances) {
        sum += dist;
    }
    double averageDistance = sum / nearestDistances.size();

    // 找出最大和最小最短距离
    double maxMinDistance = *std::max_element(nearestDistances.begin(), nearestDistances.end());
    double minMinDistance = *std::min_element(nearestDistances.begin(), nearestDistances.end());

    PLOG_INFO << "========== Summary ==========";
    PLOG_INFO << "Average nearest neighbor distance: " << averageDistance;
    PLOG_INFO << "Maximum nearest neighbor distance: " << maxMinDistance;
    PLOG_INFO << "Minimum nearest neighbor distance: " << minMinDistance;

    // 保存距离分析结果到文件
    std::string distanceOutputPath = "./src/test/nearest_neighbor_distances.txt";
    std::ofstream outFile(distanceOutputPath);
    if (outFile.is_open()) {
        outFile << "# Point Index | World Coordinate | Nearest Distance\n";
        for (size_t i = 0; i < worldPoints.size(); ++i) {
            outFile << i << " " << worldPoints[i].x() << " " << worldPoints[i].y()
                    << " " << nearestDistances[i] << "\n";
        }
        outFile << "\n# Summary\n";
        outFile << "Average: " << averageDistance << "\n";
        outFile << "Max: " << maxMinDistance << "\n";
        outFile << "Min: " << minMinDistance << "\n";
        outFile.close();
        PLOG_INFO << "Distance analysis saved to: " << distanceOutputPath;
    } else {
        PLOG_ERROR << "Failed to open distance output file: " << distanceOutputPath;
    }
}

bool TestPixel2World::calculateImagePoints(cv::Mat imageInput, cv::Size boardSize,
                                           std::vector<cv::Point2d>& imagePoints)
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
    params.minArea = 1e4;
    params.maxArea = 7e5;
    params.minCircularity = 0.7f;
    // params.blobColor = 0;
    params.filterByCircularity = true;
    params.filterByColor = false;
    params.filterByConvexity = false;
    params.filterByInertia = false;

    cv::Ptr<cv::FeatureDetector> blobDetector = cv::SimpleBlobDetector::create(params);

    std::vector<cv::KeyPoint> keypoints;
    blobDetector->detect(gray, keypoints);
    PLOG_INFO << "Blob检测到 " << keypoints.size() << "个点";
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
              << "  - 圆心坐标表: " << txtFile << std::endl;

    return true;
}

void TestPixel2World::sortAndDisplayGrouped(const std::vector<Eigen::Vector2d>& rotatedPoints,
                                            int groupSize)
{
    if (rotatedPoints.empty()) {
        PLOG_ERROR << "No points to sort and display";
        return;
    }

    // 1. 复制并排序：先按 x 从小到大，x 相同时按 y 从小到大
    std::vector<Eigen::Vector2d> sortedPoints = rotatedPoints;
    std::sort(sortedPoints.begin(), sortedPoints.end(),
              [](const Eigen::Vector2d& a, const Eigen::Vector2d& b) {
                  if (std::abs(a.x() - b.x()) > 1e-6) {
                      return a.x() < b.x();
                  }
                  return a.y() < b.y();
              });

    PLOG_INFO << "========== Sorted & Grouped Display ==========";
    PLOG_INFO << "Total points: " << sortedPoints.size()
              << ", Group size: " << groupSize
              << ", Number of groups: " << (sortedPoints.size() + groupSize - 1) / groupSize;

    // 2. 保存排序后的点
    std::string sortedOutputPath = "./src/test/sorted_world_points.txt";
    std::ofstream sortOutFile(sortedOutputPath);
    if (sortOutFile.is_open()) {
        sortOutFile << "# Index GroupIndex X Y\n";
        for (size_t i = 0; i < sortedPoints.size(); ++i) {
            sortOutFile << i << " " << (i / groupSize) << " "
                        << sortedPoints[i].x() << " " << sortedPoints[i].y() << "\n";
        }
        sortOutFile.close();
        PLOG_INFO << "Sorted points saved to: " << sortedOutputPath;
    }

    // 3. 预定义一组可区分的颜色（BGR格式）
    std::vector<cv::Scalar> colorPalette = {
        cv::Scalar(255, 0, 0),       // 蓝
        cv::Scalar(0, 0, 255),       // 红
        cv::Scalar(0, 200, 0),       // 绿
        cv::Scalar(0, 165, 255),     // 橙
        cv::Scalar(255, 0, 255),     // 洋红
        cv::Scalar(255, 255, 0),     // 青
        cv::Scalar(0, 255, 255),     // 黄
        cv::Scalar(128, 0, 128),     // 紫
        cv::Scalar(0, 128, 255),     // 深橙
        cv::Scalar(203, 192, 255),   // 粉
        cv::Scalar(42, 42, 165),     // 棕
        cv::Scalar(180, 105, 255),   // 热粉
        cv::Scalar(255, 191, 0),     // 深天蓝
        cv::Scalar(19, 69, 139),     // 深棕
        cv::Scalar(0, 100, 0),       // 深绿
        cv::Scalar(130, 0, 75),      // 靛蓝
        cv::Scalar(60, 20, 220),     // 深红
        cv::Scalar(0, 215, 255),     // 金色
        cv::Scalar(128, 128, 0),     // 蓝绿
        cv::Scalar(50, 205, 50),     // 石灰绿
        cv::Scalar(147, 20, 255),    // 深粉
        cv::Scalar(255, 144, 30),    // 道奇蓝
        cv::Scalar(34, 139, 34),     // 森林绿
        cv::Scalar(0, 0, 128),       // 栗色
        cv::Scalar(139, 139, 0),     // 暗青
        cv::Scalar(211, 0, 148),     // 深紫
        cv::Scalar(71, 99, 255),     // 番茄红
        cv::Scalar(107, 183, 189),   // 深卡其
        cv::Scalar(140, 180, 210),   // 棕褐
        cv::Scalar(0, 252, 124),     // 春绿
        cv::Scalar(255, 105, 65),    // 皇家蓝
        cv::Scalar(113, 179, 60),    // 中海蓝
        cv::Scalar(0, 69, 255),      // 橙红
        cv::Scalar(204, 50, 153),    // 中兰花紫
        cv::Scalar(250, 230, 230),   // 淡紫
        cv::Scalar(180, 130, 70),    // 钢蓝
        cv::Scalar(128, 0, 0),       // 深蓝(海军蓝)
    };

    // 4. 计算绘图范围
    double minX = std::numeric_limits<double>::max(), maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max(), maxY = std::numeric_limits<double>::lowest();
    for (const auto& pt : sortedPoints) {
        minX = std::min(minX, pt.x()); maxX = std::max(maxX, pt.x());
        minY = std::min(minY, pt.y()); maxY = std::max(maxY, pt.y());
    }

    // 5. 画布参数
    int margin = 60;
    int canvasWidth = 1200;
    int canvasHeight = 900;
    int drawAreaW = canvasWidth - 2 * margin;
    int drawAreaH = canvasHeight - 2 * margin - 40; // 预留标题和图例空间

    double rangeX = maxX - minX;
    double rangeY = maxY - minY;
    if (rangeX < 1e-9) rangeX = 1.0;
    if (rangeY < 1e-9) rangeY = 1.0;
    double scale = std::min(drawAreaW / rangeX, drawAreaH / rangeY);

    cv::Mat canvas(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(255, 255, 255));

    // 标题
    int numGroups = static_cast<int>((sortedPoints.size() + groupSize - 1) / groupSize);
    std::string title = "Sorted Points - " + std::to_string(numGroups) + " groups (every "
                        + std::to_string(groupSize) + " points)";
    cv::putText(canvas, title, cv::Point(canvasWidth / 2 - 220, 28),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(0, 0, 0), 2);

    // 6. 绘制所有点，按组着色
    for (size_t i = 0; i < sortedPoints.size(); ++i) {
        int groupIdx = static_cast<int>(i / groupSize);
        cv::Scalar color = colorPalette[groupIdx % colorPalette.size()];

        int px = margin + static_cast<int>((sortedPoints[i].x() - minX) * scale);
        int py = margin + 30 + static_cast<int>((sortedPoints[i].y() - minY) * scale);

        cv::circle(canvas, cv::Point(px, py), 4, color, -1);

        // 每组的首尾点用更大的圆标记
        if (static_cast<int>(i % groupSize) == 0) {
            cv::circle(canvas, cv::Point(px, py), 7, color, 2); // 空心大圆标记组首
        }
    }

    // 7. 绘制图例（显示前几组的颜色对应关系）
    int legendStartY = canvasHeight - 40;
    int legendX = margin;
    int maxLegendCols = 10;
    int legendColWidth = (canvasWidth - 2 * margin) / maxLegendCols;

    for (int g = 0; g < numGroups && g < 40; ++g) {
        int col = g % maxLegendCols;
        int row = g / maxLegendCols;
        int lx = legendX + col * legendColWidth;
        int ly = legendStartY - row * 18;

        cv::Scalar color = colorPalette[g % colorPalette.size()];
        cv::circle(canvas, cv::Point(lx, ly), 4, color, -1);

        std::string label = "G" + std::to_string(g);
        cv::putText(canvas, label, cv::Point(lx + 8, ly + 4),
                    cv::FONT_HERSHEY_SIMPLEX, 0.32, cv::Scalar(0, 0, 0), 1);
    }

    // 8. 显示
    cv::namedWindow("Sorted Grouped Points", cv::WINDOW_NORMAL);
    cv::resizeWindow("Sorted Grouped Points", 1200, 900);
    cv::imshow("Sorted Grouped Points", canvas);

    // 9. 保存
    std::string savePath = "./src/test/sorted_grouped_visualization.png";
    cv::imwrite(savePath, canvas);
    PLOG_INFO << "Sorted grouped visualization saved to: " << savePath;

    cv::waitKey(0);
    cv::destroyAllWindows();

    // 10. 计算每组首尾点间距统计
    calculateGroupSpanDistance(sortedPoints, groupSize);
}

void TestPixel2World::calculateGroupSpanDistance(const std::vector<Eigen::Vector2d>& sortedPoints,
                                                 int groupSize)
{
    if (sortedPoints.empty()) {
        PLOG_ERROR << "No sorted points for group span distance calculation";
        return;
    }

    int totalPoints = static_cast<int>(sortedPoints.size());
    int numGroups = (totalPoints + groupSize - 1) / groupSize;

    PLOG_INFO << "========== Group Span Distance Analysis ==========";
    PLOG_INFO << "Total points: " << totalPoints
              << ", Group size: " << groupSize
              << ", Number of groups: " << numGroups;

    std::vector<double> groupDistances;
    groupDistances.reserve(numGroups);

    for (int g = 0; g < numGroups; ++g) {
        int startIdx = g * groupSize;
        int endIdx = std::min(startIdx + groupSize - 1, totalPoints - 1);

        // 如果该组只有一个点，跳过
        if (startIdx == endIdx) {
            PLOG_WARNING << "Group " << g << ": only 1 point, skipping";
            continue;
        }

        const Eigen::Vector2d& ptStart = sortedPoints[startIdx];
        const Eigen::Vector2d& ptEnd = sortedPoints[endIdx];

        double dx = ptEnd.x() - ptStart.x();
        double dy = ptEnd.y() - ptStart.y();
        double distance = std::sqrt(dx * dx + dy * dy);

        groupDistances.push_back(distance);

        PLOG_INFO << "Group " << g
                  << ": [" << startIdx << "](" << ptStart.x() << ", " << ptStart.y() << ")"
                  << " -> [" << endIdx << "](" << ptEnd.x() << ", " << ptEnd.y() << ")"
                  << ", Distance = " << distance;
    }

    if (groupDistances.empty()) {
        PLOG_ERROR << "No valid group distances calculated";
        return;
    }

    // 统计分析
    double sum = 0.0;
    for (const auto& d : groupDistances) {
        sum += d;
    }
    double avgDistance = sum / groupDistances.size();

    auto minIt = std::min_element(groupDistances.begin(), groupDistances.end());
    auto maxIt = std::max_element(groupDistances.begin(), groupDistances.end());
    double minDistance = *minIt;
    double maxDistance = *maxIt;
    int minGroupIdx = static_cast<int>(std::distance(groupDistances.begin(), minIt));
    int maxGroupIdx = static_cast<int>(std::distance(groupDistances.begin(), maxIt));

    // 计算标准差
    double varianceSum = 0.0;
    for (const auto& d : groupDistances) {
        varianceSum += (d - avgDistance) * (d - avgDistance);
    }
    double stdDev = std::sqrt(varianceSum / groupDistances.size());

    PLOG_INFO << "========== Group Span Distance Summary ==========";
    PLOG_INFO << "Valid groups:      " << groupDistances.size();
    PLOG_INFO << "Average distance:  " << avgDistance;
    PLOG_INFO << "Min distance:      " << minDistance << " (Group " << minGroupIdx << ")";
    PLOG_INFO << "Max distance:      " << maxDistance << " (Group " << maxGroupIdx << ")";
    PLOG_INFO << "Std deviation:     " << stdDev;
    PLOG_INFO << "Range (Max-Min):   " << (maxDistance - minDistance);
}

#define _USE_MATH_DEFINES
#include <QDebug>
#include <QString>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

#include "src/config/calibration_data_io.h"
#include "src/ui/CISCamera_imageGrab/cameraImage_processor.h"
#include "src/utils/image_utils.cpp"

static cv::Mat load(const QString &path) {
    std::string local = path.toLocal8Bit().constData();
    cv::Mat m = readLargeBMP(local);
    if (m.empty()) m = cv::imread(local, cv::IMREAD_GRAYSCALE);
    return m;
}

// ── 检测白圆（与 whenDrawDetectCircles 一致） ──
static std::vector<Eigen::Vector2d> detectWhiteCircles(const cv::Mat &img) {
    cv::SimpleBlobDetector::Params params;
    params.minArea = 6e4;
    params.maxArea = 9e4;
    params.minCircularity = 0.7f;
    params.filterByCircularity = true;
    params.filterByColor = false;
    auto detector = cv::SimpleBlobDetector::create(params);
    std::vector<cv::KeyPoint> kps;
    detector->detect(img, kps);

    cv::Mat gray;
    if (img.channels() == 3)
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else
        gray = img.clone();
    cv::Mat binary;
    cv::threshold(gray, binary, 240, 255, cv::THRESH_BINARY);

    std::vector<Eigen::Vector2d> centers;
    for (auto &kp : kps) {
        int cx = cvRound(kp.pt.x), cy = cvRound(kp.pt.y);
        int roiSize = cvRound(kp.size) + 20;
        int x = std::max(0, cx - roiSize / 2);
        int y = std::max(0, cy - roiSize / 2);
        int w = std::min(roiSize, binary.cols - x);
        int h = std::min(roiSize, binary.rows - y);
        cv::Mat roiBin = binary(cv::Rect(x, y, w, h));
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(roiBin, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        if (contours.empty()) continue;
        auto &cnt = *std::max_element(contours.begin(), contours.end(), [](auto &a, auto &b) { return cv::contourArea(a) < cv::contourArea(b); });
        if (cnt.size() < 6) continue;
        cv::Point2f ctr;
        float r;
        cv::minEnclosingCircle(cnt, ctr, r);
        centers.emplace_back(ctr.x + x, ctr.y + y);
    }
    return centers;
}

// ── 检测黑环（验证图用，与 detectWhiteCircles 同结构） ──
static std::vector<Eigen::Vector2d> detectBlackCircles(const cv::Mat &img, int minA, int maxA) {
    qDebug() << "[black] step1: params minA=" << minA << "maxA=" << maxA;
    cv::SimpleBlobDetector::Params params;
    params.minArea = minA;
    params.maxArea = maxA;
    params.minCircularity = 0.6f;
    params.filterByCircularity = true;
    params.filterByColor = false;
    qDebug() << "[black] step2: create detector";
    auto detector = cv::SimpleBlobDetector::create(params);
    qDebug() << "[black] step3: detect, img channels=" << img.channels() << "size=" << img.cols << img.rows;
    std::vector<cv::KeyPoint> kps;
    detector->detect(img, kps);
    qDebug() << "[black] step4: blobs found=" << kps.size();

    cv::Mat gray;
    if (img.channels() == 3)
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else
        gray = img.clone();
    qDebug() << "[black] step5: gray ok, threshold";
    cv::Mat binary;
    cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY_INV);
    qDebug() << "[black] step6: binary ok, contours";

    std::vector<Eigen::Vector2d> centers;
    for (auto &kp : kps) {
        int cx = cvRound(kp.pt.x), cy = cvRound(kp.pt.y);
        int roiSize = cvRound(kp.size) + 20;
        int x = std::max(0, cx - roiSize / 2);
        int y = std::max(0, cy - roiSize / 2);
        int w = std::min(roiSize, binary.cols - x);
        int h = std::min(roiSize, binary.rows - y);
        cv::Mat roiBin = binary(cv::Rect(x, y, w, h));
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(roiBin, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        if (contours.empty()) continue;
        auto &cnt = *std::max_element(contours.begin(), contours.end(), [](auto &a, auto &b) { return cv::contourArea(a) < cv::contourArea(b); });
        if (cnt.size() < 6) continue;
        cv::Point2f ctr;
        float r;
        cv::minEnclosingCircle(cnt, ctr, r);
        centers.emplace_back(ctr.x + x, ctr.y + y);
    }
    qDebug() << "[black] step7: done, centers=" << centers.size();
    return centers;
}

int runEasyCalib() {
    CameraImageProcessor proc;
    proc.initCameraCalibrator();

    QString d = "D:/Code/CISCamera_DALSA/data/PaltfromCalibrate/easycalib/";
    QString dt = "D:/Code/CISCamera_DALSA/data/PaltfromCalibrate/test/";
    QString outPath = "D:/Code/CISCamera_DALSA/data/calibration_config/easy_platform_pose.json";

    // ── 1. 检测 5 张图的白色圆点 ──
    auto c0 = detectWhiteCircles(load(d + u8"原点.bmp"));
    auto cX = detectWhiteCircles(load(d + u8"原点x方向平移5mm.bmp"));
    auto cY = detectWhiteCircles(load(d + u8"原点y方向平移5mm.bmp"));
    auto c45 = detectWhiteCircles(load(d + u8"原点转45度.bmp"));
    auto c90 = detectWhiteCircles(load(d + u8"原点转90度.bmp"));

    int n0 = c0.size();
    qDebug() << "检测到:" << n0 << cX.size() << cY.size() << c45.size() << c90.size() << "个圆";
    if (n0 < 1) {
        qDebug() << "原点图无圆";
        return 1;
    }

    // ── 2. 全部转到世界坐标 ──
    auto w0 = proc.convertToWorld(c0);
    auto wX = proc.convertToWorld(cX);
    auto wY = proc.convertToWorld(cY);
    auto w45 = proc.convertToWorld(c45);
    auto w90 = proc.convertToWorld(c90);

    // ── 3. 逐点匹配：对原点每个世界点，在其他图里找最近点（阈值 50mm） ──
    auto match = [&](const std::vector<Eigen::Vector2d> &wsrc, const std::vector<Eigen::Vector2d> &wdst) -> std::vector<int> {
        // 返回 wsrc 中每个点在 wdst 中的索引，找不到为 -1
        std::vector<int> idx(wsrc.size(), -1);
        std::vector<bool> used(wdst.size(), false);
        for (size_t i = 0; i < wsrc.size(); ++i) {
            double bestDist = 50.0;
            int bestJ = -1;
            for (size_t j = 0; j < wdst.size(); ++j) {
                if (used[j]) continue;
                double d = (wsrc[i] - wdst[j]).norm();
                if (d < bestDist) {
                    bestDist = d;
                    bestJ = (int)j;
                }
            }
            if (bestJ >= 0) {
                idx[i] = bestJ;
                used[bestJ] = true;
            }
        }
        return idx;
    };

    auto iX = match(w0, wX);
    auto iY = match(w0, wY);
    auto i45 = match(w0, w45);
    auto i90 = match(w0, w90);

    // ── 4. 计算每个平台的标定结果 ──
    PlatformPoseData poseData;
    int matchedCount = 0;
    for (int i = 0; i < n0; ++i) {
        if (iX[i] < 0 || iY[i] < 0 || i45[i] < 0 || i90[i] < 0) {
            qDebug() << "平台" << i << "匹配不全，跳过";
            continue;
        }
        Eigen::Vector2d oW = w0[i];
        Eigen::Vector2d xW = wX[iX[i]];
        Eigen::Vector2d yW = wY[iY[i]];
        Eigen::Vector2d r45W = w45[i45[i]];
        Eigen::Vector2d r90W = w90[i90[i]];

        Eigen::Vector2d dXW = CameraImageProcessor::calcDirectionVector(oW, xW);
        Eigen::Vector2d dYW = CameraImageProcessor::calcDirectionVector(oW, yW);
        Eigen::Vector2d rotCenter = CameraImageProcessor::calcCircumcenter(oW, r45W, r90W);

        // 方向向量 → 旋转向量
        Eigen::Vector3d dx3(dXW.x(), dXW.y(), 0);
        Eigen::Vector3d dy3(dYW.x(), dYW.y(), 0);
        Eigen::Vector3d dz3 = dx3.cross(dy3);
        cv::Mat Rmat(3, 3, CV_64F);
        Rmat.at<double>(0, 0) = dx3(0);
        Rmat.at<double>(0, 1) = dy3(0);
        Rmat.at<double>(0, 2) = dz3(0);
        Rmat.at<double>(1, 0) = dx3(1);
        Rmat.at<double>(1, 1) = dy3(1);
        Rmat.at<double>(1, 2) = dz3(1);
        Rmat.at<double>(2, 0) = dx3(2);
        Rmat.at<double>(2, 1) = dy3(2);
        Rmat.at<double>(2, 2) = dz3(2);
        cv::Mat rvec;
        cv::Rodrigues(Rmat, rvec);

        poseData.allRotVecs.push_back(Eigen::Vector3d(rvec.at<double>(0), rvec.at<double>(1), rvec.at<double>(2)));
        poseData.allTransVecs.push_back(Eigen::Vector3d(rotCenter.x(), rotCenter.y(), 0));
        ++matchedCount;

        qDebug() << "平台" << i << " dirX" << dXW.x() << dXW.y() << " dirY" << dYW.x() << dYW.y() << " center" << rotCenter.x() << rotCenter.y();
    }
    if (matchedCount == 0) {
        qDebug() << "没有成功匹配的平台";
        return 1;
    }

    poseData.allRotVecs.push_back(proc.getWorldRvec());
    poseData.allTransVecs.push_back(proc.getWorldTvec());

    if (!poseData.save(outPath.toStdString())) {
        qDebug() << "保存失败：" << outPath;
        return 1;
    }
    qDebug() << "简易标定结果已保存到" << outPath << "(" << matchedCount << "个平台)";

    // ── 5. 验证图黑圆检测 ──
    qDebug() << "\n===== 验证图黑圆 =====";
    QStringList tests = {
        u8"标定验证原点.bmp",
        u8"标定验证x方向5mm.bmp",
        u8"标定验证y方向5mm.bmp",
        u8"标定验证旋转10度.bmp",
    };
    for (auto &f : tests) {
        auto img = load(dt + f);
        qDebug() << f << "empty?" << img.empty() << "size" << img.cols << img.rows;
        if (img.empty()) continue;
        auto pts = detectBlackCircles(img, 150000, 200000);
        qDebug() << "  检测到" << pts.size() << "个黑圆";
        for (size_t i = 0; i < pts.size(); ++i) {
            qDebug() << "  黑圆" << i << "center=" << pts[i].x() << pts[i].y();
        }
    }
    return 0;
}

// ── 标定误差对比分析：棋盘格(std) vs 简易标定(easy) vs 实际 ──
void runCalibComparison() {
    // 按 std 标定序号 0-6 (平台1-7)
    std::vector<Eigen::Vector2d> origin = {
        {14845, 1257.5},      // P1 std_0
        {26555.5, 10304},     // P2 std_1
        {26042.7, 26176.3},   // P3 std_2
        {16085.5, 27542.4},   // P4 std_3
        {18193.70, 13418.97}, // P5 std_4
        {11110.5, 9738.5},    // P6 std_5
        {11392.5, 30789.5},   // P7 std_6
    };

    struct TestCase {
        QString name;
        std::vector<Eigen::Vector2d> actual;
        std::vector<Eigen::Vector2d> stdV;
        std::vector<Eigen::Vector2d> easyV;
    };

    std::vector<TestCase> cases = {
        {"Case1: 旋转10度 平移(0,0)",
         {{14256.1,826.5},{27086,9587},{27171.2,25904.2},{15617.5,28774.7},{18189.5,13409.5},{10962.5,9005.5},{12815.8,30538}},
         {{14231.66,820.95},{27091.04,9588.32},{27168.73,25891.91},{15608.73,28771.86},{18193.70,13418.97},{10951.05,9016.47},{12804.61,30509.41}},
         {{14232.08,820.08},{27093.20,9587.61},{27170.79,25892.10},{15610.64,28770.72},{18192.62,13417.21},{10952.42,9016.98},{12805.51,30509.05}}},
        {"Case2: 平移(5,0) 旋转0度",
         {{14857.5,1020.5},{26580,10049.5},{26050.5,25938.7},{16080.9,27304.5},{18785,13482.5},{11344.5,9704},{11385.5,30551.5}},
         {{14858.31,1021.79},{26583.87,10069.61},{26047.48,25940.19},{16079.87,27306.27},{18781.61,13488.57},{11346.19,9722.41},{11385.71,30553.48}},
         {{14860.00,1021.90},{26585.49,10069.82},{26046.34,25940.16},{16081.86,27306.23},{18775.99,13489.06},{11346.19,9722.50},{11389.67,30553.42}}},
        {"Case3: 平移(0,5) 旋转0度",
         {{15081.5,1269.5},{26782.2,10320.3},{26278.1,26181.8},{16323.9,27545.8},{19019.7,13679.3},{11133.8,9963.87},{11627.1,30779.5}},
         {{15081.04,1271.23},{26790.12,10333.16},{26278.72,26181.81},{16321.48,27537.52},{19035.48,13706.58},{11127.07,9974.22},{11628.34,30783.50}},
         {{15080.94,1272.90},{26789.92,10334.78},{26278.74,26180.66},{16321.52,27539.51},{19034.98,13700.96},{11126.98,9974.22},{11628.42,30787.47}}},
    };

    auto pixDist = [](const Eigen::Vector2d &a, const Eigen::Vector2d &b) {
        return (a - b).norm();
    };

    // 用于按平台汇总
    double perPlatStdE[7] = {}, perPlatEasyE[7] = {};
    double perPlatStdEx[7] = {}, perPlatStdEy[7] = {};
    double perPlatEasyEx[7] = {}, perPlatEasyEy[7] = {};
    int nCases = (int)cases.size();

    for (auto &tc : cases) {
        qDebug() << "";
        qDebug() << "================================================================";
        qDebug() << " " << tc.name;
        qDebug() << "================================================================";
        qDebug() << "  平台  | 实际坐标(px)        | std预测             | easy预测            | std误差  easy误差";
        qDebug() << "  ------+---------------------+---------------------+---------------------+----------------";

        for (int i = 0; i < 7; ++i) {
            double stdErr = pixDist(tc.stdV[i], tc.actual[i]);
            double easyErr = pixDist(tc.easyV[i], tc.actual[i]);

            perPlatStdE[i] += stdErr;
            perPlatEasyE[i] += easyErr;
            perPlatStdEx[i] += tc.stdV[i].x() - tc.actual[i].x();
            perPlatStdEy[i] += tc.stdV[i].y() - tc.actual[i].y();
            perPlatEasyEx[i] += tc.easyV[i].x() - tc.actual[i].x();
            perPlatEasyEy[i] += tc.easyV[i].y() - tc.actual[i].y();

            QString flag = (stdErr > 15 || easyErr > 15) ? " <<< !" : "";
            qDebug() << QString("  P%1    | (%2, %3) | (%4, %5) | (%6, %7) | %8   %9%10")
                            .arg(i + 1)
                            .arg(tc.actual[i].x(), 0, 'f', 1).arg(tc.actual[i].y(), 0, 'f', 1)
                            .arg(tc.stdV[i].x(), 0, 'f', 1).arg(tc.stdV[i].y(), 0, 'f', 1)
                            .arg(tc.easyV[i].x(), 0, 'f', 1).arg(tc.easyV[i].y(), 0, 'f', 1)
                            .arg(stdErr, 0, 'f', 2).arg(easyErr, 0, 'f', 2).arg(flag);
        }
    }

    // ── 按平台汇总 ──
    qDebug() << "";
    qDebug() << "================================================================";
    qDebug() << "  各平台误差汇总 (3个Case平均, 单位: 像素)";
    qDebug() << "================================================================";
    qDebug() << "";
    qDebug() << "  [欧氏距离误差]  越小越好";
    qDebug() << "  平台  | std误差  | easy误差 | 谁更好";
    qDebug() << "  ------+----------+----------+-------";

    for (int i = 0; i < 7; ++i) {
        double avgStd = perPlatStdE[i] / nCases;
        double avgEasy = perPlatEasyE[i] / nCases;
        QString winner = (avgStd < avgEasy) ? "std胜" : (avgStd > avgEasy) ? "easy胜" : "平";
        double gap = std::abs(avgStd - avgEasy);
        QString flag = "";
        if (avgStd > 14 || avgEasy > 14) flag = " <<< 偏大";
        qDebug() << QString("  P%1    | %2 | %3 | %4 (差%5)%6")
                        .arg(i + 1)
                        .arg(avgStd, 0, 'f', 2).arg(avgEasy, 0, 'f', 2)
                        .arg(winner).arg(gap, 0, 'f', 2).arg(flag);
    }

    // 总平均
    double totalStd = 0, totalEasy = 0;
    for (int i = 0; i < 7; ++i) { totalStd += perPlatStdE[i]; totalEasy += perPlatEasyE[i]; }
    qDebug() << "  ------+----------+----------+-------";
    qDebug() << QString("  平均  | %1 | %2 |")
                    .arg(totalStd / (7 * nCases), 0, 'f', 2)
                    .arg(totalEasy / (7 * nCases), 0, 'f', 2);

    // ── XY方向误差分解 ──
    qDebug() << "";
    qDebug() << "  [XY方向误差分解]  正=偏右/偏下, 负=偏左/偏上";
    qDebug() << "  平台  | std_dX   std_dY  | easy_dX   easy_dY";
    qDebug() << "  ------+-----------------+------------------";

    for (int i = 0; i < 7; ++i) {
        double sdx = perPlatStdEx[i] / nCases;
        double sdy = perPlatStdEy[i] / nCases;
        double edx = perPlatEasyEx[i] / nCases;
        double edy = perPlatEasyEy[i] / nCases;
        qDebug() << QString("  P%1    | %+8.2f %+8.2f | %+8.2f %+8.2f")
                        .arg(i + 1).arg(sdx).arg(sdy).arg(edx).arg(edy);
    }
}

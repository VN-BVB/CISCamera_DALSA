#include <algorithm>
#include <string>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "sorting_strategy.h"
#include "src/utils/geometry_utils.h"
#include "src/utils/scoped_timer.h"

SortingStrategy::SortingStrategy() {}

bool SortingStrategy::process(ContourData& context) {
    auto contour = context.getSubpixelContour();
    auto [startPoint, endPoint] = ContourFeatureCalculator::calculateStartAndEndPoint(contour);
    int startIndex = ContourUtils::findPointIndex(startPoint, contour);
    int endIndex = ContourUtils::findPointIndex(endPoint, contour);

    auto sortedContour = ContourFeatureCalculator::sortContourByNearestNeighbor(contour, startIndex, endIndex);
    // 修正走向为逆时针：取起点a、中点b、终点c，叉积>0表示顺时针（图像坐标系y向下），反转
    if (sortedContour.size() >= 3) {
        const auto& a = sortedContour.front();
        const auto& b = sortedContour[sortedContour.size() / 2];
        const auto& c = sortedContour.back();
        float cross = GeometryUtils::crossProduct(b - a, c - a);
        if (cross > 0.0f) {
            std::reverse(sortedContour.begin(), sortedContour.end());
        }
    }
    // 调试可视化：绘制极点、起点（排序后首点）、终点（排序后末点）及开口连线
    if (!sortedContour.empty()) {
        cv::Rect bbox = cv::boundingRect(contour);
        const int kMargin = 30;
        cv::Mat canvas = cv::Mat::zeros(bbox.height + 2 * kMargin, bbox.width + 2 * kMargin, CV_8UC3);
        auto toCanvas = [&](const cv::Point2f& p) {
            return cv::Point(static_cast<int>(p.x - bbox.x) + kMargin,
                             static_cast<int>(p.y - bbox.y) + kMargin);
        };
        // 轮廓点（浅灰）
        for (const auto& p : contour) {
            cv::circle(canvas, toCanvas(p), 1, cv::Scalar(200, 200, 200), -1);
        }
        cv::Point2f center = ContourUtils::calculateCentralPoint(contour);
        const cv::Point2f& sortStart = sortedContour.front();
        const cv::Point2f& sortEnd = sortedContour.back();
        cv::Point centerPx = toCanvas(center);
        cv::Point startPx = toCanvas(sortStart);
        cv::Point endPx = toCanvas(sortEnd);
        // 极点到起终点的连线（黄色），可视化开口角度差
        cv::line(canvas, centerPx, startPx, cv::Scalar(0, 255, 255), 1);
        cv::line(canvas, centerPx, endPx, cv::Scalar(0, 255, 255), 1);
        // 极点（黄色十字）
        cv::drawMarker(canvas, centerPx, cv::Scalar(0, 255, 255), cv::MARKER_CROSS, 12, 2);
        // 起点（绿色 S）
        cv::circle(canvas, startPx, 6, cv::Scalar(0, 255, 0), -1);
        cv::putText(canvas, "S", cv::Point(startPx.x + 8, startPx.y - 8),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
        // 终点（红色 E）
        cv::circle(canvas, endPx, 6, cv::Scalar(0, 0, 255), -1);
        cv::putText(canvas, "E", cv::Point(endPx.x + 8, endPx.y - 8),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);

        std::string outputPath = "E:/work/Car_door_ring_splicing/image/背面打光/260622/sorting_angular_" +
                                 std::to_string(context.getId()) + ".bmp";
        cv::imwrite(outputPath, canvas);
    }
    context.setSortedContour(sortedContour);
    return true;
}

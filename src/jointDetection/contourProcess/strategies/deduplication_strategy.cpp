#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "deduplication_strategy.h"

DeduplicationStrategy::DeduplicationStrategy() {}

bool DeduplicationStrategy::process(ContourData &context) {
    auto contour = context.getSubpixelContour();
    auto deduplicated = ContourFeatureCalculator::removeDuplicatePoints(contour);
    auto downsampled = ContourFeatureCalculator::downsampleByTwo(deduplicated);
    context.setSubpixelContour(downsampled);

    // 调试可视化：读入底图，把去重前后的轮廓画在同一张图上对比
    cv::Mat base = cv::imread("E:/work/Car_door_ring_splicing/image/背面打光/260622/cropped/28984_13220.bmp",
                              cv::IMREAD_COLOR);
    if (!base.empty()) {
        // 亚像素 Point2f -> 整数 Point，便于 OpenCV 绘制
        auto toIntContour = [](const std::vector<cv::Point2f>& src) {
            std::vector<cv::Point> dst;
            dst.reserve(src.size());
            for (const auto& p : src) {
                dst.emplace_back(cvRound(p.x), cvRound(p.y));
            }
            return dst;
        };
        std::vector<cv::Point> contourInt = toIntContour(contour);
        std::vector<cv::Point> deduplicatedInt = toIntContour(deduplicated);
        // 原始轮廓用红色绘制
        if (contourInt.size() >= 2) {
            cv::polylines(base, contourInt, false, cv::Scalar(0, 0, 255), 1);
        }
        // 去重后轮廓用绿色绘制
        // if (deduplicatedInt.size() >= 2) {
        //     cv::polylines(base, deduplicatedInt, false, cv::Scalar(0, 255, 0), 1);
        // }
        // 用 contour id 区分文件，避免多次调用覆盖同一文件
        std::string outputPath = "E:/work/Car_door_ring_splicing/image/背面打光/260622/cropped/28984_13220_dedup_" +
                                 std::to_string(context.getId()) + ".bmp";
        // cv::imwrite(outputPath, base);
    }

    return true;
}

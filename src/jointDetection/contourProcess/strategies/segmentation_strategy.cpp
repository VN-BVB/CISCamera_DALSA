#include "segmentation_strategy.h"

SegmentationStrategy::SegmentationStrategy() {}

bool SegmentationStrategy::process(ContourData& context) {
    auto contour = context.getSortedContour();
    auto cornerPoints = context.getCornerPoints();

    // 按角点索引切分轮廓（内部按 radius=10 剔除每段角点邻域）
    auto segments = ContourSegmenter::splitContourByCorners(contour, cornerPoints);
    context.setSegmentedContours(segments);

    // 用完整排序轮廓的外接矩形中心作为 CCW 排序参考极点
    auto centroid = ContourUtils::calculateCentralPoint(contour);
    auto sortedSegments = ContourSegmenter::sortSegmentsCounterClockwise(segments, centroid);
    context.setSortedSegments(sortedSegments);

    return true;
}

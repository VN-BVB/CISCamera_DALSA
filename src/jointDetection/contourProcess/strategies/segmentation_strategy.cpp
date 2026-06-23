#include "segmentation_strategy.h"

SegmentationStrategy::SegmentationStrategy() {}

bool SegmentationStrategy::process(ContourData& context) {
    auto contour = context.getSortedContour();
    auto cornerPoints = context.getCornerPoints();

    // 移除角点附近点
    auto filteredContour = ContourFeatureCalculator::removePointsNearCorners(contour, cornerPoints);

    // 分割轮廓
    auto segments = ContourSegmenter::segmentContour(filteredContour);
    context.setSegmentedContours(segments);

    // 排序分割轮廓
    auto centroid = ContourUtils::calculateCentralPoint(filteredContour);
    auto sortedSegments = ContourSegmenter::sortSegmentsCounterClockwise(segments, centroid);
    context.setSortedSegments(sortedSegments);

    return true;
}

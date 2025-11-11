#ifndef SEGMENTATION_STRATEGY_H
#define SEGMENTATION_STRATEGY_H

#include "contour_processing_strategy.h"
#include "methods/contour_feature_calculator.h"
#include "methods/contour_segmenter.h"
#include "methods/contour_utils.h"

class SegmentationStrategy : public ContourProcessingStrategy
{
public:
    SegmentationStrategy();
    bool process(ContourData& context) override {
        auto contour = context.getSortedContour();
        auto cornerPoints = context.getCornerPoints();

        // 移除角点附近点
        auto filteredContour = ContourFeatureCalculator::removePointsNearCorners(contour, cornerPoints);

        // 分割轮廓
        auto segments = ContourSegmenter::segmentContour(filteredContour);
        context.setSegmentedContours(segments);

        // 排序分割轮廓
        auto centroid = ContourUtils::calculateCentroid(filteredContour);
        auto sortedSegments = ContourSegmenter::sortSegmentsCounterClockwise(segments, centroid);
        context.setSortedSegments(sortedSegments);

        return true;
    }

    std::string getName() const override { return "SegmentationStrategy"; }
};

#endif // SEGMENTATION_STRATEGY_H

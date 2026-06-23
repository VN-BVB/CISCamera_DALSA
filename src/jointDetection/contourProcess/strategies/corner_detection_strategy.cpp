#include "corner_detection_strategy.h"

CornerDetectionStrategy::CornerDetectionStrategy() {}

bool CornerDetectionStrategy::process(ContourData& context) {
    auto contour = context.getSortedContour();
    auto cornerPoints = ContourFeatureCalculator::detectCornerPoints(contour);
    context.setCornerPoints(cornerPoints);
    return true;
}


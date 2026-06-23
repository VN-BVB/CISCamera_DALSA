#include "deduplication_strategy.h"

DeduplicationStrategy::DeduplicationStrategy() {}

bool DeduplicationStrategy::process(ContourData &context) {
    auto contour = context.getSubpixelContour();
    auto deduplicated = ContourFeatureCalculator::removeDuplicatePoints(contour);
    context.setSubpixelContour(deduplicated);
    return true;
}

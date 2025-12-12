#include "sorting_strategy.h"
#include "src/utils/scoped_timer.h"

SortingStrategy::SortingStrategy() {}

bool SortingStrategy::process(ContourData& context) {
    SCOPED_TIMER("SortingStrategy");
    auto contour = context.getSubpixelContour();
    auto direction = context.getOpeningDirection();
    int startIndex = 0;
    int endIndex = 0;
    auto startPoint = ContourFeatureCalculator::calculateStartPoint(direction, contour);
    startIndex = ContourUtils::findPointIndex(startPoint, contour);
    auto endPoint = ContourFeatureCalculator::calculateEndPoint(direction, contour);
    endIndex = ContourUtils::findPointIndex(endPoint, contour);

    {
        // SCOPED_TIMER("222222");
        auto sortedContour = ContourFeatureCalculator::sortContourByNearestNeighbor(contour, startIndex, endIndex);
        context.setSortedContour(sortedContour);
    }
    return true;
}

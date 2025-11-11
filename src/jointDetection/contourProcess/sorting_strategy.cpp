#include "sorting_strategy.h"

SortingStrategy::SortingStrategy() {}

bool SortingStrategy::process(ContourData& context) {
    auto contour = context.getSubpixelContour();
    auto direction = context.getOpeningDirection();

    auto startPoint = ContourFeatureCalculator::calculateStartPoint(direction, contour);
    int startIndex = ContourUtils::findPointIndex(startPoint, contour);
    auto endPoint = ContourFeatureCalculator::calculateEndPoint(direction, contour);
    int endIndex = ContourUtils::findPointIndex(endPoint, contour);

    auto sortedContour = ContourFeatureCalculator::sortContourByNearestNeighbor(contour, startIndex, endIndex);
    context.setSortedContour(sortedContour);
    return true;
}

#ifndef SORTING_STRATEGY_H
#define SORTING_STRATEGY_H

#include "contour_processing_strategy.h"
#include "methods/contour_feature_calculator.h"
#include "methods/contour_utils.h"

class SortingStrategy : public ContourProcessingStrategy
{
public:
    SortingStrategy();
    bool process(ContourData& context) override {
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

    std::string getName() const override { return "SortingStrategy"; }
};

#endif // SORTING_STRATEGY_H

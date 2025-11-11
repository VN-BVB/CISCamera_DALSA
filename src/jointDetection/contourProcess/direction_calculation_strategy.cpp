#include "direction_calculation_strategy.h"

DirectionCalculationStrategy::DirectionCalculationStrategy() {}

bool DirectionCalculationStrategy::process(ContourData& context) {
    auto contour = context.getSubpixelContour();
    auto direction = ContourFeatureCalculator::calculateOpeningDirection(contour);
    context.setOpeningDirection(direction);
    return true;
}

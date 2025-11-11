#ifndef DIRECTION_CALCULATION_STRATEGY_H
#define DIRECTION_CALCULATION_STRATEGY_H

#include "contour_processing_strategy.h"
#include "methods/contour_feature_calculator.h"

class DirectionCalculationStrategy : public ContourProcessingStrategy
{
public:
    DirectionCalculationStrategy();
    bool process(ContourData& context) override {
        auto contour = context.getSubpixelContour();
        auto direction = ContourFeatureCalculator::calculateOpeningDirection(contour);
        context.setOpeningDirection(direction);
        return true;
    }

    std::string getName() const override { return "DirectionCalculationStrategy"; }
};

#endif // DIRECTION_CALCULATION_STRATEGY_H

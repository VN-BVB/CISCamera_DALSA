#ifndef DIRECTION_CALCULATION_STRATEGY_H
#define DIRECTION_CALCULATION_STRATEGY_H

#include "src/jointDetection/contourProcess/contour_processing_strategy.h"
#include "src/jointDetection/contourProcess/methods/contour_feature_calculator.h"

class DirectionCalculationStrategy : public ContourProcessingStrategy
{
public:
    DirectionCalculationStrategy();
    bool process(ContourData& context) override;

    std::string getName() const override { return "DirectionCalculationStrategy"; }
};

#endif // DIRECTION_CALCULATION_STRATEGY_H

#ifndef CORNER_DETECTION_STRATEGY_H
#define CORNER_DETECTION_STRATEGY_H

#include "contour_processing_strategy.h"
#include "methods/contour_feature_calculator.h"

class CornerDetectionStrategy : public ContourProcessingStrategy
{
public:
    CornerDetectionStrategy();
    bool process(ContourData& context) override;

    std::string getName() const override { return "CornerDetectionStrategy"; }
};

#endif // CORNER_DETECTION_STRATEGY_H

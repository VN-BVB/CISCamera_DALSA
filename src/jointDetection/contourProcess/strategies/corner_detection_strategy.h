#ifndef CORNER_DETECTION_STRATEGY_H
#define CORNER_DETECTION_STRATEGY_H

#include "src/jointDetection/contourProcess/contour_processing_strategy.h"
#include "src/jointDetection/contourProcess/methods/contour_feature_calculator.h"

class CornerDetectionStrategy : public ContourProcessingStrategy
{
public:
    CornerDetectionStrategy();
    bool process(ContourData& context) override;

    std::string getName() const override { return "CornerDetectionStrategy"; }
};

#endif // CORNER_DETECTION_STRATEGY_H

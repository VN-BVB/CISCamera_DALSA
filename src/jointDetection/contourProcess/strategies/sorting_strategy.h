#ifndef SORTING_STRATEGY_H
#define SORTING_STRATEGY_H

#include "src/jointDetection/contourProcess/contour_processing_strategy.h"
#include "src/jointDetection/contourProcess/methods/contour_feature_calculator.h"
#include "src/jointDetection/contourProcess/methods/contour_utils.h"

class SortingStrategy : public ContourProcessingStrategy
{
public:
    SortingStrategy();
    bool process(ContourData& context) override;

    std::string getName() const override { return "SortingStrategy"; }
};

#endif // SORTING_STRATEGY_H

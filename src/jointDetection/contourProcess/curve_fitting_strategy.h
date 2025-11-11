#ifndef CURVE_FITTING_STRATEGY_H
#define CURVE_FITTING_STRATEGY_H

#include "contour_processing_strategy.h"
#include "methods/contour_fitter.h"
#include "methods/contour_utils.h"

class CurveFittingStrategy : public ContourProcessingStrategy
{
public:
    CurveFittingStrategy();
    bool process(ContourData& context) override;

    std::string getName() const override { return "CurveFittingStrategy"; }
};

#endif // CURVE_FITTING_STRATEGY_H

#ifndef DEDUPLICATION_STRATEGY_H
#define DEDUPLICATION_STRATEGY_H

#include "contour_processing_strategy.h"
#include "methods/contour_feature_calculator.h"

/**
 * @brief 轮廓去重策略
 */
class DeduplicationStrategy : public ContourProcessingStrategy
{
public:
    DeduplicationStrategy();
    virtual bool process(ContourData &context) override {
        auto contour = context.getSubpixelContour();
        auto deduplicated = ContourFeatureCalculator::removeDuplicatePoints(contour);
        context.setSubpixelContour(deduplicated);
        return true;
    }

    std::string getName() const override {return "DeduplicationStrategy";}
};

#endif // DEDUPLICATION_STRATEGY_H

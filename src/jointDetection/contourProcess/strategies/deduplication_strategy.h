#ifndef DEDUPLICATION_STRATEGY_H
#define DEDUPLICATION_STRATEGY_H

#include "src/jointDetection/contourProcess/contour_processing_strategy.h"
#include "src/jointDetection/contourProcess/methods/contour_feature_calculator.h"

/**
 * @brief 轮廓去重策略
 */
class DeduplicationStrategy : public ContourProcessingStrategy
{
public:
    DeduplicationStrategy();
    virtual bool process(ContourData &context) override;

    std::string getName() const override {return "DeduplicationStrategy";}
};

#endif // DEDUPLICATION_STRATEGY_H

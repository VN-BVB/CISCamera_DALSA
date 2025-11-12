#ifndef SEGMENTATION_STRATEGY_H
#define SEGMENTATION_STRATEGY_H

#include "src/jointDetection/contourProcess/contour_processing_strategy.h"
#include "src/jointDetection/contourProcess/methods/contour_feature_calculator.h"
#include "src/jointDetection/contourProcess/methods/contour_segmenter.h"
#include "src/jointDetection/contourProcess/methods/contour_utils.h"

class SegmentationStrategy : public ContourProcessingStrategy
{
public:
    SegmentationStrategy();
    bool process(ContourData& context) override;

    std::string getName() const override { return "SegmentationStrategy"; }
};

#endif // SEGMENTATION_STRATEGY_H

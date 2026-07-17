#ifndef CONTOUR_PROCESSING_PIPELINE_H
#define CONTOUR_PROCESSING_PIPELINE_H

#include "contour_processing_strategy.h"

/**
 * @brief 轮廓处理管道 - 管道模式，责任链模式的一种变体
 */
class ContourProcessingPipeline
{
public:
    ContourProcessingPipeline();

    // 添加处理策略
    void addStrategy(ContourProcessingStrategyPtr strategy);

    // 设置处理策略序列
    void setStrategies(const std::vector<ContourProcessingStrategyPtr>& strategies);

    // 执行管道处理
    bool process(ContourData& context);

    // 获取处理结果摘要
    std::string getSummary(const ContourData& context) const;

    // 清空策略
    void clearStrategies();

private:
    std::vector<ContourProcessingStrategyPtr> m_strategies;
};

#endif // CONTOUR_PROCESSING_PIPELINE_H

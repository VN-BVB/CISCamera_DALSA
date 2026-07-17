#ifndef CONTOUR_PROCESSING_STRATEGY_H
#define CONTOUR_PROCESSING_STRATEGY_H

#include <opencv2/opencv.hpp>
#include "methods/contour_data.h"

/**
 * @brief 轮廓处理策略 - 这里有点过渡使用设计模式了，增加了代码量
 */
class ContourProcessingStrategy
{
public:
    ContourProcessingStrategy();
    virtual ~ContourProcessingStrategy() = default;

    /**
     * @brief 处理轮廓数据
     * @param context 处理上下文
     * @return 是否处理成功
     */
    virtual bool process(ContourData& context) = 0;

    /**
     * @brief 获取策略名称
     */
    virtual std::string getName() const = 0;
};

using ContourProcessingStrategyPtr = std::shared_ptr<ContourProcessingStrategy>;

#endif // CONTOUR_PROCESSING_STRATEGY_H

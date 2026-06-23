#ifndef WORKPIECE_ROI_MAPPER_H
#define WORKPIECE_ROI_MAPPER_H

#include <map>
#include "src/jointDetection/image_process_worker.h"

class WorkpieceRoiMapper
{
public:
    // 构建工件与ROI之间的映射关系
    // 将EdgeAssembly的组合结果转换为工件到processedRoiInfos的映射
    static std::map<int, std::vector<int>> buildWorkpieceRoiMapping(
        const std::map<int, std::vector<int>>& combinationResult,
        const std::map<int, ProcessedROIInfo>& processedRoiInfos);
};

#endif // WORKPIECE_ROI_MAPPER_H
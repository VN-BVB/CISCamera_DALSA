#ifndef WORKPIECE_PLATFORM_MAPPER_H
#define WORKPIECE_PLATFORM_MAPPER_H

#include <map>
#include <vector>
#include <Eigen/Core>

#include "src/jointDetection/image_process_worker.h"
#include "src/utils/platform_pose_io.h"

// 工件 → 对位平台 的映射构造器。
// 与 WorkpieceRoiMapper 平行：仅静态方法、无状态。
class WorkpiecePlatformMapper
{
public:
    // 计算每个工件的外接矩形中心（世界系，毫米）。
    // 外接矩形由工件下所有缝隙端点的世界坐标求 AABB 得到。
    // 没有端点的工件被跳过（写入 warning 日志）。
    static std::map<int, Eigen::Vector2d> computeWorkpieceCenters(
        const std::map<int, std::vector<int>>& combinationResult,
        const std::map<int, ProcessedROIInfo>& processedRoiInfos);

    // 给定工件中心 + 全部平台，按 2D 欧氏距离最近原则建立 workpieceId → platformId。
    // platforms 为空时返回空 map。
    static std::map<int, int> buildMapping(
        const std::map<int, Eigen::Vector2d>& workpieceCenters,
        const std::vector<PlatformAxis>& platforms);
};

#endif  // WORKPIECE_PLATFORM_MAPPER_H

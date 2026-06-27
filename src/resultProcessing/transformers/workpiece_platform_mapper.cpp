#include "workpiece_platform_mapper.h"

#include <cmath>
#include <limits>
#include <unordered_set>
#include <plog/Log.h>

#include "src/utils/geometry_utils.h"

std::map<int, Eigen::Vector2d> WorkpiecePlatformMapper::computeWorkpieceCenters(
    const std::map<int, std::vector<int>>& combinationResult,
    const std::map<int, ProcessedROIInfo>& processedRoiInfos)
{
    PLOG_INFO << "开始计算工件外接矩形中心（基于缝隙端点）";

    std::map<int, Eigen::Vector2d> centers;

    for (const auto& [workpieceId, contourIds] : combinationResult) {
        std::unordered_set<int> contourIdSet(contourIds.begin(), contourIds.end());

        // 收集属于该工件所有轮廓的端点像素坐标
        std::vector<cv::Point2f> pixelPoints;
        for (const auto& [roiKey, roiInfo] : processedRoiInfos) {
            for (const auto& ep : roiInfo.endPoints) {
                if (contourIdSet.count(ep.contourId)) {
                    pixelPoints.push_back(ep.coordinates);
                }
            }
        }

        if (pixelPoints.empty()) {
            PLOG_WARNING << "工件 " << workpieceId << " 没有端点数据，跳过中心计算";
            continue;
        }

        // 端点坐标已是全图像素坐标（与 dxf_saver.cpp / json_transformer.cpp 中的约定一致）
        std::vector<Eigen::Vector2d> worldPoints = GeometryUtils::pixel2World(pixelPoints);
        if (worldPoints.empty()) {
            PLOG_WARNING << "工件 " << workpieceId << " pixel2World 失败，跳过";
            continue;
        }

        // 求世界系 AABB 中心
        double minX = std::numeric_limits<double>::max();
        double minY = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double maxY = std::numeric_limits<double>::lowest();
        for (const auto& wp : worldPoints) {
            if (wp.x() < minX) minX = wp.x();
            if (wp.y() < minY) minY = wp.y();
            if (wp.x() > maxX) maxX = wp.x();
            if (wp.y() > maxY) maxY = wp.y();
        }

        Eigen::Vector2d center(0.5 * (minX + maxX), 0.5 * (minY + maxY));
        centers[workpieceId] = center;
        PLOG_INFO << "工件 " << workpieceId << " 中心 (世界系): ("
                  << center.x() << ", " << center.y() << ")";
    }

    PLOG_INFO << "工件外接矩形中心计算完成，共 " << centers.size() << " 个工件";
    return centers;
}

std::map<int, int> WorkpiecePlatformMapper::buildMapping(
    const std::map<int, Eigen::Vector2d>& workpieceCenters,
    const std::vector<PlatformAxis>& platforms)
{
    std::map<int, int> mapping;

    if (platforms.empty()) {
        PLOG_WARNING << "平台列表为空，无法建立工件→平台映射";
        return mapping;
    }

    for (const auto& [workpieceId, center] : workpieceCenters) {
        double bestDist = std::numeric_limits<double>::max();
        int bestPlatformId = -1;

        for (const auto& pa : platforms) {
            // 平台原点 T 的 z 分量为 0，按 2D 欧氏距离比较即可
            double dx = pa.T.x() - center.x();
            double dy = pa.T.y() - center.y();
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist < bestDist) {
                bestDist = dist;
                bestPlatformId = pa.id;
            }
        }

        if (bestPlatformId >= 0) {
            mapping[workpieceId] = bestPlatformId;
            PLOG_INFO << "工件 " << workpieceId << " 关联平台 " << bestPlatformId
                      << "（距离 " << bestDist << " mm）";
        }
    }

    return mapping;
}

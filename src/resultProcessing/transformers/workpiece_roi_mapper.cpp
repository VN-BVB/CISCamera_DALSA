#include "workpiece_roi_mapper.h"
#include <plog/Log.h>

std::map<int, std::vector<int>> WorkpieceRoiMapper::buildWorkpieceRoiMapping(
    const std::map<int, std::vector<int>>& combinationResult,
    const std::map<int, ProcessedROIInfo>& processedRoiInfos)
{
    PLOG_INFO << "开始构建工件与ROI的映射关系";

    std::map<int, std::vector<int>> workpieceToRoiInfos;

    for (const auto& [workpieceId, contourIds] : combinationResult) {
        std::vector<int> relatedRoiInfos;

        // 在processedRoiInfos中查找包含对应轮廓ID的processedROIInfo
        for (const auto& [roiKey, roiInfo] : processedRoiInfos) {
            for (const auto& contourId : contourIds) {
                bool found = false;
                for (const auto& contourData : roiInfo.contourDatas) {
                    if (contourData.getId() == contourId) {
                        relatedRoiInfos.push_back(roiInfo.index);
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
        }

        workpieceToRoiInfos[workpieceId] = relatedRoiInfos;
        PLOG_INFO << "工件 " << workpieceId << " 关联的ROI: "
                  << [relatedRoiInfos]() {
                        QString str;
                        for (int roi : relatedRoiInfos) {
                            str += QString::number(roi) + " ";
                        }
                        return str;
                    }().toStdString();
    }

    PLOG_INFO << "工件与ROI映射关系构建完成";
    return workpieceToRoiInfos;
}
#include "result_processor.h"
#include <plog/Log.h>

ResultProcessor::ResultProcessor(QObject *parent)
    : QObject(parent), m_dxfSaver(std::make_shared<DXFSaver>())
{
}

void ResultProcessor::whenEdgeAssemblyFinished(const std::map<int, std::vector<int>>& combinationResult,
                                               const std::map<int, ProcessedROIInfo>& processedRoiInfos)
{
    PLOG_INFO << "ResultProcessor: 接收到EdgeAssembly完成信号";
    PLOG_INFO << "工件数量: " << combinationResult.size();
    PLOG_INFO << "ROI数量: " << processedRoiInfos.size();

    try {
        // 1. 使用transformer进行数据转换：构建工件与ROI的映射关系
        auto workpieceToRoiInfos = WorkpieceRoiMapper::buildWorkpieceRoiMapping(combinationResult, processedRoiInfos);

        // 2. 调用output模块保存DXF文件
        m_dxfSaver->whenAllImagesProcessed(workpieceToRoiInfos, processedRoiInfos);

        PLOG_INFO << "处理完成";
    } catch (const std::exception& e) {
        PLOG_ERROR << "处理时出错: " << e.what();
    }
}

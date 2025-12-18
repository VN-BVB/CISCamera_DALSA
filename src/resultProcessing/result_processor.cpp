#include <plog/Log.h>

#include "result_processor.h"
#include "src/utils/scoped_timer.h"

ResultProcessor::ResultProcessor(QObject *parent)
    : QObject(parent), m_dxfSaver(std::make_shared<DXFSaver>()),
      m_jsonTransformer(nullptr), 
      m_jsonSender(std::make_unique<JsonSender>()),
      m_jsonSaver(std::make_unique<JsonSaver>())
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

        SCOPED_TIMER("保存json数据");
        // 3. 创建JsonTransformer并生成JSON数据
        m_jsonTransformer = std::make_unique<JsonTransformer>(processedRoiInfos);
        std::string jsonString = m_jsonTransformer->generateJson(combinationResult, 0);

        // 4. 发送JSON到共享内存
        m_jsonSender->sendJsonWithLogging(jsonString, 0);

        // 5. 保存JSON到文件作为备份
        m_jsonSaver->saveJsonToFile(jsonString, 0);

        PLOG_INFO << "处理完成（DXF + JSON）";
    } catch (const std::exception& e) {
        PLOG_ERROR << "处理时出错: " << e.what();
    }
}



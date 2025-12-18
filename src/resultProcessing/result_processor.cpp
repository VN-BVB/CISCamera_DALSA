#include <plog/Log.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "result_processor.h"
#include "src/utils/scoped_timer.h"

ResultProcessor::ResultProcessor(QObject *parent)
    : QObject(parent), m_dxfSaver(std::make_shared<DXFSaver>()), m_jsonTransformer(nullptr)
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

        // 4. 保存JSON到文件
        saveJsonToFile(jsonString, 0);

        PLOG_INFO << "处理完成（DXF + JSON）";
    } catch (const std::exception& e) {
        PLOG_ERROR << "处理时出错: " << e.what();
    }
}

void ResultProcessor::saveJsonToFile(const std::string& jsonString, int batchNumber)
{
    try {
        // 生成文件名
        std::string fileName = "./data/seamEndpointInfos/seam_result_batch_" + std::to_string(batchNumber) + ".json";

        // 确保目录存在
        std::filesystem::path filePath(fileName);
        std::filesystem::path dirPath = filePath.parent_path();

        if (!std::filesystem::exists(dirPath)) {
            std::error_code ec;
            if (std::filesystem::create_directories(dirPath, ec)) {
                PLOG_INFO << "成功创建目录: " << dirPath.string();
            } else {
                PLOG_ERROR << "无法创建目录 " << dirPath.string() << ": " << ec.message();
                return;
            }
        }

        // 打开文件进行写入
        std::ofstream jsonFile(fileName);
        if (!jsonFile.is_open()) {
            PLOG_ERROR << "无法打开JSON文件进行写入: " << fileName;
            return;
        }

        // 写入JSON内容
        jsonFile << jsonString;

        // 确保缓冲区被刷新
        jsonFile.flush();
        jsonFile.close();

        // std::cout << "\n=== JSON Output ===\n" << jsonString << "\n==================\n" << std::endl;
    } catch (const std::exception& e) {
        PLOG_ERROR << "保存JSON文件时出错: " << e.what();
    }
}

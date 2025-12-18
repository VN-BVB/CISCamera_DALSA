#ifndef JSON_TRANSFORMER_H
#define JSON_TRANSFORMER_H

#include <map>
#include <string>
#include <sstream>
#include "src/jointDetection/image_process_worker.h"
#include "json_data_structure.h"
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

class JsonTransformer
{
public:
    /**
     * @brief 构造函数，传入共享数据
     * @param processedRoiInfos ROI处理结果数据
     */
    explicit JsonTransformer(const std::map<int, ProcessedROIInfo>& processedRoiInfos);

    /**
     * @brief 将处理结果转换为批次结果格式
     * @param combinationResult 工件组合结果：工件ID → 轮廓ID列表
     * @param batchNumber 批次号
     * @return BatchResultData 结构化数据
     */
    BatchResultData transformToBatchResultData(
        const std::map<int, std::vector<int>>& combinationResult,
        int batchNumber = 0);

    /**
     * @brief 将BatchResultData序列化为JSON字符串
     * @param batchData 批次结果数据
     * @return JSON字符串
     */
    std::string serializeToJson(const BatchResultData& batchData);

    /**
     * @brief 直接生成JSON字符串（一步完成）
     */
    std::string generateJson(
        const std::map<int, std::vector<int>>& combinationResult,
        int batchNumber = 0);

private:
    // 共享数据成员，避免重复传递
    const std::map<int, ProcessedROIInfo>& m_processedRoiInfos;
    std::vector<SeamEndpoint> m_allEndpoints;  // 从processedRoiInfos中解析出来
    std::map<int, int> m_contourToWorkpieceMapping;

    /**
     * @brief 从processedRoiInfos中提取所有端点数据
     */
    void extractAllEndpoints();

    /**
     * @brief 构建轮廓ID到工件ID的映射
     */
    void buildContourToWorkpieceMapping(
        const std::map<int, std::vector<int>>& combinationResult);

    /**
     * @brief 创建工件信息
     */
    WorkpieceInfo createWorkpieceInfo(int workpieceId, const std::vector<int>& contourIds);

    /**
     * @brief 创建边（轮廓）信息
     */
    EdgeInfo createEdgeInfo(const ContourData& contourData);

    /**
     * @brief 创建端点信息
     */
    EndpointInfo createEndpointInfo(const SeamEndpoint& endpoint);
};

#endif // JSON_TRANSFORMER_H

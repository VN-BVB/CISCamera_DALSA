#include "json_transformer.h"
#include <plog/Log.h>

JsonTransformer::JsonTransformer(const std::map<int, ProcessedROIInfo>& processedRoiInfos)
    : m_processedRoiInfos(processedRoiInfos)
{
    // 构造时提取端点数据
    extractAllEndpoints();
    PLOG_INFO << "JsonTransformer构造完成，提取到 " << m_allEndpoints.size() << " 个端点";
}

void JsonTransformer::extractAllEndpoints()
{
    PLOG_INFO << "从processedRoiInfos中提取所有端点数据";

    m_allEndpoints.clear();
    for (const auto& [roiKey, roiInfo] : m_processedRoiInfos) {
        for (const auto& endpoint : roiInfo.endPoints) {
            m_allEndpoints.push_back(endpoint);
        }
    }

    PLOG_INFO << "提取完成，共 " << m_allEndpoints.size() << " 个端点";
}

void JsonTransformer::buildContourToWorkpieceMapping(
    const std::map<int, std::vector<int>>& combinationResult)
{
    PLOG_INFO << "开始构建轮廓ID到工件ID的映射关系";

    m_contourToWorkpieceMapping.clear();
    // 遍历每个工件
    for (const auto& [workpieceId, contourIds] : combinationResult) {
        // 遍历工件中的每个轮廓
        for (int contourId : contourIds) {
            m_contourToWorkpieceMapping[contourId] = workpieceId;
            PLOG_DEBUG << "轮廓 " << contourId << " → 工件 " << workpieceId;
        }
    }
    PLOG_INFO << "轮廓-工件映射构建完成，共映射 " << m_contourToWorkpieceMapping.size() << " 个轮廓";
}

EndpointInfo JsonTransformer::createEndpointInfo(const SeamEndpoint& endpoint)
{
    PLOG_DEBUG << "创建端点信息，端点ID: " << endpoint.id << "，轮廓ID: " << endpoint.contourId;

    EndpointInfo endpointInfo;

    // 1. 点序号：直接使用SeamEndpoint中的ID
    endpointInfo.pointId = endpoint.id;

    // 2. 坐标：转换为double类型
    endpointInfo.coordinates = {
        static_cast<double>(endpoint.coordinates.x),
        static_cast<double>(endpoint.coordinates.y)
    };

    // 3. 对应点的序号
    endpointInfo.correspondingPointId = endpoint.correspondingIntersectionId;

    // 4. 对应点所属工件的序号：查找对应点的contourId
    endpointInfo.correspondingWorkpieceId = -1;  // 默认值

    // 查找对应点
    for (const auto& targetEndpoint : m_allEndpoints) {
        if (targetEndpoint.id == endpoint.correspondingIntersectionId) {
            // 找到对应点，获取其contourId
            int correspondingContourId = targetEndpoint.contourId;

            // 通过contourToWorkpieceMapping查找对应工件
            auto it = m_contourToWorkpieceMapping.find(correspondingContourId);
            if (it != m_contourToWorkpieceMapping.end()) {
                // 工件序号直接使用映射值（已经从0开始）
                endpointInfo.correspondingWorkpieceId = it->second;
                PLOG_DEBUG << "端点 " << endpointInfo.pointId << " 的对应点 " << endpointInfo.correspondingPointId
                          << " 属于轮廓 " << correspondingContourId << "，工件序号 " << endpointInfo.correspondingWorkpieceId;
            } else {
                PLOG_WARNING << "未找到对应点轮廓ID " << correspondingContourId << " 所属的工件";
            }
            break;
        }
    }

    if (endpointInfo.correspondingWorkpieceId == -1) {
        PLOG_WARNING << "未找到端点 " << endpointInfo.pointId << " 的对应点 " << endpointInfo.correspondingPointId;
    }

    return endpointInfo;
}

EdgeInfo JsonTransformer::createEdgeInfo(const ContourData& contourData)
{
    PLOG_DEBUG << "创建边信息，轮廓ID: " << contourData.getId();

    EdgeInfo edgeInfo;
    edgeInfo.edgeId = contourData.getId();

    // 获取轮廓的交点（端点）数据
    const auto& intersections = contourData.getIntersections();

    if (intersections.size() >= 2) {
        // 处理端点0
        for (const auto& seamEndpoint : m_allEndpoints) {
            if (seamEndpoint.id == intersections[0].id) {
                EndpointInfo endpoint0 = createEndpointInfo(seamEndpoint);
                edgeInfo.endpoints["端点0"] = endpoint0;
                break;
            }
        }

        // 处理端点1
        for (const auto& seamEndpoint : m_allEndpoints) {
            if (seamEndpoint.id == intersections[1].id) {
                EndpointInfo endpoint1 = createEndpointInfo(seamEndpoint);
                edgeInfo.endpoints["端点1"] = endpoint1;
                break;
            }
        }

        PLOG_DEBUG << "边 " << contourData.getId() << " 创建完成，包含 " << edgeInfo.endpoints.size() << " 个端点";
    } else {
        PLOG_WARNING << "轮廓ID " << contourData.getId() << " 的端点数据不完整，只有 "
                    << intersections.size() << " 个交点";
    }

    return edgeInfo;
}

WorkpieceInfo JsonTransformer::createWorkpieceInfo(int workpieceId, const std::vector<int>& contourIds)
{
    PLOG_DEBUG << "创建工件信息，工件ID: " << workpieceId << "，包含轮廓数量: " << contourIds.size();

    WorkpieceInfo workpieceInfo;

    // 为每条轮廓（边）创建信息
    for (size_t i = 0; i < contourIds.size(); ++i) {
        int contourId = contourIds[i];
        std::string edgeKey = "边" + std::to_string(i);  // 边0, 边1, 边2...

        // 查找包含该轮廓的ROI
        const ContourData* targetContour = nullptr;
        for (const auto& [roiKey, roiInfo] : m_processedRoiInfos) {
            for (const auto& contourData : roiInfo.contourDatas) {
                if (contourData.getId() == contourId) {
                    targetContour = &contourData;
                    break;
                }
            }
            if (targetContour) break;
        }

        if (targetContour) {
            EdgeInfo edgeInfo = createEdgeInfo(*targetContour);
            workpieceInfo.edges[edgeKey] = edgeInfo;

            PLOG_DEBUG << "工件 " << workpieceId << " 创建边 " << edgeKey
                      << "（轮廓ID: " << contourId << "），包含 "
                      << edgeInfo.endpoints.size() << " 个端点";
        } else {
            PLOG_WARNING << "未找到轮廓ID " << contourId << " 的数据";
        }
    }

    PLOG_DEBUG << "工件 " << workpieceId << " 创建完成，包含 "
              << workpieceInfo.edges.size() << " 条边";
    return workpieceInfo;
}

BatchResultData JsonTransformer::transformToBatchResultData(
    const std::map<int, std::vector<int>>& combinationResult,
    int batchNumber)
{
    PLOG_INFO << "开始转换数据为批次结果格式，批次号: " << batchNumber;

    BatchResultData batchData;
    batchData.batchNumber = batchNumber;

    // 1. 构建轮廓ID到工件ID的映射关系
    buildContourToWorkpieceMapping(combinationResult);

    // 2. 为每个工件创建信息
    for (const auto& [workpieceId, contourIds] : combinationResult) {
        // 将工件ID从1开始映射为从0开始的索引：工件1 → "工件0", 工件2 → "工件1"
        std::string workpieceKey = "工件" + std::to_string(workpieceId - 1);

        WorkpieceInfo workpieceInfo = createWorkpieceInfo(workpieceId, contourIds);
        batchData.workpieces[workpieceKey] = workpieceInfo;

        PLOG_INFO << "创建工件 " << workpieceKey << "，包含 " << contourIds.size() << " 条边";
    }

    PLOG_INFO << "批次结果数据转换完成，包含 " << batchData.workpieces.size() << " 个工件";
    return batchData;
}

std::string JsonTransformer::serializeToJson(const BatchResultData& batchData)
{
    PLOG_INFO << "开始序列化为JSON字符串";

    std::stringstream jsonStream;
    try {
        // 确保archive在复制字符串前完成析构
        {
            cereal::JSONOutputArchive archive(jsonStream);
            archive(batchData);
        } // archive在这里析构，确保序列化完成

        // 确保流被刷新
        jsonStream.flush();

        std::string jsonString = jsonStream.str();
        PLOG_INFO << "JSON序列化完成，字符串长度: " << jsonString.length();
        PLOG_DEBUG << "JSON内容预览: " << jsonString.substr(0, 200) << "...";

        return jsonString;
    } catch (const std::exception& e) {
        PLOG_ERROR << "JSON序列化失败: " << e.what();
        return "{}";
    }
}

std::string JsonTransformer::generateJson(
    const std::map<int, std::vector<int>>& combinationResult,
    int batchNumber)
{
    PLOG_INFO << "直接生成JSON字符串，批次号: " << batchNumber;

    // 1. 转换为批次结果数据
    BatchResultData batchData = transformToBatchResultData(combinationResult, batchNumber);

    // 2. 序列化为JSON字符串
    return serializeToJson(batchData);
}

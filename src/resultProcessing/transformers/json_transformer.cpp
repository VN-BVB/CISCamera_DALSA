#include "json_transformer.h"
#include <plog/Log.h>

/**
 * @brief JsonTransformer 构造函数
 * @param processedRoiInfos ROI处理结果数据，包含轮廓信息、端点信息等
 */
JsonTransformer::JsonTransformer(const std::map<int, ProcessedROIInfo>& processedRoiInfos)
    : m_processedRoiInfos(processedRoiInfos)
{
    // 构造时提取端点数据
    extractAllEndpoints();
}

/**
 * @brief 从processedRoiInfos中提取所有端点数据
 * 遍历所有ROI的端点信息，存储到m_allEndpoints中供后续处理使用
 */
void JsonTransformer::extractAllEndpoints()
{
    m_allEndpoints.clear();
    for (const auto& [roiKey, roiInfo] : m_processedRoiInfos) {
        for (const auto& endpoint : roiInfo.endPoints) {
            m_allEndpoints.push_back(endpoint);
        }
    }
}

/**
 * @brief 构建轮廓ID到工件ID的映射
 * @param combinationResult 工件组合结果：工件ID → 轮廓ID列表
 * @details 根据工件组合结果，建立轮廓ID到工件ID的映射关系
 *          用于后续查找端点对应的工件信息
 */
void JsonTransformer::buildContourToWorkpieceMapping(
    const std::map<int, std::vector<int>>& combinationResult)
{
    m_contourToWorkpieceMapping.clear();
    // 遍历每个工件
    for (const auto& [workpieceId, contourIds] : combinationResult) {
        // 遍历工件中的每个轮廓
        for (int contourId : contourIds) {
            m_contourToWorkpieceMapping[contourId] = workpieceId;
        }
    }
}

/**
 * @brief 创建端点信息
 * @param endpoint 原始端点数据结构
 * @return 转换后的端点信息结构
 * @details 将SeamEndpoint转换为JSON序列化所需的EndpointInfo格式
 *          包含点序号、坐标、对应点信息等，并查找对应点所属的工件
 */
EndpointInfo JsonTransformer::createEndpointInfo(const SeamEndpoint& endpoint)
{
    EndpointInfo endpointInfo;

    // 1. 点序号：直接使用SeamEndpoint中的ID
    endpointInfo.pointId = endpoint.id;

    // 2. 坐标：转换为世界坐标
    std::vector<cv::Point2f> pixelPoints = {endpoint.coordinates};
    std::vector<Eigen::Vector2d> worldPoints = GeometryUtils::pixel2World(pixelPoints);
    if (!worldPoints.empty()) {
        endpointInfo.coordinates = {
            worldPoints[0].x(),
            worldPoints[0].y()
        };
    } 
    else {
        endpointInfo.coordinates = {
            static_cast<double>(endpoint.coordinates.x),
            static_cast<double>(endpoint.coordinates.y)
        };
        PLOG_WARNING << "端点 " << endpoint.id << " 坐标转换失败，使用像素坐标";
    }

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
                endpointInfo.correspondingWorkpieceId = it->second;
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

/**
 * @brief 创建边（轮廓）信息
 * @param contourData 轮廓数据，包含轮廓ID和交点信息
 * @return 转换后的边信息结构
 * @details 将ContourData转换为JSON序列化所需的EdgeInfo格式
 *          根据轮廓的交点信息查找对应的端点数据，构建边的信息
 */
EdgeInfo JsonTransformer::createEdgeInfo(const ContourData& contourData)
{
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
    } else {
        PLOG_WARNING << "轮廓ID " << contourData.getId() << " 的端点数据不完整，只有 "
                     << intersections.size() << " 个交点";
    }

    return edgeInfo;
}

/**
 * @brief 创建工件信息
 * @param workpieceId 工件ID
 * @param contourIds 该工件包含的轮廓ID列表
 * @return 转换后的工件信息结构
 * @details 根据工件ID和轮廓列表，从processedRoiInfos中查找对应的轮廓数据
 *          为每个轮廓创建边信息，构建完整的工件数据结构
 */
WorkpieceInfo JsonTransformer::createWorkpieceInfo(int workpieceId, const std::vector<int>& contourIds)
{
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
        } else {
            PLOG_WARNING << "未找到轮廓ID " << contourId << " 的数据";
        }
    }
    return workpieceInfo;
}

/**
 * @brief 将处理结果转换为批次结果格式
 * @param combinationResult 工件组合结果：工件ID → 轮廓ID列表
 * @param batchNumber 批次号，默认为0
 * @return BatchResultData 结构化数据
 * @details 核心转换函数，将图像处理的原始结果转换为JSON序列化所需的格式
 *          包含构建轮廓-工件映射、创建工件信息等步骤
 */
BatchResultData JsonTransformer::transformToBatchResultData(const std::map<int, std::vector<int>>& combinationResult,
                                                            int batchNumber)
{
    BatchResultData batchData;
    batchData.batchNumber = batchNumber;

    // 1. 构建轮廓ID到工件ID的映射关系
    buildContourToWorkpieceMapping(combinationResult);

    // 2. 为每个工件创建信息
    for (const auto& [workpieceId, contourIds] : combinationResult) {
        std::string workpieceKey = "工件" + std::to_string(workpieceId);
        WorkpieceInfo workpieceInfo = createWorkpieceInfo(workpieceId, contourIds);
        batchData.workpieces[workpieceKey] = workpieceInfo;
    }

    return batchData;
}

/**
 * @brief 将BatchResultData序列化为JSON字符串
 * @param batchData 批次结果数据
 * @return 格式化的JSON字符串
 */
std::string JsonTransformer::serializeToJson(const BatchResultData& batchData)
{
    std::stringstream jsonStream;
    try {
        // 手动序列化，避免value0包装层
        {
            cereal::JSONOutputArchive archive(jsonStream);
            // 序列化批次号
            archive(cereal::make_nvp("批次号", batchData.batchNumber));
            // 序列化每个工件
            for (const auto& [workpieceKey, workpieceInfo] : batchData.workpieces) {
                archive(cereal::make_nvp(workpieceKey, workpieceInfo));
            }
        }

        // 确保流被刷新
        jsonStream.flush();

        std::string jsonString = jsonStream.str();
        return jsonString;
    } catch (const std::exception& e) {
        PLOG_ERROR << "JSON序列化失败: " << e.what();
        return "{}";
    }
}

/**
 * @brief 直接生成JSON字符串（一步完成）
 * @param combinationResult 工件组合结果：工件ID → 轮廓ID列表
 * @param batchNumber 批次号，默认为0
 * @return 格式化的JSON字符串
 * @details 对外的便捷接口，一步完成数据转换和JSON序列化
 */
std::string JsonTransformer::generateJson(
    const std::map<int, std::vector<int>>& combinationResult,
    int batchNumber)
{
    // 1. 转换为批次结果数据
    BatchResultData batchData = transformToBatchResultData(combinationResult, batchNumber);

    // 2. 序列化为JSON字符串
    return serializeToJson(batchData);
}

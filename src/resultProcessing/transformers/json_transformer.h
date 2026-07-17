#ifndef JSON_TRANSFORMER_H
#define JSON_TRANSFORMER_H

#include <map>
#include <string>
#include <sstream>
#include "src/jointDetection/image_process_worker.h"
#include "src/utils/geometry_utils.h"
#include "json_data_structure.h"
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

class JsonTransformer
{
public:
    explicit JsonTransformer(const std::map<int, ProcessedROIInfo>& processedRoiInfos);

    BatchResultData transformToBatchResultData(const std::map<int, std::vector<int>>& combinationResult,
                                               const std::map<int, int>& workpieceToPlatform,
                                               int batchNumber = 0);
    std::string serializeToJson(const BatchResultData& batchData);
    std::string generateJson(const std::map<int, std::vector<int>>& combinationResult,
                             const std::map<int, int>& workpieceToPlatform,
                             int batchNumber = 0);

private:
    void extractAllEndpoints();
    void buildContourToWorkpieceMapping(const std::map<int, std::vector<int>>& combinationResult);
    WorkpieceInfo createWorkpieceInfo(int workpieceId, const std::vector<int>& contourIds,
                                      const std::map<int, int>& workpieceToPlatform);
    EdgeInfo createEdgeInfo(const ContourData& contourData);
    EndpointInfo createEndpointInfo(const SeamEndpoint& endpoint);

    const std::map<int, ProcessedROIInfo>& m_processedRoiInfos;
    std::vector<SeamEndpoint> m_allEndpoints;                   // 从processedRoiInfos中解析出来
    std::map<int, int> m_contourToWorkpieceMapping;
    std::map<int, int> m_oldToNewWorkpieceId;                   // 旧工件ID → 新工件ID（= 对位平台ID）
};
#endif // JSON_TRANSFORMER_H

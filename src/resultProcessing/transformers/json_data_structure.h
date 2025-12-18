#ifndef JSON_DATA_STRUCTURE_H
#define JSON_DATA_STRUCTURE_H

#include <vector>
#include <map>
#include <string>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

/**
 * @brief 端点信息结构
 * 对应JSON中的端点对象
 */
struct EndpointInfo {
    int pointId;                        // 点序号
    std::vector<double> coordinates;    // 坐标 [x, y]
    int correspondingPointId;           // 对应点的序号
    int correspondingWorkpieceId;       // 对应点所属工件的序号

    // cereal序列化支持
    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(pointId),
           CEREAL_NVP(coordinates),
           CEREAL_NVP(correspondingPointId),
           CEREAL_NVP(correspondingWorkpieceId));
    }
};

/**
 * @brief 边（轮廓）信息结构
 * 对应JSON中的边对象
 */
struct EdgeInfo {
    int edgeId;                                           // 边序号
    std::map<std::string, EndpointInfo> endpoints;        // 端点信息 "端点0", "端点1"

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(edgeId),
           CEREAL_NVP(endpoints));
    }
};

/**
 * @brief 工件信息结构
 * 对应JSON中的工件对象
 */
struct WorkpieceInfo {
    std::map<std::string, EdgeInfo> edges;    // 边信息 "边0", "边1", "边2"...

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(edges));
    }
};

/**
 * @brief 批次结果数据结构
 * 对应整个JSON文档
 */
struct BatchResultData {
    int batchNumber;                                          // 批次号
    std::map<std::string, WorkpieceInfo> workpieces;         // 工件信息 "工件0", "工件1"...

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(batchNumber),
           CEREAL_NVP(workpieces));
    }
};

#endif // JSON_DATA_STRUCTURE_H
#include <iostream>
#include <QString>

#include <plog/Log.h>

#include "workpiece_generator.h"
#include "edge_assembly.h"
#include "src/utils/geometry_utils.h"

WorkpieceGenerator::WorkpieceGenerator(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs) : m_cbbs(cbbs)
{}

/**
 * @brief 检查组合是否合法
 * @param workpiece 工件组合
 * @return 是否合法
 */
bool WorkpieceGenerator::isLegalCombination(const WorkpieceBoundingBox& workpiece) {
    for (const auto& candidate : m_cbbs) {
        if (!workpiece.isLegal(candidate)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 计算轮廓之间的距离矩阵
 * @param unpairedContours 未配对的轮廓字典（ID → CBB）
 * @return 距离矩阵（pair<ID1,ID2> → distance）
 */
DistanceMatrix WorkpieceGenerator::calculateDistanceMatrix(
    const UnpairedContoursMap& unpairedContours) {
    DistanceMatrix matrix;

    for (auto it1 = unpairedContours.begin(); it1 != unpairedContours.end(); ++it1) {
        cv::Point2f center1 = it1->second->getCenterPoint();
        for (auto it2 = std::next(it1); it2 != unpairedContours.end(); ++it2) {
            cv::Point2f center2 = it2->second->getCenterPoint();
            float dx = center2.x - center1.x;
            float dy = center2.y - center1.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            // 存储双向距离
            matrix[{it1->first, it2->first}] = distance;
            matrix[{it2->first, it1->first}] = distance;
        }
    }
    return matrix;
}

/**
 * @brief 为每个轮廓创建距离排序的索引列表
 * @param unpairedContours 未配对的轮廓字典（ID → CBB）
 * @param distanceMatrix 距离矩阵
 * @return 每个轮廓ID的最近邻ID列表
 */
NearestIndicesMap WorkpieceGenerator::createNearestIndices(
    const UnpairedContoursMap& unpairedContours,
    const DistanceMatrix& distanceMatrix) {
    NearestIndicesMap nearestIndices;

    for (const auto& [id1, _] : unpairedContours) {
        std::vector<std::pair<float, int>> distances;
        for (const auto& [id2, __] : unpairedContours) {
            if (id1 != id2) {
                distances.emplace_back(distanceMatrix.at({id1, id2}), id2);
            }
        }
        // 按距离从小到大排序
        std::sort(distances.begin(), distances.end());
        for (const auto& [dist, id] : distances) {
            nearestIndices[id1].push_back(id);
        }
    }
    return nearestIndices;
}

/**
 * @brief 根据开口方向对候选轮廓进行分区
 * @param currentId 当前轮廓ID
 * @param currentDirection 当前轮廓开口方向
 * @param currentCenter 当前轮廓中心点
 * @param candidateIds 候选轮廓ID列表
 * @param unpairedContours 未配对的轮廓字典（ID → CBB）
 * @return 分区后的ID列表（优先区域和其他区域）
 */
std::pair<std::vector<int>, std::vector<int>>
WorkpieceGenerator::partitionCandidatesByDirection(int currentId,
                                                   OpeningDirection currentDirection,
                                                   const cv::Point2f& currentCenter,
                                                   const std::vector<int>& candidateIds,
                                                   const UnpairedContoursMap& unpairedContours) {

    std::vector<int> preferredIds;
    std::vector<int> otherIds;

    for (int candidateId : candidateIds) {
        if (currentId >= candidateId) continue; // 避免重复组合

        cv::Point2f candidateCenter = unpairedContours.at(candidateId)->getCenterPoint();
        float dx = candidateCenter.x - currentCenter.x;
        float dy = candidateCenter.y - currentCenter.y;

        // 根据开口方向判断候选轮廓的位置
        bool isInPreferredArea = false;
        switch (currentDirection) {
        case OpeningDirection::LEFT:
            isInPreferredArea = (dx < 0); // 候选轮廓在当前轮廓左侧
            break;
        case OpeningDirection::RIGHT:
            isInPreferredArea = (dx > 0); // 候选轮廓在当前轮廓右侧
            break;
        case OpeningDirection::UP:
            isInPreferredArea = (dy < 0); // 候选轮廓在当前轮廓上方
            break;
        case OpeningDirection::DOWN:
            isInPreferredArea = (dy > 0); // 候选轮廓在当前轮廓下方
            break;
        case OpeningDirection::UNKNOWN:
        default:
            isInPreferredArea = true; // 未知方向，不分区
            break;
        }

        if (isInPreferredArea) {
            preferredIds.push_back(candidateId);
        } else {
            otherIds.push_back(candidateId);
        }
    }
    return {preferredIds, otherIds};
}

/**
 * @brief 尝试生成2轮廓组合
 * @param currentId 当前轮廓ID
 * @param candidateIds 候选轮廓ID列表
 * @param unpairedContours 未配对的轮廓字典（ID → CBB）
 * @param generatedCombinations 已生成的组合集合
 * @return 是否成功生成组合
 */
bool WorkpieceGenerator::tryGenerateTwoContourCombination(int currentId,
                                                          const std::vector<int>& candidateIds,
                                                          const UnpairedContoursMap& unpairedContours,
                                                          std::set<std::set<int>>& generatedCombinations) {

    bool foundCombination = false;

    for (int candidateId : candidateIds) {
        WorkpieceBoundingBox wp2;
        // 检查添加是否成功
        if (!wp2.addContourBoundingBox(unpairedContours.at(currentId)) ||
            !wp2.addContourBoundingBox(unpairedContours.at(candidateId))){
            continue;
        }

        if (isLegalCombination(wp2)) {
            std::set<int> combination = {currentId, candidateId};
            if (generatedCombinations.find(combination) == generatedCombinations.end()) {
                m_possibleWorkpieces.push_back(wp2);
                generatedCombinations.insert(combination);
                foundCombination = true;
                // return true;
            }
        }
    }
    return foundCombination;
}

/**
 * @brief 尝试生成3轮廓组合
 * @param currentId 当前轮廓ID
 * @param secondId 第二个轮廓ID
 * @param candidateIds 候选轮廓ID列表
 * @param unpairedContours 未配对的轮廓字典（ID → CBB）
 * @param generatedCombinations 已生成的组合集合
 * @return 是否成功生成组合
 */
bool WorkpieceGenerator::tryGenerateThreeContourCombination(int currentId,
                                                            int secondId,
                                                            const std::vector<int>& candidateIds,
                                                            const UnpairedContoursMap& unpairedContours,
                                                            std::set<std::set<int>>& generatedCombinations) {

    bool foundCombination = false;

    for (int k : candidateIds) {
        if (k == currentId || k == secondId) continue;
        if (currentId >= k || secondId >= k) continue; // 确保有序，避免重复

        WorkpieceBoundingBox wp3;
        // 检查添加是否成功
        if (!wp3.addContourBoundingBox(unpairedContours.at(currentId)) ||
            !wp3.addContourBoundingBox(unpairedContours.at(secondId)) ||
            !wp3.addContourBoundingBox(unpairedContours.at(k))) {
            continue;
        }

        if (isLegalCombination(wp3)) {
            std::set<int> combination = {currentId, secondId, k};
            if (generatedCombinations.find(combination) == generatedCombinations.end()) {
                m_possibleWorkpieces.push_back(wp3);
                generatedCombinations.insert(combination);
                foundCombination = true;
                // return true;
            }
        }
    }
    return foundCombination;
}

/**
 * @brief 搜索并生成工件组合
 * @param currentId 当前轮廓ID
 * @param unpairedContours 未配对的轮廓字典（ID → CBB）
 * @param nearestIndices 最近邻ID映射（ID → sorted IDs）
 * @param generatedCombinations 已生成的组合集合
 */
void WorkpieceGenerator::searchAndGenerateCombinations(int currentId,
                                                       const UnpairedContoursMap& unpairedContours,
                                                       const NearestIndicesMap& nearestIndices,
                                                       std::set<std::set<int>>& generatedCombinations) {

    // 获取当前轮廓信息
    OpeningDirection currentDirection = unpairedContours.at(currentId)->getOpeningDirection();
    cv::Point2f currentCenter = unpairedContours.at(currentId)->getCenterPoint();

    // 根据开口方向分区
    auto [preferredIds, otherIds] = partitionCandidatesByDirection(
        currentId, currentDirection, currentCenter, nearestIndices.at(currentId), unpairedContours);

    tryGenerateTwoContourCombination(currentId, preferredIds, unpairedContours, generatedCombinations);

    // 尝试3轮廓组合
    for (int secondId : preferredIds) {
        tryGenerateThreeContourCombination(currentId, secondId, preferredIds, unpairedContours, generatedCombinations);
    }
}

/**
 * @brief 输出可能的工件组合
 */
void WorkpieceGenerator::outputPossibleWorkpieces() {
    PLOG_INFO << "可能的工件组合";
    int i = 0;
    for (auto& workPiece : m_possibleWorkpieces) {
        std::vector<int> ids = workPiece.getContourIds();
        QString str = QString::number(i++) + ": ";
        for (auto& id : ids) {
            str += QString("%1,").arg(id);
        }
        PLOG_INFO << str.toStdString();
    }
}

/**
 * @brief 将轮廓矩形组合，生成所有可能的工件
 * @return 无返回值
 */
void WorkpieceGenerator::generateWorkpieces() {
    // 获取所有未配对的轮廓，以ID为键构建字典
    UnpairedContoursMap unpairedContours;
    for (auto& c : m_cbbs) {
        if (!c->getIsPaired()) {
            unpairedContours[c->getId()] = c;
        }
    }

    if (unpairedContours.empty()) {
        PLOG_INFO << "没有未配对的轮廓";
        return;
    }

    // 计算距离矩阵和最近邻索引
    auto distanceMatrix = calculateDistanceMatrix(unpairedContours);
    auto nearestIndices = createNearestIndices(unpairedContours, distanceMatrix);

    // 生成的组合
    std::set<std::set<int>> generatedCombinations;

    // 遍历所有未配对轮廓的ID
    for (const auto& [currentId, _] : unpairedContours) {
        searchAndGenerateCombinations(currentId, unpairedContours, nearestIndices, generatedCombinations);
    }

    // 输出可能的工件组合
    outputPossibleWorkpieces();
}

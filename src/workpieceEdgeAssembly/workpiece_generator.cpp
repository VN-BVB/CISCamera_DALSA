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
 * @param unpairedContours 未配对的轮廓列表
 * @return 距离矩阵
 */
std::vector<std::vector<float>> WorkpieceGenerator::calculateDistanceMatrix(
    const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours) {
    std::vector<std::vector<float>> distanceMatrix(
        unpairedContours.size(), std::vector<float>(unpairedContours.size(), 0.0f));

    for (int i = 0; i < unpairedContours.size(); ++i) {
        cv::Point2f center1 = unpairedContours[i]->getCenterPoint();
        for (int j = i + 1; j < unpairedContours.size(); ++j) {
            cv::Point2f center2 = unpairedContours[j]->getCenterPoint();
            float dx = center2.x - center1.x;
            float dy = center2.y - center1.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            distanceMatrix[i][j] = distance;
            distanceMatrix[j][i] = distance;
        }
    }
    return distanceMatrix;
}

/**
 * @brief 为每个轮廓创建距离排序的索引列表
 * @param unpairedContours 未配对的轮廓列表
 * @param distanceMatrix 距离矩阵
 * @return 每个轮廓的最近邻索引列表
 */
std::vector<std::vector<int>> WorkpieceGenerator::createNearestIndices(
    const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours,
    const std::vector<std::vector<float>>& distanceMatrix) {
    std::vector<std::vector<int>> nearestIndices(unpairedContours.size());

    for (int i = 0; i < unpairedContours.size(); ++i) {
        std::vector<std::pair<float, int>> distances;
        for (int j = 0; j < unpairedContours.size(); ++j) {
            if (i != j) {
                distances.emplace_back(distanceMatrix[i][j], j);
            }
        }
        // 按距离从小到大排序
        std::sort(distances.begin(), distances.end());
        for (const auto& dist : distances) {
            nearestIndices[i].push_back(dist.second);
        }
    }
    return nearestIndices;
}

/**
 * @brief 根据开口方向对候选轮廓进行分区
 * @param currentIndex 当前轮廓索引
 * @param currentDirection 当前轮廓开口方向
 * @param currentCenter 当前轮廓中心点
 * @param candidateIndices 候选轮廓索引列表
 * @param unpairedContours 未配对的轮廓列表
 * @return 分区后的索引列表（优先区域和其他区域）
 */
std::pair<std::vector<int>, std::vector<int>>
WorkpieceGenerator::partitionCandidatesByDirection(int currentIndex,
                                                   OpeningDirection currentDirection,
                                                   const cv::Point2f& currentCenter,
                                                   const std::vector<int>& candidateIndices,
                                                   const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours) {

    std::vector<int> preferredIndices;
    std::vector<int> otherIndices;

    for (int j : candidateIndices) {
        if (currentIndex >= j) continue; // 避免重复组合

        cv::Point2f candidateCenter = unpairedContours[j]->getCenterPoint();
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
            preferredIndices.push_back(j);
        } else {
            otherIndices.push_back(j);
        }
    }
    return {preferredIndices, otherIndices};
}

/**
 * @brief 尝试生成2轮廓组合
 * @param currentIndex 当前轮廓索引
 * @param candidateIndices 候选轮廓索引列表
 * @param unpairedContours 未配对的轮廓列表
 * @param generatedCombinations 已生成的组合集合
 * @return 是否成功生成组合
 */
bool WorkpieceGenerator::tryGenerateTwoContourCombination(int currentIndex,
                                                          const std::vector<int>& candidateIndices,
                                                          const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours,
                                                          std::set<std::set<int>>& generatedCombinations) {

    bool foundCombination = false;

    for (int j : candidateIndices) {
        WorkpieceBoundingBox wp2;
        // 检查添加是否成功
        if (!wp2.addContourBoundingBox(unpairedContours[currentIndex]) ||
            !wp2.addContourBoundingBox(unpairedContours[j])){
            continue;
        }

        if (isLegalCombination(wp2)) {
            std::set<int> combination = {currentIndex, j};
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
 * @param currentIndex 当前轮廓索引
 * @param secondIndex 第二个轮廓索引
 * @param candidateIndices 候选轮廓索引列表
 * @param unpairedContours 未配对的轮廓列表
 * @param generatedCombinations 已生成的组合集合
 * @return 是否成功生成组合
 */
bool WorkpieceGenerator::tryGenerateThreeContourCombination(int currentIndex,
                                                            int secondIndex,
                                                            const std::vector<int>& candidateIndices,
                                                            const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours,
                                                            std::set<std::set<int>>& generatedCombinations) {

    bool foundCombination = false;

    for (int k : candidateIndices) {
        if (k == currentIndex || k == secondIndex) continue;
        if (currentIndex >= k || secondIndex >= k) continue; // 确保有序，避免重复

        WorkpieceBoundingBox wp3;
        // 检查添加是否成功
        if (!wp3.addContourBoundingBox(unpairedContours[currentIndex]) ||
            !wp3.addContourBoundingBox(unpairedContours[secondIndex]) ||
            !wp3.addContourBoundingBox(unpairedContours[k])) {
            continue;
        }

        if (isLegalCombination(wp3)) {
            std::set<int> combination = {currentIndex, secondIndex, k};
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
 * @param currentIndex 当前轮廓索引
 * @param unpairedContours 未配对的轮廓列表
 * @param nearestIndices 最近邻索引列表
 * @param generatedCombinations 已生成的组合集合
 */
void WorkpieceGenerator::searchAndGenerateCombinations(int currentIndex,
                                                       const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours,
                                                       const std::vector<std::vector<int>>& nearestIndices,
                                                       std::set<std::set<int>>& generatedCombinations) {

    // 获取当前轮廓信息
    OpeningDirection currentDirection = unpairedContours[currentIndex]->getOpeningDirection();
    cv::Point2f currentCenter = unpairedContours[currentIndex]->getCenterPoint();

    // 根据开口方向分区
    auto [preferredIndices, otherIndices] = partitionCandidatesByDirection(
        currentIndex, currentDirection, currentCenter, nearestIndices[currentIndex], unpairedContours);

    tryGenerateTwoContourCombination(currentIndex, preferredIndices, unpairedContours, generatedCombinations);

    // 尝试3轮廓组合
    for (int j : preferredIndices) {
        tryGenerateThreeContourCombination(currentIndex, j, preferredIndices, unpairedContours, generatedCombinations);
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
    // 获取所有未配对的轮廓
    std::vector<std::shared_ptr<ContourBoundingBox>> unpairedContours;
    for (auto& c : m_cbbs) {
        if (!c->getIsPaired()) {
            unpairedContours.push_back(c);
        }
    }

    if (unpairedContours.empty()) {
        PLOG_INFO << "没有未配对的轮廓";
        return;
    }

    // 计算距离矩阵和最近邻索引
    auto distanceMatrix = calculateDistanceMatrix(unpairedContours);
    auto nearestIndices = createNearestIndices(unpairedContours, distanceMatrix);

    // 使用集合来跟踪已经生成的组合，避免重复
    std::set<std::set<int>> generatedCombinations;

    // 优先搜索最近邻组合
    for (int i = 0; i < unpairedContours.size(); ++i) {
        searchAndGenerateCombinations(i, unpairedContours, nearestIndices, generatedCombinations);
    }

    // 输出可能的工件组合
    outputPossibleWorkpieces();
}

#ifndef WORKPIECE_GENERATOR_H
#define WORKPIECE_GENERATOR_H

#include "workpiece_bounding_box.h"
#include <cstdint>
#include <functional>

class WorkpieceGenerator
{
public:
    WorkpieceGenerator(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs);
    std::vector<WorkpieceBoundingBox> getPossibleWorkpieces() const { return m_possibleWorkpieces; }

    void generateWorkpieces();

private:
    // 位运算类型定义
    typedef uint64_t ContourSet;

    // 辅助方法
    bool isLegalCombination(const WorkpieceBoundingBox& workpiece);

    // 重构后的工件生成辅助方法
    std::vector<std::vector<float>> calculateDistanceMatrix(
        const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours);
    std::vector<std::vector<int>> createNearestIndices(
        const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours,
        const std::vector<std::vector<float>>& distanceMatrix);
    std::pair<std::vector<int>, std::vector<int>> partitionCandidatesByDirection(
        int currentIndex, OpeningDirection currentDirection, const cv::Point2f& currentCenter,
        const std::vector<int>& candidateIndices,
        const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours);
    bool tryGenerateTwoContourCombination(
        int currentIndex, const std::vector<int>& candidateIndices,
        const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours,
        std::set<std::set<int>>& generatedCombinations);
    bool tryGenerateThreeContourCombination(
        int currentIndex, int secondIndex, const std::vector<int>& candidateIndices,
        const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours,
        std::set<std::set<int>>& generatedCombinations);
    void searchAndGenerateCombinations(
        int currentIndex, const std::vector<std::shared_ptr<ContourBoundingBox>>& unpairedContours,
        const std::vector<std::vector<int>>& nearestIndices,
        std::set<std::set<int>>& generatedCombinations);
    void outputPossibleWorkpieces();

    std::vector<std::shared_ptr<ContourBoundingBox>> m_cbbs;                // 所有待分组的轮廓框
    std::vector<WorkpieceBoundingBox> m_possibleWorkpieces;                 // 所有可能的工件组合
};

#endif // WORKPIECE_GENERATOR_H

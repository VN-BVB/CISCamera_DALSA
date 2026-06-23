#ifndef DOOR_BELL_COMBINER_H
#define DOOR_BELL_COMBINER_H

#include "workpiece_bounding_box.h"
#include "workpiece_generator.h"
#include <cstdint>
#include <functional>

class DoorBellCombiner
{
public:
    DoorBellCombiner(const std::vector<WorkpieceBoundingBox>& possibleWorkpieces);
    std::vector<std::vector<int>> getValidCombinations() const { return m_validCombinations; }
    std::vector<int> getMostLikelyCombination() const { return m_mostLikelyCombination; }

    void generateValidCombinations(int n, const std::vector<std::shared_ptr<ContourBoundingBox>>& cbbs);
    void calculateMostLikelyCombination();
    std::map<int, std::vector<int>> outputResult();

private:
    // 位运算类型定义
    typedef uint64_t ContourSet;

    // 工件信息结构体（用于动态规划）
    struct WorkpieceInfo {
        int index;
        ContourSet contourMask;
        int contourCount;
        float boundingBoxArea;

        WorkpieceInfo(int idx, const WorkpieceBoundingBox& wp);
    };

    // 辅助方法
    int getMaxContoursPerWorkpiece(const std::vector<WorkpieceInfo>& infos, int start) const;
    bool hasIntersectionWithSelected(const std::vector<int>& selected, int newIndex) const;
    int countBits(ContourSet mask) const;
    ContourSet createTargetMask(const std::vector<std::shared_ptr<ContourBoundingBox>>& cbbs) const;

    // 组合生成方法
    void generateOptimizedCombinations(int start,
                                       int k,
                                       std::vector<int>& current,
                                       std::set<int>& usedContourIds,
                                       std::vector<std::vector<int>>& result,
                                       const std::vector<std::shared_ptr<ContourBoundingBox>>& cbbs);
    void generateOptimizedCombinationsDP(int k,
                                         std::vector<std::vector<int>>& result,
                                         const std::vector<std::shared_ptr<ContourBoundingBox>>& cbbs);
    void dfsCombinations(const std::vector<WorkpieceInfo>& infos, int start, int k,
                         ContourSet targetMask, ContourSet usedMask,
                         std::vector<int>& current, std::vector<std::vector<int>>& result);

    // 检查组合中工件是否相交
    bool doWorkpiecesIntersect(const WorkpieceBoundingBox& wp1, const WorkpieceBoundingBox& wp2) const;
    bool checkWorkpieceIntersections(const std::vector<WorkpieceBoundingBox>& workpieces,
                                     const std::vector<int>& combination);

    // 计算工件组合的总线段长度
    float calculateCombinationTotalLength(const std::vector<int>& combination) const;

    std::vector<WorkpieceBoundingBox> m_possibleWorkpieces;                 // 所有可能的工件
    std::vector<std::vector<int>> m_validCombinations;                      // 最终合法的工件组合
    std::vector<int> m_mostLikelyCombination;                               // 最有可能的工件组合
};

#endif // DOOR_BELL_COMBINER_H

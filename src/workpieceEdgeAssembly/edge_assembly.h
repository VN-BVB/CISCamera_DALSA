#ifndef EDGE_ASSEMBLY_H
#define EDGE_ASSEMBLY_H

#include "workpiece_bounding_box.h"
#include <cstdint>
#include <functional>

class EdgeAssembly
{
public:
    EdgeAssembly(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs);
    std::vector<WorkpieceBoundingBox> getpossibleWorkpieces() const {return m_possibleWorkpieces;}
    std::vector<std::vector<int>> getValidCombinations() const {return m_validCombinations;}
    std::vector<int> getMostLikelyCombination() const {return m_mostLikelyCombination;}  // 获取最有可能的组合

    void makeTestExample(JointSeam jointSeam);
    void generateWorkpiece();        // 生成所有可能的 2 轮廓、3 轮廓工件
    void validateCombinations(int n);        // 根据门环工件个数，组合工件，保证包含所有轮廓且，轮廓不重合
    void outputResult();                //输出分组结果（结果由工件：其轮廓的id组成）
    bool isCombinationValid(const std::vector<WorkpieceBoundingBox>& workpieces,
                            const std::vector<int>& combination);
    void generateCombinations(int n, int k, int start, std::vector<int>& current,
                              std::vector<std::vector<int>>& result);
    void calculateMostLikelyCombination();  // 计算最有可能的工件组合

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

    // 优化的组合生成方法
    void generateOptimizedCombinations(int start, int k,
                                       std::vector<int>& current,
                                       std::set<int>& usedContourIds,
                                       std::vector<std::vector<int>>& result);

    // 动态规划+位运算优化方法
    void generateOptimizedCombinationsDP(int k, std::vector<std::vector<int>>& result);
    void dfsCombinations(const std::vector<WorkpieceInfo>& infos, int start, int k,
                         ContourSet targetMask, ContourSet usedMask,
                         std::vector<int>& current, std::vector<std::vector<int>>& result);

    // 辅助方法
    int getMaxContoursPerWorkpiece(const std::vector<WorkpieceInfo>& infos, int start) const;
    bool hasIntersectionWithSelected(const std::vector<int>& selected, int newIndex) const;
    int countBits(ContourSet mask) const;
    ContourSet createTargetMask() const;

    // 检查组合是否合法（不检查轮廓ID重复）
    bool isCombinationValidWithoutIdCheck(const std::vector<WorkpieceBoundingBox>& workpieces,
                                          const std::vector<int>& combination);

    // 计算工件组合的总线段长度
    float calculateCombinationTotalLength(const std::vector<int>& combination) const;
    bool doWorkpiecesIntersect(const WorkpieceBoundingBox& wp1, const WorkpieceBoundingBox& wp2) const;

    std::vector<std::shared_ptr<ContourBoundingBox>> m_cbbs;                // 所有待分组的轮廓框
    std::vector<WorkpieceBoundingBox> m_possibleWorkpieces;                 // 所有可能的工件组合
    std::vector<std::vector<int>> m_validCombinations;                      // 最终合法的工件组合
    std::vector<int> m_mostLikelyCombination;                                // 最有可能的工件组合
};

#endif // EDGE_ASSEMBLY_H

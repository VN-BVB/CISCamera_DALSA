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
    std::vector<int> getMostLikelyCombination() const {return m_mostLikelyCombination;}

    void run(int n);

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

    // =========辅助方法=========
    // 获取工件集合中最大轮廓数
    int getMaxContoursPerWorkpiece(const std::vector<WorkpieceInfo>& infos, int start) const;
    // 检查工件是否与已选工件相交
    bool hasIntersectionWithSelected(const std::vector<int>& selected, int newIndex) const;
    // 计算轮廓集合掩码中设置的位数（即包含的轮廓数量）
    int countBits(ContourSet mask) const;
    // 创建目标轮廓掩码，表示需要覆盖的所有轮廓
    ContourSet createTargetMask() const;

    // =========组合生成方法=========
    // 使用多种剪枝策略和启发式搜索的组合生成算法
    void generateOptimizedCombinations(int start, int k,
                                       std::vector<int>& current,
                                       std::set<int>& usedContourIds,
                                       std::vector<std::vector<int>>& result);
    // 动态规划+位运算优化的组合生成算法
    void generateOptimizedCombinationsDP(int k, std::vector<std::vector<int>>& result);
    // DFS搜索组合（动态规划核心）
    void dfsCombinations(const std::vector<WorkpieceInfo>& infos, int start, int k,
                         ContourSet targetMask, ContourSet usedMask,
                         std::vector<int>& current, std::vector<std::vector<int>>& result);

    // =========检查组合中工件是否相交=========
    // 检查两个工件是否相交
    bool doWorkpiecesIntersect(const WorkpieceBoundingBox& wp1, const WorkpieceBoundingBox& wp2) const;
    // 检查组合是否合法：根据组合中是否有工件相交
    bool checkWorkpieceIntersections(const std::vector<WorkpieceBoundingBox>& workpieces,
                                     const std::vector<int>& combination);

    // 计算工件组合的总线段长度
    float calculateCombinationTotalLength(const std::vector<int>& combination) const;

    // =========流程方法=========
    // 将轮廓矩形组合，生成所有可能的工件
    void generateWorkpiece();
    // 根据门环工件个数，组合工件，保证包含所有轮廓且，轮廓不重合
    void generateValidateCombinations(int n);       // 根据门环工件个数，组合工件，保证包含所有轮廓且，轮廓不重合
    // 计算最有可能的工件组合
    void calculateMostLikelyCombination();
    // 输出分组结果（结果由工件：其轮廓的id组成）
    void outputResult();

    std::vector<std::shared_ptr<ContourBoundingBox>> m_cbbs;                // 所有待分组的轮廓框
    std::vector<WorkpieceBoundingBox> m_possibleWorkpieces;                 // 所有可能的工件组合
    std::vector<std::vector<int>> m_validCombinations;                      // 最终合法的工件组合
    std::vector<int> m_mostLikelyCombination;                               // 最有可能的工件组合
};

#endif // EDGE_ASSEMBLY_H

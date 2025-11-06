#ifndef EDGE_ASSEMBLY_H
#define EDGE_ASSEMBLY_H

#include "workpiece_bounding_box.h"

class EdgeAssembly
{
public:
    EdgeAssembly(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs);
    std::vector<WorkpieceBoundingBox> getpossibleWorkpieces() const {return m_possibleWorkpieces;}
    std::vector<std::vector<int>> getValidCombinations() const {return m_validCombinations;}

    void makeTestExample(JointSeam jointSeam);
    void generateWorkpiece();        // 生成所有可能的 2 轮廓、3 轮廓工件
    void validateCombinations(int n);        // 根据门环工件个数，组合工件，保证包含所有轮廓且，轮廓不重合
    void outputResult();                //输出分组结果（结果由工件：其轮廓的id组成）
    bool isCombinationValid(const std::vector<WorkpieceBoundingBox>& workpieces,
                            const std::vector<int>& combination);
    void generateCombinations(int n, int k, int start, std::vector<int>& current,
                              std::vector<std::vector<int>>& result);

private:
    std::vector<std::shared_ptr<ContourBoundingBox>> m_cbbs;                // 所有待分组的轮廓框
    std::vector<WorkpieceBoundingBox> m_possibleWorkpieces;                 // 所有可能的工件组合
    std::vector<std::vector<int>> m_validCombinations;                      // 最终合法的工件组合
};

#endif // EDGE_ASSEMBLY_H

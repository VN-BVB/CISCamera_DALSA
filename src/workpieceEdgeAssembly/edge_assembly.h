#ifndef EDGE_ASSEMBLY_H
#define EDGE_ASSEMBLY_H

#include "workpiece_bounding_box.h"

class EdgeAssembly
{
public:
    EdgeAssembly();

    void makeTestExample(JointSeam jointSeam);
    void generateWorkpiece();        // 生成所有可能的 2 轮廓、3 轮廓工件
    void validateCombinations(int n);        // 根据门环工件个数，组合工件，保证轮廓不重合
    void outputResult();                //输出分组结果（结果由工件：其轮廓的id组成）

private:
    std::vector<std::shared_ptr<ContourBoundingBox>> m_cbbs;                // 所有待分组的轮廓框
    std::vector<WorkpieceBoundingBox> m_resultWorkpieces; // 最终分组结果
};

#endif // EDGE_ASSEMBLY_H

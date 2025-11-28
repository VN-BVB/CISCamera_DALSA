#include "edge_assembly.h"
#include "workpiece_generator.h"
#include "workpiece_combiner.h"

EdgeAssembly::EdgeAssembly() {}

EdgeAssembly::EdgeAssembly(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs)
{
    m_cbbs = cbbs;
    m_workpieceGenerator = std::make_unique<WorkpieceGenerator>(cbbs);
}

std::vector<WorkpieceBoundingBox> EdgeAssembly::getPossibleWorkpieces() const
{
    return m_workpieceGenerator->getPossibleWorkpieces();
}

std::vector<std::vector<int>> EdgeAssembly::getValidCombinations() const
{
    if (m_workpieceCombiner) {
        return m_workpieceCombiner->getValidCombinations();
    }
    return {};
}

std::vector<int> EdgeAssembly::getMostLikelyCombination() const
{
    if (m_workpieceCombiner) {
        return m_workpieceCombiner->getMostLikelyCombination();
    }
    return {};
}

void EdgeAssembly::run(int n)
{
    // 第一步：生成所有可能的工件
    m_workpieceGenerator->generateWorkpieces();

    // 第二步：组合工件成门环
    m_workpieceCombiner = std::make_unique<WorkpieceCombiner>(m_workpieceGenerator->getPossibleWorkpieces());
    m_workpieceCombiner->generateValidCombinations(n, m_cbbs);
    m_workpieceCombiner->calculateMostLikelyCombination();
    m_workpieceCombiner->outputResult();
}

void EdgeAssembly::whenAllImagesProcessed(const std::map<int, ProcessedROIInfo>& processedRoiInfos)
{
    // 1、解析出轮廓数据
    std::vector<std::shared_ptr<ContourBoundingBox>> cbbs;
    for (const auto& [key, roiInfo] : processedRoiInfos) {
        int index = roiInfo.index;
        if (roiInfo.contourDatas.size() >=2) {
            ContourData firstContourData = roiInfo.contourDatas[0];
            std::shared_ptr<ContourBoundingBox> firstCbb = std::make_shared<ContourBoundingBox>();
            firstCbb->initContourData(index * 2, firstContourData);
            cbbs.push_back(firstCbb);
            ContourData secondContourData = roiInfo.contourDatas[1];
            std::shared_ptr<ContourBoundingBox> secondCbb = std::make_shared<ContourBoundingBox>();
            secondCbb->initContourData(index * 2 + 1, secondContourData);
            cbbs.push_back(secondCbb);
        }
    }
    m_cbbs = cbbs;
    // 2、生成所有可能的工件
    m_workpieceGenerator = std::make_unique<WorkpieceGenerator>(cbbs);
    m_workpieceGenerator->generateWorkpieces();

    // 3、组合工件成门环
    m_workpieceCombiner = std::make_unique<WorkpieceCombiner>(m_workpieceGenerator->getPossibleWorkpieces());
    m_workpieceCombiner->generateValidCombinations(9, m_cbbs);
    m_workpieceCombiner->calculateMostLikelyCombination();
    m_workpieceCombiner->outputResult();
}




























#include <plog/Log.h>
#include "edge_assembly.h"
#include "workpiece_generator.h"
#include "door_bell_combiner.h"

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
    m_workpieceCombiner = std::make_unique<DoorBellCombiner>(m_workpieceGenerator->getPossibleWorkpieces());
    m_workpieceCombiner->generateValidCombinations(n, m_cbbs);
    m_workpieceCombiner->calculateMostLikelyCombination();
    m_workpieceCombiner->outputResult();
}

void EdgeAssembly::whenAllImagesProcessed(const std::map<int, ProcessedROIInfo>& processedRoiInfos)
{
    // 1、解析出轮廓数据
    std::vector<std::shared_ptr<ContourBoundingBox>> cbbs;
    for (const auto& [key, roiInfo] : processedRoiInfos) {
        for (const auto& [contourId, contourData] : roiInfo.contourDatas) {
            std::shared_ptr<ContourBoundingBox> cbb = std::make_shared<ContourBoundingBox>();
            cbb->initContourData(contourId, contourData);
            cbbs.push_back(cbb);
        }
    }
    m_cbbs = cbbs;
    // 2、生成所有可能的工件
    m_workpieceGenerator = std::make_unique<WorkpieceGenerator>(cbbs);
    m_workpieceGenerator->generateWorkpieces();

    // 3、组合工件成门环
    m_workpieceCombiner = std::make_unique<DoorBellCombiner>(m_workpieceGenerator->getPossibleWorkpieces());
    m_workpieceCombiner->generateValidCombinations(9, m_cbbs);
    m_workpieceCombiner->calculateMostLikelyCombination();
    std::map<int, std::vector<int>> combinationResult = m_workpieceCombiner->outputResult();

    // 4、发送组合完成信号
    std::map<int, std::vector<int>> workpieceToRoiInfos;
    for (const auto& [workpieceId, contourIds] : combinationResult) {
        std::vector<int> relatedRoiInfos;
        // 在processedRoiInfos中查找包含对应轮廓ID的ProcessedROIInfo
        for (const auto& [roiKey, roiInfo] : processedRoiInfos) {
            for (const auto& contourId : contourIds) {
                if (roiInfo.contourDatas.find(contourId) != roiInfo.contourDatas.end()) {
                    relatedRoiInfos.push_back(roiInfo.index);
                    break;
                }
            }
        }
        workpieceToRoiInfos[workpieceId] = relatedRoiInfos;
    }

    emit sendEdgeAssemblyFinished(workpieceToRoiInfos, processedRoiInfos);
}




























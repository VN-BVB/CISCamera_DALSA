#ifndef WORKPIECE_GENERATOR_H
#define WORKPIECE_GENERATOR_H

#include "workpiece_bounding_box.h"
#include <cstdint>
#include <functional>
#include <map>
#include <utility>

// 未配对轮廓映射类型定义（放在类外部）
typedef std::map<int, std::shared_ptr<ContourBoundingBox>> UnpairedContoursMap;
typedef std::map<std::pair<int, int>, float> DistanceMatrix;
typedef std::map<int, std::vector<int>> NearestIndicesMap;

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
    DistanceMatrix calculateDistanceMatrix(const UnpairedContoursMap& unpairedContours);
    NearestIndicesMap createNearestIndices(const UnpairedContoursMap& unpairedContours,
                                          const DistanceMatrix& distanceMatrix);
    std::pair<std::vector<int>, std::vector<int>> partitionCandidatesByDirection(int currentId, const cv::Point2f& openingDirection, const cv::Point2f& currentCenter,
                                                                                 const std::vector<int>& candidateIds,
                                                                                 const UnpairedContoursMap& unpairedContours);
    bool tryGenerateTwoContourCombination(int currentId, const std::vector<int>& candidateIds,
                                          const UnpairedContoursMap& unpairedContours,
                                          std::set<std::set<int>>& generatedCombinations);
    bool tryGenerateThreeContourCombination(int currentId, int secondId, const std::vector<int>& candidateIds,
                                            const UnpairedContoursMap& unpairedContours,
                                            std::set<std::set<int>>& generatedCombinations);
    void searchAndGenerateCombinations(int currentId, const UnpairedContoursMap& unpairedContours,
                                       const NearestIndicesMap& nearestIndices,
                                       std::set<std::set<int>>& generatedCombinations);
    void outputPossibleWorkpieces();

    std::vector<std::shared_ptr<ContourBoundingBox>> m_cbbs;                // 所有待分组的轮廓框
    std::vector<WorkpieceBoundingBox> m_possibleWorkpieces;                 // 所有可能的工件组合
};

#endif // WORKPIECE_GENERATOR_H

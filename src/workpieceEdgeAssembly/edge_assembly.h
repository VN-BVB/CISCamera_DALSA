#ifndef EDGE_ASSEMBLY_H
#define EDGE_ASSEMBLY_H

#include "workpiece_generator.h"
#include "workpiece_combiner.h"
#include "src/jointDetection/image_process_worker.h"
#include <memory>

class EdgeAssembly
{
public:
    EdgeAssembly();
    explicit EdgeAssembly(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs);
    std::vector<WorkpieceBoundingBox> getPossibleWorkpieces() const;
    std::vector<std::vector<int>> getValidCombinations() const;
    std::vector<int> getMostLikelyCombination() const;

    void run(int n);

public slots:
    void whenAllImagesProcessed(std::map<int, ProcessedROIInfo> processedRoiInfos);

private:
    std::vector<std::shared_ptr<ContourBoundingBox>> m_cbbs;
    std::unique_ptr<WorkpieceGenerator> m_workpieceGenerator;
    std::unique_ptr<WorkpieceCombiner> m_workpieceCombiner;
};

#endif // EDGE_ASSEMBLY_H

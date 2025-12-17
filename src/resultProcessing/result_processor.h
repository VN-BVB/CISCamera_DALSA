#ifndef RESULT_PROCESSOR_H
#define RESULT_PROCESSOR_H

#include <QObject>
#include <map>
#include <memory>
#include "src/jointDetection/image_process_worker.h"
#include "src/resultProcessing/outputs/dxfSaver/dxf_saver.h"
#include "src/resultProcessing/transformers/workpiece_roi_mapper.h"

class ResultProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ResultProcessor(QObject *parent = nullptr);

public slots:
    void whenEdgeAssemblyFinished(const std::map<int, std::vector<int>>& combinationResult,
                                 const std::map<int, ProcessedROIInfo>& processedRoiInfos);

private:
    std::shared_ptr<DXFSaver> m_dxfSaver;
};

#endif // RESULT_PROCESSOR_H

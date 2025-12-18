#ifndef RESULT_PROCESSOR_H
#define RESULT_PROCESSOR_H

#include <QObject>
#include <map>
#include <memory>
#include "src/jointDetection/image_process_worker.h"
#include "src/resultProcessing/outputs/dxfSaver/dxf_saver.h"
#include "src/resultProcessing/outputs/jsonSaver/json_saver.h"
#include "src/resultProcessing/outputs/transferResult/json_sender.h"
#include "src/resultProcessing/transformers/workpiece_roi_mapper.h"
#include "src/resultProcessing/transformers/json_transformer.h"

class ResultProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ResultProcessor(QObject *parent = nullptr);

public slots:
    void whenEdgeAssemblyFinished(const std::map<int, std::vector<int>>& combinationResult,
                                 const std::map<int, ProcessedROIInfo>& processedRoiInfos);

private:
    void sendJsonToSharedMemory(const std::string& jsonString, int batchNumber);

    std::shared_ptr<DXFSaver> m_dxfSaver;
    std::unique_ptr<JsonTransformer> m_jsonTransformer;
    std::unique_ptr<JsonSender> m_jsonSender;
    std::unique_ptr<JsonSaver> m_jsonSaver;
};

#endif // RESULT_PROCESSOR_H

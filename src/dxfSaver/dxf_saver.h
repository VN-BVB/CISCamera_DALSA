#ifndef DXF_SAVER_H
#define DXF_SAVER_H

#include <QObject>
#include <map>
#include <opencv2/core/core.hpp>
#include "../jointDetection/image_process_worker.h"
#include "../../3rdParty/dxflib-3.26.4-src/dl_dxf.h"

class DXFSaver : public QObject
{
    // Q_OBJECT
public:
    DXFSaver();

public slots:
    void whenAllImagesProcessed(std::map<int, ProcessedROIInfo> processedRoiInfos);
};

#endif // DXF_SAVER_H

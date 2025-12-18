#ifndef DXF_SAVER_H
#define DXF_SAVER_H

#include <QObject>
#include <map>
#include <opencv2/core/core.hpp>
#include "src/jointDetection/image_process_worker.h"
#include "../../3rdParty/dxflib-3.26.4-src/dl_dxf.h"

class DXFSaver : public QObject
{
    // Q_OBJECT
public:
    DXFSaver();

public slots:
    void whenAllImagesProcessed(const std::map<int, std::vector<int>>& workpieceToRoiInfos,
                                const std::map<int, ProcessedROIInfo>& processedRoiInfos);

private:
    // 绘制亚像素轮廓
    void drawSubpixelContours(DL_Dxf& dxf, DL_WriterA* dw, const DL_Attributes& attributes,
                              const ProcessedROIInfo& roiInfo);

    // 绘制样条曲线
    void drawSplines(DL_Dxf& dxf, DL_WriterA* dw, const DL_Attributes& attributes,
                     const ProcessedROIInfo& roiInfo);
};

#endif // DXF_SAVER_H

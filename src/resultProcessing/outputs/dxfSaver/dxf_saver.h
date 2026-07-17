#ifndef DXF_SAVER_H
#define DXF_SAVER_H

#include <QObject>
#include <map>
#include <opencv2/core/core.hpp>
#include <Eigen/Core>
#include "src/jointDetection/image_process_worker.h"
#include "src/utils/platform_pose_io.h"
#include "../../3rdParty/dxflib-3.26.4-src/dl_dxf.h"

class DXFSaver : public QObject
{
    // Q_OBJECT
public:
    DXFSaver();

public slots:
    void whenAllImagesProcessed(const std::map<int, std::vector<int>>& workpieceToRoiInfos,
                                const std::map<int, ProcessedROIInfo>& processedRoiInfos,
                                const std::map<int, Eigen::Vector2d>& workpieceCenters,
                                const std::map<int, int>& workpieceToPlatform,
                                const std::vector<PlatformAxis>& platforms);

private:
    // 绘制亚像素轮廓
    void drawSubpixelContours(DL_Dxf& dxf, DL_WriterA* dw, const DL_Attributes& attributes,
                              const ProcessedROIInfo& roiInfo);

    // 绘制样条曲线
    void drawSplines(DL_Dxf& dxf, DL_WriterA* dw, const DL_Attributes& attributes,
                     const ProcessedROIInfo& roiInfo);

    // 绘制缝隙端点
    void drawEndpoints(DL_Dxf& dxf, DL_WriterA* dw, const DL_Attributes& attributes,
                       const ProcessedROIInfo& roiInfo);

    // 绘制所有对位平台坐标系（数据由调用方加载后传入）
    void drawPlatforms(DL_Dxf& dxf, DL_WriterA* dw,
                       const DL_Attributes& attrX,
                       const DL_Attributes& attrY,
                       const DL_Attributes& attrOrigin,
                       const std::vector<PlatformAxis>& platforms);

    // 绘制工件中心到关联平台原点的连线（用于目视校验映射）
    void drawWorkpiecePlatformLinks(DL_Dxf& dxf, DL_WriterA* dw,
                                    const DL_Attributes& attr,
                                    const std::map<int, Eigen::Vector2d>& workpieceCenters,
                                    const std::map<int, int>& workpieceToPlatform,
                                    const std::vector<PlatformAxis>& platforms);
};

#endif // DXF_SAVER_H

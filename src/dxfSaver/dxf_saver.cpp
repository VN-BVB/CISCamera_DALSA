#include <iostream>
#include <plog/Log.h>
#include "dxf_saver.h"
#include "../utils/geometry_utils.h"

const int SPACING_BETWEEN_ROIS = 100; // ROI之间的间距，避免重叠

DXFSaver::DXFSaver() : QObject(nullptr) {}

void DXFSaver::whenAllImagesProcessed(const std::map<int, std::vector<int>>& workpieceToRoiInfos,
                                      const std::map<int, ProcessedROIInfo>& processedRoiInfos)
{
    try {
        // 创建DXF对象和写入器 - 使用简单的文件名，与测试文件类似
        DL_Dxf dxf;
        DL_WriterA* dw = dxf.out("joint_detected.dxf", DL_Codes::AC1015);

        if (!dw || dw->openFailed()) {
            PLOG_ERROR << "无法创建DXF文件!";
            return;
        }

        // 严格按照测试文件的结构和顺序写入DXF头部
        dxf.writeHeader(*dw);
        dw->sectionEnd();

        // 写入表部分
        dw->sectionTables();

        // 写入视口表
        dxf.writeVPort(*dw);

        // 写入线型表 - 完全按照测试文件的方式
        dw->tableLinetypes(1);
        dxf.writeLinetype(*dw, DL_LinetypeData("CONTINUOUS", "Continuous", 0, 0, 0.0));
        dxf.writeLinetype(*dw, DL_LinetypeData("BYLAYER", "", 0, 0, 0.0));
        dxf.writeLinetype(*dw, DL_LinetypeData("BYBLOCK", "", 0, 0, 0.0));
        dw->tableEnd();

        // 写入图层表
        dw->tableLayers(1);
        dxf.writeLayer(
            *dw,
            DL_LayerData("0", 0),
            DL_Attributes("", 1, 0x00ff0000, 15, "CONTINUOUS")
            );
        dw->tableEnd();

        // 写入样式表
        dw->tableStyle(1);
        DL_StyleData style("Standard", 0, 0.0, 1.0, 0.0, 0, 2.5, "txt", "");
        style.bold = false;
        style.italic = false;
        dxf.writeStyle(*dw, style);
        dw->tableEnd();

        // 写入视图表
        dxf.writeView(*dw);

        // 写入UCS表
        dxf.writeUcs(*dw);

        // 写入应用程序ID表
        dw->tableAppid(1);
        dxf.writeAppid(*dw, "ACAD");
        dw->tableEnd();

        // 写入标注样式表
        dxf.writeDimStyle(*dw, 2.5, 0.625, 0.625, 0.625, 2.5);

        // 写入块记录表
        dxf.writeBlockRecord(*dw);
        dw->tableEnd();

        dw->sectionEnd();

        // 写入块部分 - 确保包含所有测试文件中的块
        dw->sectionBlocks();
        dxf.writeBlock(*dw, DL_BlockData("*Model_Space", 0, 0.0, 0.0, 0.0));
        dxf.writeEndBlock(*dw, "*Model_Space");
        dxf.writeBlock(*dw, DL_BlockData("*Paper_Space", 0, 0.0, 0.0, 0.0));
        dxf.writeEndBlock(*dw, "*Paper_Space");
        dxf.writeBlock(*dw, DL_BlockData("*Paper_Space0", 0, 0.0, 0.0, 0.0));
        dxf.writeEndBlock(*dw, "*Paper_Space0");
        dw->sectionEnd();

        // 写入实体部分
        dw->sectionEntities();

        DL_Attributes attributes("0", 256, -1, -1, "BYLAYER");

        // 遍历所有处理过的ROI
        int roiIndex = 0;
        for (const auto& roiPair : processedRoiInfos) {
            const ProcessedROIInfo& roiInfo = roiPair.second;

            // 计算偏移量，避免ROI重叠
            double offsetX = roiIndex * SPACING_BETWEEN_ROIS;

            // 绘制亚像素轮廓
            drawSubpixelContours(dxf, dw, attributes, roiInfo, offsetX);

            roiIndex++;
        }

        // 结束实体部分
        dw->sectionEnd();

        // 写入对象部分 - 确保与测试文件一致
        dxf.writeObjects(*dw, "MY_OBJECTS");
        dxf.writeObjectsEnd(*dw);

        // 结束DXF文件
        dw->dxfEOF();
        dw->close();
        delete dw;

        PLOG_INFO << "DXF文件创建成功: joint_detected.dxf";
    } catch (const std::exception& e) {
        PLOG_ERROR << "创建DXF文件时出错: " << e.what();
    }
}


void DXFSaver::drawSubpixelContours(DL_Dxf& dxf, DL_WriterA* dw, const DL_Attributes& attributes,
                                    const ProcessedROIInfo& roiInfo, double offsetX)
{
    // 检查是否有亚像素轮廓数据
    if (roiInfo.subpixelContours.empty()) {
        PLOG_INFO << "ROI索引 " << roiInfo.index << " 没有亚像素轮廓数据";
        return;
    }

    PLOG_INFO << "处理ROI索引 " << roiInfo.index << "，亚像素轮廓数量: " << roiInfo.subpixelContours.size();

    // 遍历每个亚像素轮廓
    for (size_t contourIdx = 0; contourIdx < roiInfo.subpixelContours.size(); ++contourIdx) {
        const std::vector<cv::Point2f>& contour = roiInfo.subpixelContours[contourIdx];
        PLOG_INFO << "  绘制轮廓 " << contourIdx << "，包含 " << contour.size() << " 个点";

        // 将像素坐标转换为世界坐标
        std::vector<Eigen::Vector2d> worldPoints = GeometryUtils::pixel2World(contour);

        for (size_t i = 0; i < worldPoints.size() - 1; ++i) {
            const Eigen::Vector2d& wp1 = worldPoints[i];
            const Eigen::Vector2d& wp2 = worldPoints[i + 1];

            DL_LineData lineData(
                wp1.y() + offsetX, wp1.x(), 0.0,  // 交换x和y坐标
                wp2.y() + offsetX, wp2.x(), 0.0   // 交换x和y坐标
                );

            // 写入线到DXF文件
            dxf.writeLine(*dw, lineData, attributes);
        }
    }
}

#include "dxf_saver.h"
#include <iostream>

const int SPACING_BETWEEN_ROIS = 100; // ROI之间的间距，避免重叠

DXFSaver::DXFSaver() : QObject(nullptr) {}

void DXFSaver::whenAllImagesProcessed(std::map<int, ProcessedROIInfo> processedRoiInfos)
{
    try {
        // 创建DXF对象和写入器 - 使用简单的文件名，与测试文件类似
        DL_Dxf dxf;
        DL_WriterA* dw = dxf.out("joint_detected.dxf", DL_Codes::AC1015);

        if (!dw || dw->openFailed()) {
            std::cerr << "无法创建DXF文件!" << std::endl;
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

        // 先添加一条测试线，验证基本功能是否正常
        DL_LineData testLineData(10, 5, 0, 30, 5, 0);
        dxf.writeLine(*dw, testLineData, attributes);

        // 遍历所有处理过的ROI
        int roiIndex = 0;
        for (const auto& roiPair : processedRoiInfos) {
            const ProcessedROIInfo& roiInfo = roiPair.second;

            // 计算偏移量，避免ROI重叠
            double offsetX = roiIndex * SPACING_BETWEEN_ROIS;

            // 检查endPoints是否有数据
            if (roiInfo.endPoints.size() >= 2) {
                std::cout << "处理ROI索引 " << roiInfo.index << "，端点数量: " << roiInfo.endPoints.size() << std::endl;

                // 将端点连成线
                for (size_t i = 0; i < roiInfo.endPoints.size() - 1; ++i) {
                    const cv::Point2f& p1 = roiInfo.endPoints[i];
                    const cv::Point2f& p2 = roiInfo.endPoints[i + 1];

                    // 创建线数据，添加偏移量
                    DL_LineData lineData(
                        p1.x + offsetX, p1.y, 0.0,
                        p2.x + offsetX, p2.y, 0.0
                        );

                    // 写入线到DXF文件
                    dxf.writeLine(*dw, lineData, attributes);
                }

                // 如果端点数量大于2，将最后一个点与第一个点连接
                if (roiInfo.endPoints.size() > 2) {
                    const cv::Point2f& pFirst = roiInfo.endPoints[0];
                    const cv::Point2f& pLast = roiInfo.endPoints.back();

                    DL_LineData lineData(
                        pLast.x + offsetX, pLast.y, 0.0,
                        pFirst.x + offsetX, pFirst.y, 0.0
                        );

                    dxf.writeLine(*dw, lineData, attributes);
                }
            }

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

        std::cout << "DXF文件创建成功: joint_detected.dxf" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "创建DXF文件时出错: " << e.what() << std::endl;
    }
}

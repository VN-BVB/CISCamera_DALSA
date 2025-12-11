#include <iostream>
#include <plog/Log.h>
#include "dxf_saver.h"
#include "../utils/geometry_utils.h"

DXFSaver::DXFSaver() : QObject(nullptr) {}

void DXFSaver::whenAllImagesProcessed(const std::map<int, std::vector<int>>& workpieceToRoiInfos,
                                      const std::map<int, ProcessedROIInfo>& processedRoiInfos)
{
    try {
        // 创建DXF对象和写入器
        DL_Dxf dxf;
        DL_WriterA* dw = dxf.out("joint_detected.dxf", DL_Codes::AC1015);

        if (!dw || dw->openFailed()) {
            PLOG_ERROR << "无法创建DXF文件!";
            return;
        }

        // 写入DXF头部
        dxf.writeHeader(*dw);
        dw->sectionEnd();

        // 写入表部分
        dw->sectionTables();

        // 写入视口表
        dxf.writeVPort(*dw);

        // 写入线型表
        dw->tableLinetypes(1);
        dxf.writeLinetype(*dw, DL_LinetypeData("CONTINUOUS", "Continuous", 0, 0, 0.0));
        dxf.writeLinetype(*dw, DL_LinetypeData("BYLAYER", "", 0, 0, 0.0));
        dxf.writeLinetype(*dw, DL_LinetypeData("BYBLOCK", "", 0, 0, 0.0));
        dw->tableEnd();

        // 写入图层表
        dw->tableLayers(3);  // 增加图层数量
        // 0层
        dxf.writeLayer(
            *dw,
            DL_LayerData("0", 0),
            DL_Attributes("", 1, 0x00ff0000, 15, "CONTINUOUS")
            );
        // 亚像素轮廓图层（蓝色）
        dxf.writeLayer(
            *dw,
            DL_LayerData("SubpixelContours", 0),
            DL_Attributes("", 1, 0xffffff, 15, "CONTINUOUS")
            );
        // 样条曲线图层（红色）
        dxf.writeLayer(
            *dw,
            DL_LayerData("Splines", 0),
            DL_Attributes("", 1, 0xff0000, 15, "CONTINUOUS")
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

        // 创建不同图层的属性
        DL_Attributes subpixelAttributes("SubpixelContours", 256, -1, -1, "BYLAYER");
        DL_Attributes splineAttributes("Splines", 256, -1, -1, "BYLAYER");

        // 遍历所有处理过的ROI
        int roiIndex = 0;
        for (const auto& roiPair : processedRoiInfos) {
            const ProcessedROIInfo& roiInfo = roiPair.second;

            // 绘制亚像素轮廓（使用蓝色图层）
            drawSubpixelContours(dxf, dw, subpixelAttributes, roiInfo);

            // 绘制样条曲线（使用红色图层）
            drawSplines(dxf, dw, splineAttributes, roiInfo);

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
                                    const ProcessedROIInfo& roiInfo)
{
    // 检查是否有亚像素轮廓数据
    if (roiInfo.subpixelContours.empty()) {
        PLOG_INFO << "ROI索引 " << roiInfo.index << " 没有亚像素轮廓数据";
        return;
    }

    PLOG_INFO << "处理ROI索引 " << roiInfo.index << "，亚像素轮廓数量: " << roiInfo.subpixelContours.size();

    cv::Mat white_bg = cv::Mat(400, 200, CV_8UC3, cv::Scalar(255, 255, 255));

    // 遍历每个亚像素轮廓
    for (size_t contourIdx = 0; contourIdx < roiInfo.subpixelContours.size(); ++contourIdx) {
        const std::vector<cv::Point2f>& contour = roiInfo.subpixelContours[contourIdx];
        PLOG_INFO << "  绘制轮廓 " << contourIdx << "，包含 " << contour.size() << " 个点";

        // 将像素坐标转换为世界坐标
        std::vector<Eigen::Vector2d> worldPoints = GeometryUtils::pixel2World(contour);

        // 在DXF文件中绘制
        for (size_t i = 0; i < worldPoints.size() - 1; ++i) {
            const Eigen::Vector2d& wp1 = worldPoints[i];
            const Eigen::Vector2d& wp2 = worldPoints[i + 1];

            DL_LineData lineData(
                wp1.y(), wp1.x(), 0.0,  // 交换x和y坐标
                wp2.y(), wp2.x(), 0.0
                );

            // 写入线到DXF文件
            dxf.writeLine(*dw, lineData, attributes);
        }
    }
}

void DXFSaver::drawSplines(DL_Dxf& dxf, DL_WriterA* dw, const DL_Attributes& attributes,
                           const ProcessedROIInfo& roiInfo)
{
    // 检查是否有样条曲线数据
    if (roiInfo.splines.empty()) {
        PLOG_INFO << "ROI索引 " << roiInfo.index << " 没有样条曲线数据";
        return;
    }

    PLOG_INFO << "处理ROI索引 " << roiInfo.index << "，样条曲线数量: " << roiInfo.splines.size();

    // 遍历每个样条曲线
    for (size_t splineIdx = 0; splineIdx < roiInfo.splines.size(); ++splineIdx) {
        const tinyspline::BSpline& spline = roiInfo.splines[splineIdx];
        PLOG_INFO << "  绘制样条曲线 " << splineIdx;

        // 获取样条曲线的控制点
        std::vector<tinyspline::real> controlPoints = spline.controlPoints();
        std::vector<tinyspline::real> knots = spline.knots();

        std::vector<cv::Point2f> pixelPoints;
        for (size_t i = 0; i < controlPoints.size(); i += 2) {
            pixelPoints.emplace_back(controlPoints[i], controlPoints[i+1]);
        }
        std::vector<Eigen::Vector2d> worldPoints = GeometryUtils::pixel2World(pixelPoints);

        // 创建DL_SplineData对象
        DL_SplineData splineData(
            spline.degree(),                            // 次数
            static_cast<int>(knots.size()),             // 节点数量
            static_cast<int>(worldPoints.size()),       // 控制点数量（2D）
            0,                                          // 拟合点数量（0表示没有）
            0                                           // 标志位
            );

        // 写入样条曲线数据
        dxf.writeSpline(*dw, splineData, attributes);

        // 写入控制点
        for (size_t i = 0; i < worldPoints.size(); ++i) {
            const Eigen::Vector2d& worldPoint = worldPoints[i];

            DL_ControlPointData controlPointData(
                worldPoint.y(), worldPoint.x(), 0.0,  // 交换x和y坐标
                1.0  // 权重
                );
            dxf.writeControlPoint(*dw, controlPointData);
        }

        // 写入节点
        for (double knot : knots) {
            DL_KnotData knotData(knot);
            dxf.writeKnot(*dw, knotData);
        }
    }
}

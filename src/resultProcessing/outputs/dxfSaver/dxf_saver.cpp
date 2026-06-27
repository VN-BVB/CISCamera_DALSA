#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <Eigen/Dense>
#include <plog/Log.h>
#include "dxf_saver.h"
#include "src/utils/geometry_utils.h"
#include "src/utils/platform_pose_io.h"
#include "src/utils/scoped_timer.h"

DXFSaver::DXFSaver() : QObject(nullptr) {}

void DXFSaver::whenAllImagesProcessed(const std::map<int, std::vector<int>>& workpieceToRoiInfos,
                                      const std::map<int, ProcessedROIInfo>& processedRoiInfos,
                                      const std::map<int, Eigen::Vector2d>& workpieceCenters,
                                      const std::map<int, int>& workpieceToPlatform,
                                      const std::vector<PlatformAxis>& platforms)
{
    try {
        SCOPED_TIMER("保存dxf文件");
        // 创建DXF对象和写入器
        DL_Dxf dxf;
        DL_WriterA* dw = dxf.out("./data/seamEndpointInfos/joint_detected.dxf", DL_Codes::AC1015);

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
        dw->tableLayers(8);  // 增加图层数量
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
        // 端点图层（黄色）
        dxf.writeLayer(
            *dw,
            DL_LayerData("Endpoints", 0),
            DL_Attributes("", 1, 0xffff00, 15, "CONTINUOUS")
            );
        // 平台 X 轴图层（红色）
        dxf.writeLayer(
            *dw,
            DL_LayerData("PlatformX", 0),
            DL_Attributes("", 1, 0x00ff0000, 15, "CONTINUOUS")
            );
        // 平台 Y 轴图层（绿色）
        dxf.writeLayer(
            *dw,
            DL_LayerData("PlatformY", 0),
            DL_Attributes("", 1, 0x0000ff00, 15, "CONTINUOUS")
            );
        // 平台原点十字图层（黄色）
        dxf.writeLayer(
            *dw,
            DL_LayerData("PlatformOrigin", 0),
            DL_Attributes("", 1, 0x00ffff00, 15, "CONTINUOUS")
            );
        // 工件中心→关联平台原点连线图层（青色）
        dxf.writeLayer(
            *dw,
            DL_LayerData("WorkpiecePlatformLink", 0),
            DL_Attributes("", 1, 0x0000ffff, 15, "CONTINUOUS")
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
        DL_Attributes endpointAttributes("Endpoints", 256, -1, -1, "BYLAYER");
        DL_Attributes platformXAttributes("PlatformX", 256, -1, -1, "BYLAYER");
        DL_Attributes platformYAttributes("PlatformY", 256, -1, -1, "BYLAYER");
        DL_Attributes platformOriginAttributes("PlatformOrigin", 256, -1, -1, "BYLAYER");
        DL_Attributes workpiecePlatformLinkAttributes("WorkpiecePlatformLink", 256, -1, -1, "BYLAYER");

        // 遍历所有处理过的ROI
        {
            SCOPED_TIMER("绘制轮廓");
            int roiIndex = 0;
            for (const auto& roiPair : processedRoiInfos) {

                const ProcessedROIInfo& roiInfo = roiPair.second;

                // 绘制亚像素轮廓（使用蓝色图层）
                // drawSubpixelContours(dxf, dw, subpixelAttributes, roiInfo);

                // 绘制样条曲线（使用红色图层）
                // drawSplines(dxf, dw, splineAttributes, roiInfo);

                // 绘制端点（使用黄色图层）
                drawEndpoints(dxf, dw, endpointAttributes, roiInfo);

                roiIndex++;
            }
        }

        // 平台数据由调用方加载并传入，避免文件二次读取导致与映射数据不一致
        // 绘制所有对位平台坐标系（一次，非每 ROI）
        if (!platforms.empty()) {
            drawPlatforms(dxf, dw, platformXAttributes, platformYAttributes,
                          platformOriginAttributes, platforms);

            // 绘制工件中心→关联平台原点的连线
            drawWorkpiecePlatformLinks(dxf, dw, workpiecePlatformLinkAttributes,
                                       workpieceCenters, workpieceToPlatform, platforms);
        } else {
            PLOG_WARNING << "平台数据为空，跳过平台坐标系和工件→平台连线绘制";
        }
        SCOPED_TIMER("保存dxf文件");
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
                wp1.x(), wp1.y(), 0.0,
                wp2.x(), wp2.y(), 0.0
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
                worldPoint.x(), worldPoint.y(), 0.0,
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

void DXFSaver::drawEndpoints(DL_Dxf& dxf, DL_WriterA* dw, const DL_Attributes& attributes,
                             const ProcessedROIInfo& roiInfo)
{
    // 检查是否有端点数据
    if (roiInfo.endPoints.empty()) {
        PLOG_INFO << "ROI索引 " << roiInfo.index << " 没有端点数据";
        return;
    }

    PLOG_INFO << "处理ROI索引 " << roiInfo.index << "，端点数量: " << roiInfo.endPoints.size();

    // 端点的小圆标记半径（mm），太小在 CAD 里看不清
    constexpr double kEndpointRadius = 0.5;

    // 遍历每个端点
    for (size_t i = 0; i < roiInfo.endPoints.size(); ++i) {
        const SeamEndpoint& ep = roiInfo.endPoints[i];

        // 像素坐标 → 世界坐标
        std::vector<cv::Point2f> pix = { ep.coordinates };
        std::vector<Eigen::Vector2d> worldPoints = GeometryUtils::pixel2World(pix);
        if (worldPoints.empty()) continue;

        const Eigen::Vector2d& wp = worldPoints[0];

        // 用小圆圈标记端点
        DL_CircleData circleData(wp.x(), wp.y(), 0.0, kEndpointRadius);
        dxf.writeCircle(*dw, circleData, attributes);
    }
}

void DXFSaver::drawPlatforms(DL_Dxf& dxf, DL_WriterA* dw,
                             const DL_Attributes& attrX,
                             const DL_Attributes& attrY,
                             const DL_Attributes& attrOrigin,
                             const std::vector<PlatformAxis>& platforms)
{
    // 轴线长度与原点十字半边长（mm）。T 值大致在 0~500mm 范围，30mm 长足以可见又不会拥挤。
    constexpr double kAxisLength = 30.0;
    constexpr double kCrossHalf = 3.0;

    PLOG_INFO << "绘制 " << platforms.size() << " 个对位平台坐标系";

    auto writeLine = [&](const DL_Attributes& attr,
                         const Eigen::Vector3d& a,
                         const Eigen::Vector3d& b) {
        DL_LineData lineData(a.x(), a.y(), 0.0, b.x(), b.y(), 0.0);
        dxf.writeLine(*dw, lineData, attr);
    };

    for (const auto& p : platforms) {
        writeLine(attrX, p.T, p.T + kAxisLength * p.X);
        writeLine(attrY, p.T, p.T + kAxisLength * p.Y);
        writeLine(attrOrigin, p.T - kCrossHalf * p.X, p.T + kCrossHalf * p.X);
        writeLine(attrOrigin, p.T - kCrossHalf * p.Y, p.T + kCrossHalf * p.Y);
    }
}

void DXFSaver::drawWorkpiecePlatformLinks(DL_Dxf& dxf, DL_WriterA* dw,
                                          const DL_Attributes& attr,
                                          const std::map<int, Eigen::Vector2d>& workpieceCenters,
                                          const std::map<int, int>& workpieceToPlatform,
                                          const std::vector<PlatformAxis>& platforms)
{
    if (workpieceCenters.empty() || workpieceToPlatform.empty()) {
        PLOG_INFO << "无工件→平台映射，跳过连线绘制";
        return;
    }

    PLOG_INFO << "绘制 " << workpieceCenters.size() << " 条工件→平台连线";

    for (const auto& [workpieceId, center] : workpieceCenters) {
        auto it = workpieceToPlatform.find(workpieceId);
        if (it == workpieceToPlatform.end()) continue;

        // 按 id 查找平台（id 不保证连续从 0 开始）
        auto pit = std::find_if(platforms.begin(), platforms.end(),
                                [&](const PlatformAxis& p) { return p.id == it->second; });
        if (pit == platforms.end()) {
            PLOG_WARNING << "工件 " << workpieceId << " 关联的平台 id="
                         << it->second << " 在平台列表中找不到";
            continue;
        }

        DL_LineData lineData(center.x(), center.y(), 0.0,
                             pit->T.x(), pit->T.y(), 0.0);
        dxf.writeLine(*dw, lineData, attr);
    }
}

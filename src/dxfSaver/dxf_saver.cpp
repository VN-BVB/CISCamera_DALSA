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

            // 绘制亚像素轮廓
            drawSubpixelContours(dxf, dw, attributes, roiInfo);

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

    // 绘制坐标系：画布左下角为原点，向上为x轴，向右为y轴
    // 原点在(50, 150)，为了在画布中有足够的空间显示
    cv::Point origin(0, 400);

    // 绘制x轴（向上）
    cv::line(white_bg, origin, cv::Point(origin.x, 30), cv::Scalar(0, 0, 255), 2);
    // 绘制y轴（向右）
    cv::line(white_bg, origin, cv::Point(350, origin.y), cv::Scalar(0, 255, 0), 2);

    // 绘制坐标轴标签
    cv::putText(white_bg, "X", cv::Point(origin.x - 10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
    cv::putText(white_bg, "Y", cv::Point(360, origin.y + 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
    cv::putText(white_bg, "Origin", cv::Point(origin.x + 5, origin.y + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);

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

        // 在OpenCV图像上绘制世界坐标点
        // 缩放因子，用于将世界坐标缩放到图像尺寸
        double scaleFactor = 1; // 根据实际情况调整缩放因子
        // cv::line(white_bg,white_bg,)
        // double a =  worldPoints.front().x();
        // std::cout << "======aa=======" << worldPoints.front() << std::endl;
        // std::cout << "======bb=======" << worldPoints.back() << std::endl;

        for (size_t i = 0; i < worldPoints.size(); ++i) {
            const Eigen::Vector2d& wp = worldPoints[i];

            // 转换世界坐标到图像坐标：原点在左下角，向上为x轴，向右为y轴
            // 注意：OpenCV图像坐标系是原点在左上角，向下为y轴，向右为x轴
            // 所以需要进行转换
            int imgX = origin.x + static_cast<int>(wp.y() * scaleFactor); // 世界y轴映射到图像x轴
            int imgY = origin.y - static_cast<int>(wp.x() * scaleFactor); // 世界x轴映射到图像y轴

            // 确保点在图像范围内
            if (imgX >= 0 && imgX < white_bg.cols && imgY >= 0 && imgY < white_bg.rows) {
                // 绘制点
                cv::circle(white_bg, cv::Point(imgX, imgY), 3, cv::Scalar(255, 0, 0), -1);

                // 连接相邻的点
                if (i > 0) {
                    const Eigen::Vector2d& prevWp = worldPoints[i - 1];
                    int prevImgX = origin.x + static_cast<int>(prevWp.y() * scaleFactor);
                    int prevImgY = origin.y - static_cast<int>(prevWp.x() * scaleFactor);

                    if (prevImgX >= 0 && prevImgX < white_bg.cols && prevImgY >= 0 && prevImgY < white_bg.rows) {
                        cv::line(white_bg, cv::Point(prevImgX, prevImgY), cv::Point(imgX, imgY), cv::Scalar(255, 0, 0), 1);
                    }
                }
            }
        }
    }

    // 保存绘制结果图像
    std::string filename = "world_coordinates_visualization_" + std::to_string(roiInfo.index) + ".png";
    cv::imwrite(filename, white_bg);
    PLOG_INFO << "世界坐标可视化图像已保存: " << filename;
}


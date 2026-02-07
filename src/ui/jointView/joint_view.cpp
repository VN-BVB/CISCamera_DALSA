#include <QFileDialog>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QInputDialog>
#include <plog/Log.h>

#include "joint_view.h"
#include "src/config/config_manager.h"
#include "ui_joint_view.h"
#include "src/ui/utils/display/display_scene.h"
#include "src/ui/utils/display/display_manager.h"
#include "src/ui/utils/display/graphicItems/graphic_item_component.h"
#include "src/ui/utils/display/graphicItems/graphic_item_composite.h"
#include "src/ui/utils/display/graphicItems/line_item.h"
#include "src/ui/utils/display/graphicItems/point_item.h"
#include "src/ui/utils/display/graphicItems/bspline_item.h"
#include "src/ui/utils/display/graphicItems/rotated_rect_item.h"
#include "src/ui/utils/display/display_view.h"

JointView::JointView(QWidget *parent)
    : QWidget(parent), 
    ui(new Ui::JointView),
    readWorker(new ImageReadWorker),
    processWorker(new ImageProcessWorker),
    m_showPixelContoursSquare(false)
{
    ui->setupUi(this);
    initRegisterMetaTypes();

    // 读取线程
    readWorker->moveToThread(&readThread);
    connect(&readThread, &QThread::finished, readWorker, &QObject::deleteLater);
    connect(this, &JointView::startImageRead, readWorker, &ImageReadWorker::whenReadImage);
    connect(this, &JointView::startImageReadFromSharedMemory, readWorker, &ImageReadWorker::whenReadImageFromSharedMemory);
    connect(readWorker, &ImageReadWorker::sendImageRead, this, &JointView::handleImageRead);
    connect(readWorker, &ImageReadWorker::sendErrorOccurred, this, &JointView::handleError);
    connect(readWorker, &ImageReadWorker::sendImagesRead, processWorker, &ImageProcessWorker::whenProcessMultiImages);

    // 处理线程
    processWorker->moveToThread(&processThread);
    connect(&processThread, &QThread::finished, processWorker, &QObject::deleteLater);
    connect(this, &JointView::startImageProcess, processWorker, &ImageProcessWorker::whenProcessImage);
    connect(processWorker, &ImageProcessWorker::imageProcessed, this, &JointView::handleImageProcessed);
    connect(processWorker, &ImageProcessWorker::imageProcessedCannyDevenay, this, &JointView::handleImageProcessedCannyDevenay);
    connect(processWorker, &ImageProcessWorker::errorOccurred, this, &JointView::handleError);
    connect(processWorker, &ImageProcessWorker::sendAllImagesProcessed, this, &JointView::whenALLImagesProcessed);

    // 边缘组合
    m_edgeAssembier = std::make_shared<EdgeAssembly>();
    connect(processWorker, &ImageProcessWorker::sendAllImagesProcessed, m_edgeAssembier.get(), &EdgeAssembly::whenAllImagesProcessed);

    // 结果处理器
    m_resultProcessor = std::make_shared<ResultProcessor>();
    connect(m_edgeAssembier.get(), &EdgeAssembly::sendEdgeAssemblyFinished,
            m_resultProcessor.get(), &ResultProcessor::whenEdgeAssemblyFinished);

    // 启动线程
    readThread.start();
    processThread.start();
}

void JointView::initRegisterMetaTypes()
{
    qRegisterMetaType<std::shared_ptr<cv::Mat>>("std::shared_ptr<cv::Mat>");
    qRegisterMetaType<std::shared_ptr<JointSeam>>("std::shared_ptr<JointSeam>");
    qRegisterMetaType<std::shared_ptr<std::vector<ROIWithCoords>>>("std::shared_ptr<std::vector<ROIWithCoords>>");
    qRegisterMetaType<std::map<int, ProcessedROIInfo>>("std::map<int, ProcessedROIInfo>");
}

JointView::~JointView()
{
    readThread.quit();
    readThread.wait();
    processThread.quit();
    processThread.wait();
    delete ui;
}

void JointView::on_pb_open_clicked()
{
    auto appConfig = ConfigManager::getInstance().getConfig();

    QString folderPath = QString::fromStdString(appConfig.image_path_config.default_image_folder);
    QString path = QFileDialog::getOpenFileName(this, "Select Image", folderPath, "(*.png *.jpg *.bmp)");
    // QString path = "E:/work/车门门环拼接/image/背面打光/9/1/6984_5772.bmp";
    if(path.isEmpty())
        return;

    emit startImageRead(path);
}

void JointView::handleImageRead(std::shared_ptr<cv::Mat> image)
{
    emit startImageProcess(image);
}

void JointView::clearAllResultItems()
{
    m_subpixelContours.clear();
    m_pixelContours.clear();
    m_fitTangentLines.clear();
    m_fitCurves.clear();
    m_endPointsByTangentLines.clear();
    m_fitLines.clear();
    m_endPointsByFittedLines.clear();
}

void JointView::handleImageProcessed(std::shared_ptr<cv::Mat> processedImage,
                                     std::shared_ptr<JointSeam> jointSeam)
{
    #include "src/utils/geometry_utils.h"
    // ===============计算两条直线间的距离，测试用===================
    std::vector<std::vector<cv::Point2f>> lines;
    for (auto& cd : jointSeam->getContourDatas()) {
        lines.push_back(cd.getSortedSegments()[2]);
    }

    std::vector<std::vector<Eigen::Vector2d>> worldLines;
    for (auto& line : lines) {
        worldLines.push_back(GeometryUtils::pixel2World(line));
    }
    // Eigen::Vector2d 转回 cv::Point2f 的 lambda 函数
    auto eigenToCvPoints = [](const std::vector<Eigen::Vector2d>& eigenPts) {
        std::vector<cv::Point2f> cvPts;
        cvPts.reserve(eigenPts.size());
        for (const auto& pt : eigenPts) {
            cvPts.emplace_back(static_cast<float>(pt.x()), static_cast<float>(pt.y()));
        }
        return cvPts;
    };
    // 将世界坐标转换为 cv::Point2f 类型
    std::vector<cv::Point2f> up_line_pts = eigenToCvPoints(worldLines[0]);
    std::vector<cv::Point2f> down_line_pts = eigenToCvPoints(worldLines[1]);

    cv::Vec4f up_line;
    cv::fitLine(up_line_pts, up_line, cv::DIST_L2, 0, 0.01, 0.01);
    cv::Vec4f down_line;
    cv::fitLine(down_line_pts, down_line, cv::DIST_L2, 0, 0.01, 0.01);
    double D = 0.0;
    // 随机采样lambda函数，按比例采样
    auto random_sample = [](const auto& src, float ratio=0.3) {
        std::vector<cv::Point2f> sampled;
        if(src.empty()) return sampled;

        std::random_device rd;
        std::mt19937 g(rd());
        std::sample(src.begin(), src.end(), std::back_inserter(sampled),
                    std::max(1, (int)(src.size()*ratio)), g);
        return sampled;
    };

    // 计算平均距离
    auto calc_avg_distance = [&](const std::vector<cv::Point2f>& pts, const cv::Vec4f& line) {
        double sum = 0.0;
        for (auto& pt : pts) {
            sum += std::abs(line[0]*(line[3] - pt.y) - line[1]*(line[2] - pt.x)) /
                   std::sqrt(line[0]*line[0] + line[1]*line[1]);
        }
        return pts.empty() ? 0.0 : sum / pts.size();
    };

    // 从两条直线各取30%的点进行双向计算
    auto sampled_up = random_sample(up_line_pts);
    auto sampled_down = random_sample(down_line_pts);

    double avg_up = calc_avg_distance(sampled_up, down_line);
    double avg_down = calc_avg_distance(sampled_down, up_line);
    D = (avg_up + avg_down) / 2.0;
    // ==================================





    clearAllResultItems();
    m_currentImage = processedImage;

    // contour_processor的结果获取方式
    for (auto& cd : jointSeam->getContourDatas()) {
        m_subpixelContours.push_back(cd.getSortedContour());

        for (auto& [index, curveSeg] : cd.getCurveSegments())
            m_fitCurves.push_back(curveSeg);

        for (auto& line : cd.getTangentLines())
            m_fitTangentLines.push_back(line);

        for (auto& point : cd.getIntersections())
            m_endPointsByTangentLines.push_back(point.coordinates);

        for (auto& point : cd.getIntersections())
            m_endPointsByFittedLines.push_back(point.coordinates);
    }

    // 更新显示
    updateDisplay();
}

// CannyDevenay算法对应槽函数
void JointView::handleImageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves)
{

}

void JointView::handleError(const QString &error)
{
    PLOG_INFO << "错误:" << error;
}

void JointView::whenALLImagesProcessed(const std::map<int, ProcessedROIInfo>& processedRoiInfos)
{
    DisplayManager* displayMgr = ui->gv_image->getDisplayManager();
    if (!displayMgr) return;

    DisplayView* view = displayMgr->displayView();
    DisplayScene* scene = displayMgr->displayScene();
    for (const auto& [key, roiInfo] : processedRoiInfos) {
        if (roiInfo.image && !roiInfo.image->empty()) {
            // 转换并拷贝cv::Mat的数据
            QImage qimg;
            if (roiInfo.image->type() == CV_8UC1) {
                qimg = QImage(roiInfo.image->data, roiInfo.image->cols, roiInfo.image->rows,
                              static_cast<int>(roiInfo.image->step), QImage::Format_Grayscale8).copy();
            } else {
                cv::Mat img_rgb;
                cv::cvtColor(*roiInfo.image, img_rgb, cv::COLOR_BGR2RGB);
                qimg = QImage(img_rgb.data, img_rgb.cols, img_rgb.rows,
                              static_cast<int>(img_rgb.step), QImage::Format_RGB888).copy();
            }
            QString path = "E:/work/车门门环拼接/image/背面打光/5/1/test/" + QString::number(roiInfo.index) + ".bmp";
            if (!qimg.isNull()) {
                qimg.save(path);
                QPoint ptImage(roiInfo.leftCornerPoint.x, roiInfo.leftCornerPoint.y);
                scene->whenAddDisplayImage(qimg, ptImage);
            } else {
                PLOG_ERROR << "Failed to create valid QImage for ROI index: " << roiInfo.index;
            }
        } else {
            PLOG_WARNING << "ROI image is null or empty for index: " << roiInfo.index;
        }
        for (const auto& contourData : roiInfo.contourDatas) {
            // 获取轮廓数据的排序后的轮廓点
            std::vector<cv::Point2f> contour = contourData.getSortedContour();
            if (!contour.empty()) {
                auto contourComponent = std::make_shared<ContourItem>(contour, ContourItem::subpixelContour, Qt::red, 2);
                scene->whenAddGraphicComponent(contourComponent);
                cv::Point2f cvPt = contour[0];
                QPoint pt(qRound(cvPt.x), qRound(cvPt.y));
                scene->whenAddDisplayTextItem(QString::number(contourData.getId()), pt, 20);
            }
        }
    }
    view->whenUpdateDisplayFit();
    PLOG_INFO << "显示所有轮廓";
}

// 更新显示函数
void JointView::updateDisplay() {
    if (!m_currentImage) return;

    // 在主线程中显示图像
    DisplayManager* displayMgr = ui->gv_image->getDisplayManager();
    if (!displayMgr) return;

    DisplayScene* scene = displayMgr->displayScene();
    if (!scene) return;
    ui->gv_image->displayImage(m_currentImage, true);

    // 清除所有现有的图形组件，这样在取消勾选时能移除相关显示
    // &TODO:这里也许还能优化，但现在得忙边缘检测去了
    ui->gv_image->clearAllGraphicComponents();

    if (m_showPixelContoursSquare && !m_pixelContours.empty()) {
        if (!m_subpixelContours[1].empty()) {
            auto pointComponent = std::make_shared<PointItem>(m_subpixelContours[1]);
            ui->gv_image->addGraphicComponent(pointComponent);
        }
    }

    if (m_showPixelContoursLine && !m_pixelContours.empty()) {
        // scene->whenDrawPixelContours(m_pixelContours);
    }

    if (m_showSubpixelContours && !m_subpixelContours.empty()) {
        for (const auto& contour : m_subpixelContours) {
            if (!contour.empty()) {
                auto contourComponent = std::make_shared<ContourItem> (contour, ContourItem::subpixelContour);
                ui->gv_image->addGraphicComponent(contourComponent);
            }
        }
    }

    if (m_showFitLines && !m_fitTangentLines.empty()) {
        for (const auto& line : m_fitTangentLines) {
            auto lineComponent = std::make_shared<LineItem>(line, 0.5, 100, Qt::blue);
            ui->gv_image->addGraphicComponent(lineComponent);
        }
    }

    if (m_showFitCurves && !m_fitCurves.empty()) {
        if (!m_fitCurves.empty()) {
            for (const auto& curve : m_fitCurves) {
                auto splineComponent = std::make_shared<BSplineItem>(curve.getSpline(),
                                                                     QColor(255, 0, 255),
                                                                     0.1,
                                                                     Qt::SolidLine,
                                                                     12.0);
                ui->gv_image->addGraphicComponent(splineComponent);
            }
        }
    }

    if (m_showEndPoints && !m_endPointsByTangentLines.empty()) {
        if (!m_endPointsByTangentLines.empty()) {
            auto pointComponent = std::make_shared<PointItem>(m_endPointsByTangentLines,
                                                              Qt::blue,
                                                              0.5,
                                                              15.0);
            ui->gv_image->addGraphicComponent(pointComponent);
        }
    }
}

// Checkbox槽函数实现
void JointView::on_ckb_pixelContoursSquare_toggled(bool checked) {
    m_showPixelContoursSquare = checked;
    updateDisplay();
}

void JointView::on_ckb_pixelContoursLine_toggled(bool checked) {
    m_showPixelContoursLine = checked;
    updateDisplay();
}

void JointView::on_ckb_subpixelContours_toggled(bool checked) {
    m_showSubpixelContours = checked;
    updateDisplay();
}

void JointView::on_ckb_fitlines_toggled(bool checked) {
    m_showFitLines = checked;
    updateDisplay();
}

void JointView::on_ckb_endPoints_toggled(bool checked) {
    m_showEndPoints = checked;
    updateDisplay();
}

void JointView::on_ckb_fitCurves_toggled(bool checked) {
    m_showFitCurves = checked;
    updateDisplay();
}

void JointView::on_pb_openSharedMemoryImages_clicked()
{
    bool ok;
    int imageSenderProcessID = QInputDialog::getInt(this, tr("输入发送方进程ID"),
                                                    tr("请输入发送共享内存图像的进程ID:"),
                                                    0, 0, 2147483647, 1, &ok);
    if (ok) {
        emit startImageReadFromSharedMemory(imageSenderProcessID, 30000);
    }
}


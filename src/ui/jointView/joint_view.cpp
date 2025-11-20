#include <QFileDialog>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QInputDialog>
#include <plog/Log.h>

#include "joint_view.h"
#include "ui_joint_view.h"
#include "src/ui/utils/display/display_scene.h"
#include "src/ui/utils/display/display_manager.h"
#include "src/ui/utils/display/graphicItems/graphic_item_component.h"
#include "src/ui/utils/display/graphicItems/graphic_item_composite.h"
#include "src/ui/utils/display/graphicItems/line_item.h"
#include "src/ui/utils/display/graphicItems/point_item.h"
#include "src/ui/utils/display/graphicItems/bspline_item.h"
#include "src/ui/utils/display/graphicItems/rotated_rect_item.h"

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

    // 处理线程
    processWorker->moveToThread(&processThread);
    connect(&processThread, &QThread::finished, processWorker, &QObject::deleteLater);
    connect(this, &JointView::startImageProcess, processWorker, &ImageProcessWorker::whenProcessImage);
    connect(processWorker, &ImageProcessWorker::imageProcessed, this, &JointView::handleImageProcessed);
    connect(processWorker, &ImageProcessWorker::imageProcessedCannyDevenay, this, &JointView::handleImageProcessedCannyDevenay);
    connect(processWorker, &ImageProcessWorker::errorOccurred, this, &JointView::handleError);

    // 启动线程
    readThread.start();
    processThread.start();
}

void JointView::initRegisterMetaTypes()
{
    qRegisterMetaType<std::shared_ptr<cv::Mat>>("std::shared_ptr<cv::Mat>");
    qRegisterMetaType<std::shared_ptr<JointSeam>>("std::shared_ptr<JointSeam>");
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
    QString folderPath = "E:/work/车门门环拼接/image/背面打光/9/1";
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
    clearAllResultItems();
    m_currentImage = processedImage;

    // contour_processor的结果获取方式
    for (auto& cd : jointSeam->getContourDatas()) {
        m_subpixelContours.push_back(cd.getSortedContour());

        for (auto& [index, curveSeg] : cd.getCurveSegments())
            m_fitCurves.push_back(curveSeg);

        for (auto& line : cd.getTangentLines())
            m_fitTangentLines.push_back(line);

        for (auto& point : cd.getEndPoints())
            m_endPointsByTangentLines.push_back(point);

        for (auto& point : cd.getEndPoints())
            m_endPointsByFittedLines.push_back(point);
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
                                                              Qt::yellow,
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


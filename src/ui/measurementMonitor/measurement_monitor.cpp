#include "measurement_monitor.h"

#include <QBoxLayout>
#include <QSplitter>

#include "src/jointDetection/image_process_worker.h"
#include "src/jointDetection/image_read_worker.h"
#include "src/jointDetection/joint_seam.h"
#include "src/ui/measurementMonitor/widgets/data_source_panel.h"
#include "src/ui/measurementMonitor/widgets/log_panel.h"
#include "src/ui/measurementMonitor/widgets/result_display_panel.h"
#include "src/ui/utils/display/frm_display.h"
#include "src/ui/utils/display/graphicItems/contour_item.h"
#include "src/ui/utils/display/graphicItems/line_item.h"
#include "src/ui/utils/display/graphicItems/point_item.h"

MeasurementMonitor::MeasurementMonitor(QWidget *parent)
    : QWidget(parent)
    , m_imageDisplay(nullptr)
    , m_dataSourcePanel(nullptr)
    , m_resultPanel(nullptr)
    , m_logPanel(nullptr)
    , m_mainSplitter(nullptr)
    , m_rightSplitter(nullptr)
    , m_readWorker(new ImageReadWorker)
    , m_processWorker(new ImageProcessWorker)
{
    setupUi();
    setupWorkers();
    setupConnections();
}

MeasurementMonitor::~MeasurementMonitor() {
    m_readThread.quit();
    m_readThread.wait();
    m_processThread.quit();
    m_processThread.wait();
}

void MeasurementMonitor::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setHandleWidth(1);

    m_imageDisplay = new FrmVisionDisplay(m_mainSplitter);
    m_imageDisplay->setMinimumSize(600, 400);

    QWidget* rightPanel = new QWidget(m_mainSplitter);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    m_rightSplitter = new QSplitter(Qt::Vertical, rightPanel);
    m_rightSplitter->setHandleWidth(1);

    m_dataSourcePanel = new DataSourcePanel(m_rightSplitter);
    m_dataSourcePanel->setMinimumHeight(200);

    m_resultPanel = new ResultDisplayPanel(m_rightSplitter);
    m_resultPanel->setMinimumHeight(300);

    m_rightSplitter->addWidget(m_dataSourcePanel);
    m_rightSplitter->addWidget(m_resultPanel);
    m_rightSplitter->setStretchFactor(0, 1);
    m_rightSplitter->setStretchFactor(1, 2);

    rightLayout->addWidget(m_rightSplitter);

    m_mainSplitter->addWidget(m_imageDisplay);
    m_mainSplitter->addWidget(rightPanel);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(m_mainSplitter, 1);

    m_logPanel = new LogPanel(this);
    m_logPanel->setFixedHeight(120);
    mainLayout->addWidget(m_logPanel);
}

void MeasurementMonitor::setupWorkers() {
    m_readWorker->moveToThread(&m_readThread);
    m_processWorker->moveToThread(&m_processThread);

    connect(&m_readThread, &QThread::finished, m_readWorker, &QObject::deleteLater);
    connect(&m_processThread, &QThread::finished, m_processWorker, &QObject::deleteLater);

    m_readThread.start();
    m_processThread.start();
}

void MeasurementMonitor::setupConnections() {
    connect(m_readWorker, &ImageReadWorker::sendImageRead, this, &MeasurementMonitor::onImageRead);
    connect(m_readWorker, &ImageReadWorker::sendErrorOccurred, this, &MeasurementMonitor::onError);

    connect(this, &MeasurementMonitor::onImageRead, m_processWorker, &ImageProcessWorker::whenProcessImage);

    connect(m_processWorker, &ImageProcessWorker::imageProcessed, this, &MeasurementMonitor::onImageProcessed);
    connect(m_processWorker, &ImageProcessWorker::errorOccurred, this, &MeasurementMonitor::onError);

    connect(m_dataSourcePanel, &DataSourcePanel::fileSelected, this, &MeasurementMonitor::onFileSelected);
    connect(m_dataSourcePanel, &DataSourcePanel::executeSingleMeasurementRequested,
            this, &MeasurementMonitor::onExecuteSingleMeasurementRequested);

    connect(m_resultPanel, &ResultDisplayPanel::displayOverlayFlagsChanged,
            this, &MeasurementMonitor::onDisplayOverlayFlagsChanged);
}

void MeasurementMonitor::onImageRead(std::shared_ptr<cv::Mat> image) {
    m_currentImage = image;
    m_logPanel->appendLog(QString::fromUtf8("图像读取完成，开始处理..."));
}

void MeasurementMonitor::onImageProcessed(std::shared_ptr<cv::Mat> processedImage,
                                          std::shared_ptr<JointSeam> jointSeam) {
    m_logPanel->appendLog(QString::fromUtf8("图像处理完成"));

    m_imageDisplay->displayImage(processedImage, true);
    m_imageDisplay->clearAllGraphicComponents();

    auto overlayFlags = m_resultPanel->getDisplayOverlayFlags();

    if (overlayFlags.showSubpixelContour) {
        for (const auto& contourData : jointSeam->getContourDatas()) {
            auto contour = contourData.getSortedContour();
            if (!contour.empty()) {
                auto contourItem = std::make_shared<ContourItem>(contour, ContourItem::subpixelContour, Qt::red, 2);
                m_imageDisplay->addGraphicComponent(contourItem);
            }
        }
    }

    if (overlayFlags.showFittedCenterLine) {
        for (const auto& contourData : jointSeam->getContourDatas()) {
            for (const auto& line : contourData.getTangentLines()) {
                auto lineItem = std::make_shared<LineItem>(line, 0.5, 100, Qt::green);
                m_imageDisplay->addGraphicComponent(lineItem);
            }
        }
    }

    if (overlayFlags.showMeasurementPoints) {
        for (const auto& contourData : jointSeam->getContourDatas()) {
            for (const auto& intersection : contourData.getIntersections()) {
                std::vector<cv::Point2f> points = {intersection.coordinates};
                auto pointItem = std::make_shared<PointItem>(points, Qt::blue, 0.5, 10.0);
                m_imageDisplay->addGraphicComponent(pointItem);
            }
        }
    }
}

void MeasurementMonitor::onError(const QString &error) {
    m_logPanel->appendLog(QString::fromUtf8("错误: ") + error);
}

void MeasurementMonitor::onDisplayOverlayFlagsChanged() {
    m_logPanel->appendLog(QString::fromUtf8("显示设置已更新"));
}

void MeasurementMonitor::onFileSelected(const QString &path) {
    m_logPanel->appendLog(QString::fromUtf8("选择文件: ") + path);
}

void MeasurementMonitor::onExecuteSingleMeasurementRequested() {
    m_logPanel->appendLog(QString::fromUtf8("开始执行单次测量..."));
}

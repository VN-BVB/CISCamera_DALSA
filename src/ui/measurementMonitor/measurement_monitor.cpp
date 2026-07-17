#include "measurement_monitor.h"

#include <QBoxLayout>
#include <QSplitter>

#include "src/ui/measurementMonitor/widgets/data_source_panel.h"
#include "src/ui/measurementMonitor/widgets/log_panel.h"
#include "src/ui/measurementMonitor/widgets/result_display_panel.h"

#include "src/ui/utils/display/frm_display.h"
#include "src/ui/utils/display/graphicItems/contour_item.h"
#include "src/ui/utils/display/graphicItems/line_item.h"
#include "src/ui/utils/display/graphicItems/point_item.h"
#include "src/utils/plog_utils.h"

MeasurementMonitor::MeasurementMonitor(QWidget *parent)
    : QWidget(parent)
    , m_imageDisplay(nullptr)
    , m_dataSourcePanel(nullptr)
    , m_resultPanel(nullptr)
    , m_logPanel(nullptr)
    , m_mainSplitter(nullptr)
    , m_rightSplitter(nullptr)
{
    setupUi();
    setupInternalSignals();
}

MeasurementMonitor::~MeasurementMonitor() = default;

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
    m_rightSplitter->setStretchFactor(1, 1);

    rightLayout->addWidget(m_rightSplitter);

    m_mainSplitter->addWidget(m_imageDisplay);
    m_mainSplitter->addWidget(rightPanel);
    m_mainSplitter->setStretchFactor(0, 5);
    m_mainSplitter->setStretchFactor(1, 2);

    mainLayout->addWidget(m_mainSplitter, 1);

    m_logPanel = new LogPanel(this);
    m_logPanel->setFixedHeight(120);
    mainLayout->addWidget(m_logPanel);
}

void MeasurementMonitor::setupInternalSignals() {
    // DataSourcePanel → forwarder signals
    connect(m_dataSourcePanel, &DataSourcePanel::startImageRead,
            this, &MeasurementMonitor::startImageReadRequested);
    connect(m_dataSourcePanel, &DataSourcePanel::connectSharedMemoryRequested,
            [this](int pid) {
                PLOG_INFO << "View: connectSharedMemoryRequested - PID=" << pid;
                emit startImageReadFromSharedMemoryRequested(pid);
            });
    connect(m_dataSourcePanel, &DataSourcePanel::disconnectSharedMemoryRequested,
            this, &MeasurementMonitor::stopSharedMemoryRequested);
    connect(m_dataSourcePanel, &DataSourcePanel::startAutoMeasurementRequested,
            this, &MeasurementMonitor::startAutoMeasurementRequested);
    connect(m_dataSourcePanel, &DataSourcePanel::stopAutoMeasurementRequested,
            this, &MeasurementMonitor::stopAutoMeasurementRequested);
    connect(m_dataSourcePanel, &DataSourcePanel::executeSingleMeasurementRequested,
            this, &MeasurementMonitor::executeSingleMeasurementRequested);

    // ResultDisplayPanel → forwarder signal
    connect(m_resultPanel, &ResultDisplayPanel::displayOverlayFlagsChanged,
            this, &MeasurementMonitor::displayOverlayFlagsChanged);
}

void MeasurementMonitor::setStatus(const QString& status) {
    m_resultPanel->setStatus(status);
}

void MeasurementMonitor::displayMeasurementResult(std::shared_ptr<cv::Mat> image,
                                                  std::shared_ptr<JointSeam> seam) {
    PLOG_INFO << "View: displayMeasurementResult";
    PLOG_INFO << "图像处理完成";

    m_imageDisplay->displayImage(image, true);
    m_imageDisplay->clearAllGraphicComponents();

    auto overlayFlags = m_resultPanel->getDisplayOverlayFlags();

    if (overlayFlags.showSubpixelContour) {
        for (const auto& contourData : seam->getContourDatas()) {
            auto contour = contourData.getSortedContour();
            if (!contour.empty()) {
                auto contourItem = std::make_shared<ContourItem>(contour, ContourItem::subpixelContour, Qt::red, 2);
                m_imageDisplay->addGraphicComponent(contourItem);
            }
        }
    }

    if (overlayFlags.showFittedCenterLine) {
        for (const auto& contourData : seam->getContourDatas()) {
            for (const auto& line : contourData.getTangentLines()) {
                auto lineItem = std::make_shared<LineItem>(line, 0.5, 100, Qt::green);
                m_imageDisplay->addGraphicComponent(lineItem);
            }
        }
    }

    if (overlayFlags.showMeasurementPoints) {
        for (const auto& contourData : seam->getContourDatas()) {
            for (const auto& intersection : contourData.getIntersections()) {
                std::vector<cv::Point2f> points = {intersection.coordinates};
                auto pointItem = std::make_shared<PointItem>(points, Qt::blue, 0.5, 10.0);
                m_imageDisplay->addGraphicComponent(pointItem);
            }
        }
    }
}

void MeasurementMonitor::setConnectionStatus(bool connected) {
    m_dataSourcePanel->setConnectionStatus(connected);
}

void MeasurementMonitor::setFilePath(const QString& path) {
    m_dataSourcePanel->setFilePath(path);
}

void MeasurementMonitor::setMeasurementEnabled(bool enabled) {
    m_dataSourcePanel->setMeasurementEnabled(enabled);
}

void MeasurementMonitor::displayOriginalImage(std::shared_ptr<cv::Mat> image) {
    m_imageDisplay->displayImage(image, true);
    m_imageDisplay->clearAllGraphicComponents();
}

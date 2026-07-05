#include "measurement_presenter.h"

#include "../measurement_monitor.h"
#include "../widgets/log_panel.h"
#include "src/utils/plog_utils.h"

MeasurementPresenter::MeasurementPresenter(MeasurementMonitor* view,
                                         MeasurementPipeline* model,
                                         QObject* parent)
    : QObject(parent)
    , m_view(view)
    , m_model(model)
{
    connectViewToPresenter();
    connectModelToPresenter();
}

void MeasurementPresenter::connectViewToPresenter() {
    // View signals → Presenter slots
    connect(m_view, &MeasurementMonitor::startImageReadRequested,
            this, &MeasurementPresenter::onStartImageReadRequested);
    connect(m_view, &MeasurementMonitor::startImageReadFromSharedMemoryRequested,
            this, &MeasurementPresenter::onStartImageReadFromSharedMemoryRequested);
    connect(m_view, &MeasurementMonitor::stopSharedMemoryRequested,
            this, &MeasurementPresenter::onStopSharedMemoryRequested);
    connect(m_view, &MeasurementMonitor::startAutoMeasurementRequested,
            this, &MeasurementPresenter::onStartAutoMeasurementRequested);
    connect(m_view, &MeasurementMonitor::stopAutoMeasurementRequested,
            this, &MeasurementPresenter::onStopAutoMeasurementRequested);
    connect(m_view, &MeasurementMonitor::executeSingleMeasurementRequested,
            this, &MeasurementPresenter::onExecuteSingleMeasurementRequested);
    connect(m_view, &MeasurementMonitor::displayOverlayFlagsChanged,
            this, &MeasurementPresenter::onDisplayOverlayFlagsChanged);

    PLOG_INFO << "Presenter: connected view signals to presenter slots";
}

void MeasurementPresenter::connectModelToPresenter() {
    // Model signals → Presenter slots
    connect(m_model, &MeasurementPipeline::imageRead,
            this, &MeasurementPresenter::onImageRead);
    connect(m_model, &MeasurementPipeline::measurementCompleted,
            this, &MeasurementPresenter::onMeasurementCompleted);
    connect(m_model, &MeasurementPipeline::errorOccurred,
            this, &MeasurementPresenter::onErrorOccurred);

    PLOG_INFO << "Presenter: connected model signals to presenter slots";
}

void MeasurementPresenter::onStartImageReadRequested(const QString& path) {
    PLOG_INFO << "Presenter: onStartImageReadRequested - " << path.toStdString();
    m_model->readFromFile(path);
}

void MeasurementPresenter::onStartImageReadFromSharedMemoryRequested(int pid) {
    PLOG_INFO << "Presenter: onStartImageReadFromSharedMemoryRequested - PID=" << pid;
    m_model->readFromSharedMemory(pid, 30000);
}

void MeasurementPresenter::onStopSharedMemoryRequested() {
    PLOG_INFO << "Presenter: onStopSharedMemoryRequested - not implemented yet";
    // TODO: Implement disconnect from shared memory
}

void MeasurementPresenter::onStartAutoMeasurementRequested() {
    PLOG_INFO << "Presenter: onStartAutoMeasurementRequested - not implemented yet";
    // TODO: Implement auto measurement loop
}

void MeasurementPresenter::onStopAutoMeasurementRequested() {
    PLOG_INFO << "Presenter: onStopAutoMeasurementRequested - not implemented yet";
    // TODO: Implement stop auto measurement
}

void MeasurementPresenter::onExecuteSingleMeasurementRequested() {
    PLOG_INFO << "Presenter: onExecuteSingleMeasurementRequested";
    if (m_lastImage) {
        m_model->processImage(m_lastImage);
    } else {
        m_view->appendLog(QString::fromUtf8("无图像可重测"), LogPanel::LogLevel::Warning);
    }
}

void MeasurementPresenter::onDisplayOverlayFlagsChanged(const DisplayOverlayFlags& flags) {
    PLOG_INFO << "Presenter: onDisplayOverlayFlagsChanged - showRoiFrame=" << flags.showRoiFrame
               << " showFittedCenterLine=" << flags.showFittedCenterLine;
    // TODO: Refresh display with new overlay flags
    // For now, the flags are stored in ResultDisplayPanel and used during displayMeasurementResult
}

void MeasurementPresenter::onImageRead(std::shared_ptr<cv::Mat> image) {
    PLOG_INFO << "Presenter: onImageRead - caching image and auto-triggering process";
    m_lastImage = image;
    // Automatically trigger processing after image read
    m_model->processImage(image);
}

void MeasurementPresenter::onMeasurementCompleted(std::shared_ptr<cv::Mat> image,
                                                   std::shared_ptr<JointSeam> seam) {
    PLOG_INFO << "Presenter: onMeasurementCompleted - sending to view";
    m_view->displayMeasurementResult(image, seam);
}

void MeasurementPresenter::onErrorOccurred(const QString& msg) {
    PLOG_ERROR << "Presenter: onErrorOccurred - " << msg.toStdString();
    m_view->appendLog(msg, LogPanel::LogLevel::Error);
    m_view->setStatus(QString::fromUtf8("ERROR"));
}

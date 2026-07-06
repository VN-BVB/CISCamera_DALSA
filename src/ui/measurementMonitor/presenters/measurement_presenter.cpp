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
    m_view->setMeasurementEnabled(false);
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
}

void MeasurementPresenter::connectModelToPresenter() {
    // Model signals → Presenter slots
    connect(m_model, &MeasurementPipeline::imageRead,
            this, &MeasurementPresenter::onImageRead);
    connect(m_model, &MeasurementPipeline::measurementCompleted,
            this, &MeasurementPresenter::onMeasurementCompleted);
    connect(m_model, &MeasurementPipeline::errorOccurred,
            this, &MeasurementPresenter::onErrorOccurred);
}

void MeasurementPresenter::onStartImageReadRequested(const QString& path) {
    PLOG_INFO << "Presenter: onStartImageReadRequested - " << path.toStdString();
    m_view->setMeasurementEnabled(false);
    m_view->setStatus(QString::fromUtf8("读取中..."));
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
    PLOG_INFO << "开始处理单张图像";
    if (!m_lastImage) {
        PLOG_WARNING << "Presenter: execute requested but no image loaded";
        m_view->appendLog(QString::fromUtf8("请先选择图像文件"), LogPanel::LogLevel::Warning);
        return;
    }
    m_view->setMeasurementEnabled(false);
    m_view->setStatus(QString::fromUtf8("测量中..."));
    m_model->processImage(m_lastImage);
}

void MeasurementPresenter::onDisplayOverlayFlagsChanged(const DisplayOverlayFlags& flags) {
    PLOG_INFO << "Presenter: onDisplayOverlayFlagsChanged - showRoiFrame=" << flags.showRoiFrame
               << " showFittedCenterLine=" << flags.showFittedCenterLine;
    // TODO: Refresh display with new overlay flags
    // For now, the flags are stored in ResultDisplayPanel and used during displayMeasurementResult
}

void MeasurementPresenter::onImageRead(std::shared_ptr<cv::Mat> image) {
    m_lastImage = image;
    m_view->displayOriginalImage(image);
    m_view->setMeasurementEnabled(true);
    m_view->setStatus(QString::fromUtf8("就绪 - 点击执行测量"));
}

void MeasurementPresenter::onMeasurementCompleted(std::shared_ptr<cv::Mat> image,
                                                   std::shared_ptr<JointSeam> seam) {
    PLOG_INFO << "Presenter: onMeasurementCompleted - sending to view";
    m_view->displayMeasurementResult(image, seam);
    m_view->setMeasurementEnabled(true);
    m_view->setStatus(QString::fromUtf8("测量完成"));
}

void MeasurementPresenter::onErrorOccurred(const QString& msg) {
    PLOG_ERROR << "Presenter: onErrorOccurred - " << msg.toStdString();
    m_view->appendLog(msg, LogPanel::LogLevel::Error);
    m_view->setStatus(QString::fromUtf8("ERROR"));
    m_view->setMeasurementEnabled(m_lastImage != nullptr);
}

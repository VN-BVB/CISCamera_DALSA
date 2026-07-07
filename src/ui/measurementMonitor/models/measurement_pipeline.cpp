#include "measurement_pipeline.h"

#include "src/utils/plog_utils.h"

MeasurementPipeline::MeasurementPipeline(QObject* parent)
    : QObject(parent)
    , m_readWorker(new ImageReadWorker)
    , m_processWorker(new ImageProcessWorker)
{
    registerMetaTypes();
    setupWorkers();
}

MeasurementPipeline::~MeasurementPipeline() {
    m_readThread.quit();
    m_readThread.wait();
    m_processThread.quit();
    m_processThread.wait();
}

void MeasurementPipeline::registerMetaTypes() {
    qRegisterMetaType<std::shared_ptr<cv::Mat>>("std::shared_ptr<cv::Mat>");
    qRegisterMetaType<std::shared_ptr<JointSeam>>("std::shared_ptr<JointSeam>");
    qRegisterMetaType<std::shared_ptr<std::vector<ROIWithCoords>>>("std::shared_ptr<std::vector<ROIWithCoords>>");
    qRegisterMetaType<std::map<int, ProcessedROIInfo>>("std::map<int, ProcessedROIInfo>");
}

void MeasurementPipeline::setupWorkers() {
    // Read worker
    m_readWorker->moveToThread(&m_readThread);
    connect(&m_readThread, &QThread::finished, m_readWorker, &QObject::deleteLater);

    // Read worker → pipeline
    connect(m_readWorker, &ImageReadWorker::sendImageRead, this, &MeasurementPipeline::onWorkerImageRead);
    connect(m_readWorker, &ImageReadWorker::sendImagesRead, this, &MeasurementPipeline::onWorkerImagesRead);
    connect(m_readWorker, &ImageReadWorker::sendErrorOccurred, this, &MeasurementPipeline::onWorkerError);

    // Process worker
    m_processWorker->moveToThread(&m_processThread);
    connect(&m_processThread, &QThread::finished, m_processWorker, &QObject::deleteLater);

    // Process worker → pipeline
    connect(m_processWorker, &ImageProcessWorker::imageProcessed, this, &MeasurementPipeline::onWorkerImageProcessed);
    connect(m_processWorker, &ImageProcessWorker::errorOccurred, this, &MeasurementPipeline::onWorkerError);

    // Read worker → process worker (shared memory multi-ROI path)
    connect(m_readWorker, &ImageReadWorker::sendImagesRead, m_processWorker, &ImageProcessWorker::whenProcessMultiImages);

    m_readThread.start();
    m_processThread.start();

    PLOG_INFO << "测量处理管道初始化成功";
}

void MeasurementPipeline::readFromFile(const QString& path) {
    QMetaObject::invokeMethod(m_readWorker, "whenReadImage",
                              Q_ARG(const QString&, path));
}

void MeasurementPipeline::readFromSharedMemory(int processId, int timeoutMs) {
    PLOG_INFO << "Pipeline: readFromSharedMemory - PID=" << processId << " timeout=" << timeoutMs;
    QMetaObject::invokeMethod(m_readWorker, "whenReadImageFromSharedMemory",
                              Q_ARG(int, processId), Q_ARG(int, timeoutMs));
}

void MeasurementPipeline::processImage(std::shared_ptr<cv::Mat> image) {
    PLOG_INFO << "Pipeline: processImage";
    QMetaObject::invokeMethod(m_processWorker, "whenProcessImage",
                              Q_ARG(std::shared_ptr<cv::Mat>, image));
}

void MeasurementPipeline::onWorkerImageRead(std::shared_ptr<cv::Mat> image) {
    // PLOG_INFO << "Pipeline: onWorkerImageRead - emitting imageRead";
    emit imageRead(image);
}

void MeasurementPipeline::onWorkerImagesRead(std::shared_ptr<std::vector<ROIWithCoords>> rois) {
    PLOG_INFO << "Pipeline: onWorkerImagesRead - " << (rois ? rois->size() : 0) << " ROIs";
    // For shared memory multi-ROI, process worker handles it directly
    // Results will come through onWorkerImageProcessed
}

void MeasurementPipeline::onWorkerImageProcessed(std::shared_ptr<cv::Mat> image,
                                                std::shared_ptr<JointSeam> seam) {
    PLOG_INFO << "Pipeline: onWorkerImageProcessed - emitting measurementCompleted";
    emit measurementCompleted(image, seam);
}

void MeasurementPipeline::onWorkerError(const QString& msg) {
    PLOG_ERROR << "Pipeline: onWorkerError - " << msg.toStdString();
    emit errorOccurred(msg);
}

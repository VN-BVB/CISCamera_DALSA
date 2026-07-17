#ifndef MEASUREMENT_PIPELINE_H
#define MEASUREMENT_PIPELINE_H

#include <QObject>
#include <QThread>
#include <memory>

#include "src/jointDetection/image_read_worker.h"
#include "src/jointDetection/image_process_worker.h"
#include "src/jointDetection/joint_seam.h"

class MeasurementPipeline : public QObject {
    Q_OBJECT

public:
    explicit MeasurementPipeline(QObject* parent = nullptr);
    ~MeasurementPipeline() override;

    void readFromFile(const QString& path);
    void readFromSharedMemory(int processId, int timeoutMs = 30000);
    void processImage(std::shared_ptr<cv::Mat> image);

signals:
    void imageRead(std::shared_ptr<cv::Mat> image);
    void measurementCompleted(std::shared_ptr<cv::Mat> image,
                              std::shared_ptr<JointSeam> seam);
    void errorOccurred(const QString& message);

private slots:
    void onWorkerImageRead(std::shared_ptr<cv::Mat> image);
    void onWorkerImagesRead(std::shared_ptr<std::vector<ROIWithCoords>> rois);
    void onWorkerImageProcessed(std::shared_ptr<cv::Mat> image,
                                std::shared_ptr<JointSeam> seam);
    void onWorkerError(const QString& msg);

private:
    void setupWorkers();
    void registerMetaTypes();

    QThread m_readThread;
    QThread m_processThread;
    ImageReadWorker*    m_readWorker{nullptr};
    ImageProcessWorker* m_processWorker{nullptr};
};

#endif  // MEASUREMENT_PIPELINE_H

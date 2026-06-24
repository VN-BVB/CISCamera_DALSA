#ifndef MEASUREMENT_MONITOR_H
#define MEASUREMENT_MONITOR_H

#include <QThread>
#include <QWidget>
#include <memory>
#include <opencv2/core/core.hpp>

class FrmVisionDisplay;
class ImageReadWorker;
class ImageProcessWorker;
class JointSeam;
class DataSourcePanel;
class ResultDisplayPanel;
class LogPanel;
class QSplitter;

class MeasurementMonitor : public QWidget {
    Q_OBJECT

public:
    explicit MeasurementMonitor(QWidget *parent = nullptr);
    ~MeasurementMonitor();

private:
    void setupUi();
    void setupWorkers();
    void setupConnections();

private slots:
    void onImageRead(std::shared_ptr<cv::Mat> image);
    void onImageProcessed(std::shared_ptr<cv::Mat> processedImage,
                          std::shared_ptr<JointSeam> jointSeam);
    void onError(const QString &error);
    void onDisplayOverlayFlagsChanged();
    void onFileSelected(const QString &path);
    void onExecuteSingleMeasurementRequested();

private:
    FrmVisionDisplay* m_imageDisplay;
    DataSourcePanel* m_dataSourcePanel;
    ResultDisplayPanel* m_resultPanel;
    LogPanel* m_logPanel;
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;

    QThread m_readThread;
    QThread m_processThread;
    ImageReadWorker* m_readWorker;
    ImageProcessWorker* m_processWorker;

    std::shared_ptr<cv::Mat> m_currentImage;
};

#endif  // MEASUREMENT_MONITOR_H

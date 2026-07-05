#ifndef MEASUREMENT_MONITOR_H
#define MEASUREMENT_MONITOR_H

#include <QWidget>
#include <memory>

#include "src/jointDetection/joint_seam.h"
#include "src/ui/measurementMonitor/widgets/log_panel.h"
#include "src/ui/measurementMonitor/widgets/result_display_panel.h"

class FrmVisionDisplay;
class DataSourcePanel;
class ResultDisplayPanel;
class QSplitter;

class MeasurementMonitor : public QWidget {
    Q_OBJECT

public:
    explicit MeasurementMonitor(QWidget *parent = nullptr);
    ~MeasurementMonitor();

    // Presenter → View: display commands
    void setStatus(const QString& status);
    void appendLog(const QString& msg, LogPanel::LogLevel level = LogPanel::LogLevel::Info);
    void displayMeasurementResult(std::shared_ptr<cv::Mat> image,
                                  std::shared_ptr<JointSeam> seam);
    void setConnectionStatus(bool connected);
    void setFilePath(const QString& path);

signals:
    // View → Presenter: user operations (forwarder signals from widgets)
    void startImageReadRequested(const QString& path);
    void startImageReadFromSharedMemoryRequested(int pid);
    void stopSharedMemoryRequested();
    void startAutoMeasurementRequested();
    void stopAutoMeasurementRequested();
    void executeSingleMeasurementRequested();
    void displayOverlayFlagsChanged(const DisplayOverlayFlags& flags);

private:
    void setupUi();
    void setupInternalSignals();  // Connect widget signals to forwarder signals

private:
    FrmVisionDisplay* m_imageDisplay;
    DataSourcePanel* m_dataSourcePanel;
    ResultDisplayPanel* m_resultPanel;
    LogPanel* m_logPanel;
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;
};

#endif  // MEASUREMENT_MONITOR_H

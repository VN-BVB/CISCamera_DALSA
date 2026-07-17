#ifndef MEASUREMENT_PRESENTER_H
#define MEASUREMENT_PRESENTER_H

#include <QObject>
#include <memory>

#include "../models/measurement_pipeline.h"
#include "../widgets/result_display_panel.h"

class MeasurementMonitor;

class MeasurementPresenter : public QObject {
    Q_OBJECT

public:
    explicit MeasurementPresenter(MeasurementMonitor* view,
                                 MeasurementPipeline* model,
                                 QObject* parent = nullptr);

private slots:
    // View → Presenter
    void onStartImageReadRequested(const QString& path);
    void onStartImageReadFromSharedMemoryRequested(int pid);
    void onStopSharedMemoryRequested();
    void onStartAutoMeasurementRequested();
    void onStopAutoMeasurementRequested();
    void onExecuteSingleMeasurementRequested();
    void onDisplayOverlayFlagsChanged(const DisplayOverlayFlags& flags);

    // Model → Presenter
    void onImageRead(std::shared_ptr<cv::Mat> image);
    void onMeasurementCompleted(std::shared_ptr<cv::Mat> image,
                                std::shared_ptr<JointSeam> seam);
    void onErrorOccurred(const QString& msg);

private:
    void connectViewToPresenter();
    void connectModelToPresenter();

    MeasurementMonitor* m_view;
    MeasurementPipeline* m_model;
    std::shared_ptr<cv::Mat> m_lastImage;
};

#endif  // MEASUREMENT_PRESENTER_H

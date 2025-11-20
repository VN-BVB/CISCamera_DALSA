#ifndef IMAGE_PROCESS_WORKER_H
#define IMAGE_PROCESS_WORKER_H

#include <QObject>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "edgeDetection/canny_devernay.h"
#include "contourProcess/methods/curve_seg.h"
#include "joint_seam.h"
#include "image_read_worker.h"

class ImageProcessWorker : public QObject {
    Q_OBJECT
public:
    explicit ImageProcessWorker(QObject *parent = nullptr);
    ~ImageProcessWorker();

public slots:
    void whenProcessImage(std::shared_ptr<cv::Mat> image);
    void whenProcessMultiImages(std::shared_ptr<std::vector<ROIWithCoords>> rois);

signals:
    void imageProcessed(std::shared_ptr<cv::Mat> processedImage, std::shared_ptr<JointSeam> jointSeam);
    void imageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves);
    void errorOccurred(const QString &error);
    void allImagesProcessed();

private:
    // 处理单个ROI图像的方法
    void processSingleROI(const ROIWithCoords &roi);

    ThreadPool *m_threadPool;
    std::atomic<int> m_processedCount;
    int m_totalROICount;
    std::mutex m_mutex;
};

#endif  // IMAGE_PROCESS_WORKER_H

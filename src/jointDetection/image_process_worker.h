#ifndef IMAGE_PROCESS_WORKER_H
#define IMAGE_PROCESS_WORKER_H

#include <QObject>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "edgeDetection/canny_devernay.h"
#include "contourProcess/methods/curve_seg.h"
#include "joint_seam.h"
#include "image_read_worker.h"

// 处理结果结构体
struct ProcessedROIResult
{
    std::shared_ptr<cv::Mat> image;
    std::shared_ptr<JointSeam> jointSeam;
    int x;
    int y;
};

class ImageProcessWorker : public QObject {
    Q_OBJECT
public:
    explicit ImageProcessWorker(QObject *parent = nullptr);
    ~ImageProcessWorker();

    std::vector<ProcessedROIResult> getAllProcessedResults();
    void clearProcessedResults();

public slots:
    void whenProcessImage(std::shared_ptr<cv::Mat> image);
    void whenProcessMultiImages(std::shared_ptr<std::vector<ROIWithCoords>> rois);

signals:
    void imageProcessed(std::shared_ptr<cv::Mat> processedImage, std::shared_ptr<JointSeam> jointSeam);
    void imageProcessedCannyDevenay(std::shared_ptr<cv::Mat> processedImage, std::vector<Point2fCurve> edgeCurves);
    void errorOccurred(const QString &error);
    void allImagesProcessed(std::vector<ProcessedROIResult> processedResults);
    void singleROIProcessed(const ProcessedROIResult &result);

private:
    // 处理单个ROI图像的方法
    void processSingleROI(const ROIWithCoords &roi);

    ThreadPool *m_threadPool;
    std::atomic<int> m_processedCount;
    int m_totalROICount;
    std::mutex m_mutex;
    std::vector<ProcessedROIResult> m_processedResults;
};

#endif  // IMAGE_PROCESS_WORKER_H

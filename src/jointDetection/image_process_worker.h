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

// 每个ROI结果结构体
struct ProcessedROIInfo
{
    std::shared_ptr<cv::Mat> image;                                              // ROI图像
    cv::Point leftCornerPoint;                                // ROI左上角
    std::vector<std::vector<cv::Point>> pixelContours;          // 缝隙两条像素轮廓坐标
    std::vector<std::vector<cv::Point2f>> subpixelContour;      // 缝隙两条亚像素轮廓坐标
    std::vector<cv::Vec4f> lines;                               // 缝隙所有拟合直线
    std::vector<cv::Vec4f> seamLines;                           // 缝隙两侧两条直线
    std::vector<cv::Point2f> endPoints;                         // 缝隙的四个端点
    std::vector<tinyspline::BSpline> splines;                   // 缝隙所有拟合样条曲线
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
    std::mutex m_roiInfoMutex;  // 保护ROI信息字典的互斥锁
    std::map<int, ProcessedROIInfo> m_processedRoiInfos;  // 字典，键为ROI左上角坐标
};

#endif  // IMAGE_PROCESS_WORKER_H

#include "image_process_worker.h"
#include "src/utils/image_tools.h"
#include "src/test/test_curve_seg.cpp"
#include <plog/Log.h>
#include <future>

ImageProcessWorker::ImageProcessWorker(QObject *parent) : QObject{parent}
{
    int threadCount = std::thread::hardware_concurrency();
    m_threadPool = new ThreadPool(threadCount > 0 ? threadCount : 9);
    m_processedCount = 0;
    m_totalROICount = 0;
}

ImageProcessWorker::~ImageProcessWorker()
{
    delete m_threadPool;
}

void ImageProcessWorker::whenProcessImage(std::shared_ptr<cv::Mat> image) {
    try {
        PLOG_INFO << "===================开始处理数据:=================== ";
        ImageTools imageTools;
        // cv::Mat croppedImg = (*image)(cv::Rect(5696, 7273, 180, 2083));
        // cv::imwrite("D:/Cpp_Project/WeldseamMeasurement/tests/image/cropped_img.bmp", croppedImg);
        cv::Mat croppedImg = *image; // 直接读裁剪后的图，不用再裁剪

        auto jointSeam = std::make_shared<JointSeam>(croppedImg);
        jointSeam->run();
        PLOG_INFO << "===================结束处理数据:=================== ";
        emit imageProcessed(image, jointSeam);
    } catch (const cv::Exception &e) {
        emit errorOccurred(QString("处理图像时出错: ") + e.what());
    }
}

void ImageProcessWorker::whenProcessMultiImages(std::shared_ptr<std::vector<ROIWithCoords>> rois)
{
    try {
        PLOG_INFO << "===================开始处理多张ROI图像:=================== ";

        // 重置计数器
        m_processedCount = 0;
        m_totalROICount = rois->size();

        // 如果没有ROI图像，直接返回
        if (m_totalROICount == 0) {
            PLOG_INFO << "没有ROI图像需要处理";
            emit allImagesProcessed();
            return;
        }

        PLOG_INFO << "共有" << m_totalROICount << "张ROI图像需要处理";

        // 使用线程池并行处理每张ROI图像
        std::vector<std::future<void>> futures;
        for (const auto &roi : *rois) {
            auto future = m_threadPool->enqueue(&ImageProcessWorker::processSingleROI, this, roi);
            futures.push_back(std::move(future));
        }

        // 等待所有任务完成（可选，如果需要同步的话）
        // 这里我们选择异步处理，通过信号通知完成
    } catch (const std::exception &e) {
        emit errorOccurred(QString("处理多张ROI图像时出错: ") + e.what());
    }
}

void ImageProcessWorker::processSingleROI(const ROIWithCoords &roi)
{
    try {
        PLOG_INFO << "开始处理ROI图像 (x:" << roi.x << ", y:" << roi.y << ")";

        // 创建图像的共享指针，以便在信号中传递
        auto imagePtr = std::make_shared<cv::Mat>(roi.image.clone());

        auto jointSeam = std::make_shared<JointSeam>(*imagePtr);
        jointSeam->run();

        PLOG_INFO << "完成处理ROI图像 (x:" << roi.x << ", y:" << roi.y << ")";

        // 更新处理完成的计数
        int processed = ++m_processedCount;

        // 检查是否所有图像都已处理完成
        if (processed == m_totalROICount) {
            PLOG_INFO << "===================所有ROI图像处理完成:=================== ";
            emit allImagesProcessed();
        }
    } catch (const cv::Exception &e) {
        // 注意：在工作线程中发送信号需要确保线程安全
        emit errorOccurred(QString("处理ROI图像时出错 (x:") +
                           QString::number(roi.x) + ", y:" +
                           QString::number(roi.y) + "): " +
                           e.what());
    }
}

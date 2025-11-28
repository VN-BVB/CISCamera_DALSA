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

        auto jointSeam = std::make_shared<JointSeam>(croppedImg, cv::Point2f(0,0));
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
            emit sendAllImagesProcessed(m_processedRoiInfos);
            return;
        }

        PLOG_INFO << "共有" << m_totalROICount << "张ROI图像需要处理";

        // 使用线程池并行处理每张ROI图像
        std::vector<std::future<void>> futures;
        for (const auto &roi : *rois) {
            auto future = m_threadPool->enqueue(&ImageProcessWorker::processSingleROI, this, roi);
            futures.push_back(std::move(future));
        }

        PLOG_INFO << "所有图像处理任务已提交到线程池";
    } catch (const std::exception &e) {
        emit errorOccurred(QString("处理多张ROI图像时出错: ") + e.what());
    }
}

void ImageProcessWorker::processSingleROI(const ROIWithCoords &roi)
{
    try {
        PLOG_INFO << "开始处理ROI图像 (x:" << roi.x << ", y:" << roi.y << ")";

        auto imagePtr = std::make_shared<cv::Mat>(roi.image.clone());
        cv::Point2f leftUPPoint = cv::Point2f{static_cast<float>(roi.x), static_cast<float>(roi.y)};
        auto jointSeam = std::make_shared<JointSeam>(*imagePtr, leftUPPoint);
        jointSeam->run();

        PLOG_INFO << "完成处理ROI图像 (x:" << roi.x << ", y:" << roi.y << ")";

        ProcessedROIInfo resInfo;
        resInfo.index = m_totalROICount;
        resInfo.image = imagePtr;
        resInfo.leftCornerPoint = cv::Point2f(roi.x, roi.y);
        resInfo.contourDatas = jointSeam->getContourDatas();
        for (const auto& contour : jointSeam->getContourDatas()) {
            resInfo.pixelContours .push_back(contour.getPixelContour());
            resInfo.subpixelContour.push_back(contour.getSubpixelContour());
            for (const auto& [i, curSeg] : contour.getCurveSegments()) {
                resInfo.splines.push_back(curSeg.getSpline());
            }
        }
        resInfo.lines = jointSeam->getLines();
        resInfo.endPoints = jointSeam->getEndPoints();

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_processedRoiInfos[m_processedCount] = resInfo;
            PLOG_DEBUG << "已添加处理结果，当前结果数量：" << m_processedCount;
        }
        emit singleROIProcessed(resInfo);

        // 更新处理完成的计数
        int processed = ++m_processedCount;

        // 检查是否所有图像都已处理完成
        if (processed == m_totalROICount) {
            PLOG_INFO << "===================所有ROI图像处理完成:=================== ";
            emit sendAllImagesProcessed(m_processedRoiInfos);
        }
    } catch (const cv::Exception &e) {
        // 注意：在工作线程中发送信号需要确保线程安全
        emit errorOccurred(QString("处理ROI图像时出错 (x:") +
                           QString::number(roi.x) + ", y:" +
                           QString::number(roi.y) + "): " +
                           e.what());
    }
}

std::map<int, ProcessedROIInfo> ImageProcessWorker::getAllProcessedResults()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_processedRoiInfos;
}

void ImageProcessWorker::clearProcessedResults()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_processedRoiInfos.clear();
}

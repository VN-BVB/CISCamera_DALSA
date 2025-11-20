#include "image_read_worker.h"
#include <plog/Log.h>

ImageReadWorker::ImageReadWorker(QObject *parent) : QObject{parent},
    sharedMemory(nullptr),
    dataAvailableSemaphore(nullptr),
    dataReadSemaphore(nullptr),
    m_threadPool(new ThreadPool(4))
{}

ImageReadWorker::~ImageReadWorker()
{
    delete m_threadPool;
    cleanup();
}

void ImageReadWorker::cleanup()
{
    if (sharedMemory)
    {
        if (sharedMemory->isAttached())
        {
            sharedMemory->detach();
        }
        delete sharedMemory;
        sharedMemory = nullptr;
    }

    if (dataAvailableSemaphore)
    {
        delete dataAvailableSemaphore;
        dataAvailableSemaphore = nullptr;
    }

    if (dataReadSemaphore)
    {
        delete dataReadSemaphore;
        dataReadSemaphore = nullptr;
    }
}

bool ImageReadWorker::connectToSender(int processId)
{
    QString processIdStr = QString::number(processId);

    // 清理之前的资源
    cleanup();

    // 创建信号量对象（注意：接收方不使用Create标志，而是打开已存在的信号量）
    dataAvailableSemaphore = new QSystemSemaphore("ROI_Semaphore_" + processIdStr + "_DataAvailable");
    dataReadSemaphore = new QSystemSemaphore("ROI_Semaphore_" + processIdStr + "_DataRead");

    // 创建共享内存对象
    sharedMemory = new QSharedMemory("ROI_Shared_Memory_" + processIdStr);

    // 尝试连接到共享内存
    if (!sharedMemory->attach())
    {
        emit sendErrorOccurred(QString("Failed to attach to shared memory for process %1: %2").arg(processId).arg(sharedMemory->errorString()));
        cleanup();
        return false;
    }

    return true;
}

void ImageReadWorker::whenReadImage(const QString &path) {
    try {
        cv::Mat img = cv::imread(path.toStdString(), cv::IMREAD_GRAYSCALE);
        if (img.empty()) {
            emit sendErrorOccurred("无法加载图像");
            return;
        }
        auto imagePtr = std::make_shared<cv::Mat>(img);
        emit sendImageRead(imagePtr);
    }
    catch(const std::exception& e){
        emit sendErrorOccurred(QString("读取图像出错：") + e.what());
    }
}

void ImageReadWorker::whenReadImageFromSharedMemory(int processId, int timeoutMs) {
    try {
        // 连接到发送方进程
        if (!connectToSender(processId)) {
            emit sendErrorOccurred(QString("连接到发送方进程%1失败").arg(processId));
            return;
        }

        // 等待并读取ROIs
        std::vector<cv::Mat> rois = waitAndReadROIs(timeoutMs);
        int i = 0;
        for (auto& roi : rois) {
            std::string imagePath = "E:/work/车门门环拼接/image/共享内存测试/" + std::to_string(i++) + ".bmp";
            cv::imwrite(imagePath, roi);
        }

        if (rois.empty()) {
            emit sendErrorOccurred("没有从共享内存中读取到图像数据");
            return;
        }

        // 转换为shared_ptr并发出信号
        std::vector<std::shared_ptr<cv::Mat>> images;
        for (auto& roi : rois) {
            images.push_back(std::make_shared<cv::Mat>(roi));
        }

        emit sendImagesRead(images);
    }
    catch(const std::exception& e) {
        emit sendErrorOccurred(QString("从共享内存读取图像出错：") + e.what());
    }
}

std::vector<cv::Mat> ImageReadWorker::waitAndReadROIs(int timeoutMs)
{
    std::vector<cv::Mat> rois;

    if (!sharedMemory || !sharedMemory->isAttached() ||
        !dataAvailableSemaphore || !dataReadSemaphore)
    {
        emit sendErrorOccurred("未正确连接到发送方进程");
        return rois;
    }

    // 实现带超时的信号量获取
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;

    // 使用原子变量来确保线程安全
    std::atomic<bool> dataReceived(false);

    // 启动定时器
    timer.start(timeoutMs);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    // 使用线程池执行信号量获取任务
    auto future = m_threadPool->enqueue([&]() {
        bool result = dataAvailableSemaphore->acquire();
        dataReceived = result;
        loop.quit();
        return result;
    });
    loop.exec();    // 等待获取信号量或超时

    // 检查是否超时
    if (timer.isActive()) {
        // 正常获取到信号量
        timer.stop();
        PLOG_INFO << "接收到数据可用信号，开始读取ROIs...";
    }
    else {
        // 超时了
        PLOG_WARNING << "等待数据超时，中断操作";
        emit sendErrorOccurred("等待数据超时");
        return rois;
    }

    // 等待future完成
    future.wait();

    if (dataReceived) {
        // 从共享内存读取ROIs
        rois = readROIsFromMemory();
        // 通知发送方数据已读取完成
        PLOG_INFO << "通知发送方数据已读取完成";
        dataReadSemaphore->release();
    }
    else {
        emit sendErrorOccurred("获取信号量失败");
    }

    return rois;
}

std::vector<cv::Mat> ImageReadWorker::readROIsFromMemory()
{
    std::vector<cv::Mat> rois;

    if (!sharedMemory || !sharedMemory->isAttached())
    {
        emit sendErrorOccurred("共享内存未连接");
        return rois;
    }

    try
    {
        // 锁定共享内存以进行安全访问
        if (!sharedMemory->lock())
        {
            emit sendErrorOccurred("无法锁定共享内存");
            return rois;
        }

        // 获取头部信息
        ROIHeader *header = reinterpret_cast<ROIHeader *>(sharedMemory->data());

        // 检查数据是否准备好
        if (!header->dataReady)
        {
            sharedMemory->unlock();
            emit sendErrorOccurred("共享内存中的数据未准备好");
            return rois;
        }

        // 获取ROI信息数组
        ROIInfo *infos = reinterpret_cast<ROIInfo *>(
            reinterpret_cast<char *>(header) + sizeof(ROIHeader));

        // 获取图像数据起始位置
        uchar *imageData = reinterpret_cast<uchar *>(
            reinterpret_cast<char *>(infos) + sizeof(ROIInfo) * header->roiCount);

        // 读取每个ROI
        for (int i = 0; i < header->roiCount; i++)
        {
            int width = infos[i].width;
            int height = infos[i].height;
            int offset = infos[i].offset;
            int channels = infos[i].channels;
            int format = infos[i].format;

            // 根据通道数创建适当的图像矩阵
            cv::Mat image;
            if (channels == 1)
            {
                // 灰度图像
                image = cv::Mat(height, width, CV_8UC1);
                memcpy(image.data, imageData + offset, width * height);
            }
            else if (channels == 3)
            {
                // 彩色图像 - OpenCV默认使用BGR格式
                image = cv::Mat(height, width, CV_8UC3);
                memcpy(image.data, imageData + offset, width * height * 3);

                // 如果格式是RGB，需要转换为BGR
                if (format == 1)
                { // RGB格式
                    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
                }
                // 格式2是BGR，不需要转换
            }
            else if (channels == 4)
            {
                // 带Alpha通道的图像
                image = cv::Mat(height, width, CV_8UC4);
                memcpy(image.data, imageData + offset, width * height * 4);

                // 如果是RGBA格式，转换为BGRA
                if (format == 3)
                { // RGBA格式
                    cv::cvtColor(image, image, cv::COLOR_RGBA2BGRA);
                }
                // 格式4是BGRA，不需要转换
            }

            if (!image.empty())
            {
                rois.push_back(image);
            }
        }

        // 解锁共享内存
        sharedMemory->unlock();
    }
    catch (const std::exception &e)
    {
        if (sharedMemory && sharedMemory->isAttached())
        {
            sharedMemory->unlock();
        }
        emit sendErrorOccurred(QString("共享内存操作异常: ") + e.what());
    }

    return rois;
}

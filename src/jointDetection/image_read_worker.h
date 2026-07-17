#ifndef IMAGE_READ_WORKER_H
#define IMAGE_READ_WORKER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QTimer>
#include <QEventLoop>
#include <QThread>
#include <vector>

#include "../utils/ThreadPool.h"

// 共享内存头部
struct ROIHeader
{
    int version;    // 版本号
    int roiCount;   // ROI数量
    bool dataReady; // 数据是否准备好
};

// ROI信息结构---用于接收共享内存的内容，与发送端的格式相同
struct ROIInfo
{
    int id;       // ROI唯一标识
    int x;        // ROI左上角x坐标
    int y;        // ROI左上角y坐标
    int width;    // ROI宽度
    int height;   // ROI高度
    int offset;   // 图像数据在共享内存中的偏移量
    int channels; // 图像通道数（1:灰度, 3:RGB, 4:RGBA/BGRA）
    int format;   // 图像格式（例如：0:灰度, 1:RGB, 2:BGR, 3:RGBA, 4:BGRA）
};

// 带坐标信息的ROI数据结构---用于存放共享内存的解析结果，方便后续使用
struct ROIWithCoords
{
    int id;
    cv::Mat image;
    int x;        // ROI左上角x坐标
    int y;        // ROI左上角y坐标
};

// @TODO:实现循环等待读图功能，并且在读完图后发送所有ROI给图像处理线程处理
// @TODO:采用文件读取进程ID，而不是手动输入pid
class ImageReadWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImageReadWorker(QObject *parent = nullptr);
    ~ImageReadWorker();

    // 连接到发送方进程
    bool connectToSender(int processId);

public slots:
    void whenReadImage(const QString &path);
    void whenReadImageFromSharedMemory(int processId, int timeoutMs = 30000);

signals:
    void sendImageRead(std::shared_ptr<cv::Mat> image);                         // 单张图像读取完成信号
    void sendErrorOccurred(const QString &error);
    void sendImagesRead(std::shared_ptr<std::vector<ROIWithCoords>> rois);      // 多张图像读取完成信号

private:
    // 清理资源
    void cleanup();
    // 从共享内存读取ROIs
    std::vector<ROIWithCoords> readROIsFromMemory();
    // 等待并读取ROIs
    std::vector<ROIWithCoords> waitAndReadROIs(int timeoutMs = 30000);

    QSharedMemory *sharedMemory;
    QSystemSemaphore *dataAvailableSemaphore; // 数据可用信号量
    QSystemSemaphore *dataReadSemaphore;      // 数据已读信号量
    ThreadPool *m_threadPool;
};

#endif // IMAGE_READ_WORKER_H

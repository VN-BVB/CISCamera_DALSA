#ifndef JSON_SENDER_H
#define JSON_SENDER_H

#include <QString>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QMutex>
#include <QTimer>
#include <QEventLoop>
#include <QThread>
#include <string>

/**
 * @brief JSON共享内存头部结构
 */
struct JsonSharedHeader {
    int batchNumber;    // 批次号（也作为版本号）
    size_t jsonSize;    // JSON数据大小（字节）
    bool dataReady;     // 数据是否准备好
};

/**
 * @brief JSON共享内存发送器
 * 负责将JSON数据通过共享内存发送给其他进程
 */
class JsonSender
{
public:
    JsonSender();
    ~JsonSender();

    bool sendJson(const std::string& jsonString, int batchNumber);
    bool sendJsonWithLogging(const std::string& jsonString, int batchNumber);

    QString getSharedMemoryKey() const;
    QString getDataAvailableSemaphoreKey() const;
    QString getDataReadSemaphoreKey() const;

private:
    /**
     * @brief 初始化同步对象
     */
    void initializeSyncObjects();

    /**
     * @brief 清理资源
     */
    void cleanup();

private:
    QSharedMemory* m_sharedMemory;
    QSystemSemaphore* m_dataAvailableSemaphore;  // 数据可用信号量
    QSystemSemaphore* m_dataReadSemaphore;       // 数据已读信号量
    QMutex m_mutex;                              // 保护内部状态
};

#endif // JSON_SENDER_H
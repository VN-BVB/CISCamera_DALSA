#ifndef ASYNC_JSON_SENDER_H
#define ASYNC_JSON_SENDER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include "batch_send_status.h"
#include "json_sender.h"

/**
 * @brief 异步JSON发送器
 */
class AsyncJsonSender : public QObject
{
    Q_OBJECT

public:
    explicit AsyncJsonSender(int timeoutMs = 600000, QObject *parent = nullptr);
    ~AsyncJsonSender();

    bool sendJsonAsync(const std::string& jsonString, int batchNumber);
    bool canSendNewBatch() const;
    BatchSendStatus getCurrentStatus() const;
    void forceCleanup();
    void setTimeout(int timeoutMs);
    int getTimeout() const;

signals:
    void sendCompleted(int batchNumber, bool success);
    void timeoutOccurred(int batchNumber);
    void earlyBatchArrived(int currentBatchNumber, int newBatchNumber);
    void statusChanged(BatchSendStatus oldStatus, BatchSendStatus newStatus);

private slots:
    void onTimeout();

private:
    void setStatus(BatchSendStatus newStatus);
    void performSend(const std::string& jsonString, int batchNumber);

private:
    JsonSender m_jsonSender;                    // 实际的JSON发送器
    QTimer* m_timeoutTimer;                     // 超时定时器
    QThread* m_workerThread;                    // 工作线程

    std::atomic<BatchSendStatus> m_status{BATCH_IDLE};  // 当前状态
    std::atomic<int> m_currentBatchNumber{-1};  // 当前批次号
    std::atomic<int> m_timeoutMs{600000};       // 超时时间（毫秒）
    std::atomic<bool> m_shouldStop{false};      // 是否应该停止

    QMutex m_pendingDataMutex;                  // 保护待发送数据的互斥锁
    bool m_hasPendingSend{false};               // 是否有待处理的发送
    std::string m_pendingJsonString;            // 待发送的JSON字符串
    int m_pendingBatchNumber{0};                // 待发送的批次号
};

#endif // ASYNC_JSON_SENDER_H
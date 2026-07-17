#include "async_json_sender.h"
#include <plog/Log.h>

/**
 * @brief AsyncJsonSender 构造函数
 * @param timeoutMs 超时时间（毫秒），默认10分钟
 * @param parent 父对象
 * @details 初始化所有成员变量，创建工作线程和定时器
 *          设置初始状态为 IDLE
 */
AsyncJsonSender::AsyncJsonSender(int timeoutMs, QObject *parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
    , m_workerThread(new QThread(this))
    , m_timeoutMs(timeoutMs)
{
    // 初始化状态
    m_status = BATCH_IDLE;
    m_currentBatchNumber = -1;
    m_shouldStop = false;
    m_hasPendingSend = false;

    // 设置定时器
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &AsyncJsonSender::onTimeout);

    // 启动工作线程
    m_workerThread->start();
}

/**
 * @brief 析构函数
 * @details 停止工作线程，清理资源
 */
AsyncJsonSender::~AsyncJsonSender()
{
    m_shouldStop = true;

    // 停止定时器
    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }

    // 停止工作线程
    if (m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait(5000); // 最多等待5秒
    }

    PLOG_INFO << "AsyncJsonSender 已销毁";
}

/**
 * @brief 异步发送JSON数据（非阻塞）
 * @param jsonString JSON字符串数据
 * @param batchNumber 批次号
 * @return 是否可以开始发送（如果当前状态不允许则返回false）
 * @details 检查当前状态，如果可以发送则保存数据并在工作线程中执行
 *          不会阻塞调用线程
 */
bool AsyncJsonSender::sendJsonAsync(const std::string& jsonString, int batchNumber)
{
    if (jsonString.empty()) {
        PLOG_ERROR << "JSON字符串为空，无法发送";
        return false;
    }

    // 检查当前状态
    if (!canSendNewBatch()) {
        BatchSendStatus currentStatus = getCurrentStatus();
        PLOG_WARNING << "当前状态不允许发送新批次，状态: " << static_cast<int>(currentStatus)
                     << "，当前批次: " << m_currentBatchNumber.load()
                     << "，新批次: " << batchNumber;

        // 发送提前到达信号
        emit earlyBatchArrived(m_currentBatchNumber.load(), batchNumber);
        return false;
    }

    // 保存待发送数据
    {
        QMutexLocker locker(&m_pendingDataMutex);
        m_pendingJsonString = jsonString;
        m_pendingBatchNumber = batchNumber;
        m_hasPendingSend = true;
    }

    // 设置状态
    setStatus(BATCH_SENDING);
    m_currentBatchNumber = batchNumber;

    // 在工作线程中执行发送
    QTimer::singleShot(0, [this]() {
        QMutexLocker locker(&m_pendingDataMutex);
        if (m_hasPendingSend) {
            performSend(m_pendingJsonString, m_pendingBatchNumber);
            m_hasPendingSend = false;
        }
    });

    PLOG_INFO << "开始异步发送批次 " << batchNumber;
    return true;
}

/**
 * @brief 检查是否可以发送新批次
 * @return 是否处于可发送状态
 * @details 只有在 IDLE 状态下才能发送新批次
 */
bool AsyncJsonSender::canSendNewBatch() const
{
    return m_status.load() == BATCH_IDLE;
}

/**
 * @brief 获取当前状态
 * @return 当前发送状态
 */
BatchSendStatus AsyncJsonSender::getCurrentStatus() const
{
    return m_status.load();
}

/**
 * @brief 强制清理当前状态（用于超时或错误恢复）
 * @details 重置所有状态，停止定时器，回到空闲状态
 */
void AsyncJsonSender::forceCleanup()
{
    // 停止定时器
    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }

    // 重置状态
    setStatus(BATCH_IDLE);
    m_currentBatchNumber = -1;

    // 清理待发送数据
    {
        QMutexLocker locker(&m_pendingDataMutex);
        m_hasPendingSend = false;
        m_pendingJsonString.clear();
        m_pendingBatchNumber = 0;
    }
}

void AsyncJsonSender::setTimeout(int timeoutMs)
{
    m_timeoutMs = timeoutMs;
}

int AsyncJsonSender::getTimeout() const
{
    return m_timeoutMs.load();
}

/**
 * @brief 设置新状态
 * @param newStatus 新状态
 * @details 原子性地更新状态，并发出状态改变信号
 */
void AsyncJsonSender::setStatus(BatchSendStatus newStatus)
{
    BatchSendStatus oldStatus = m_status.exchange(newStatus);
    if (oldStatus != newStatus) {
        PLOG_DEBUG << "状态改变: " << static_cast<int>(oldStatus) << " -> " << static_cast<int>(newStatus);
        emit statusChanged(oldStatus, newStatus);
    }
}

/**
 * @brief 在后台线程中执行实际发送
 * @param jsonString JSON字符串
 * @param batchNumber 批次号
 * @details 调用 JsonSender 执行实际的共享内存发送操作
 *          发送完成后根据结果设置状态并发出信号
 */
void AsyncJsonSender::performSend(const std::string& jsonString, int batchNumber)
{
    if (m_shouldStop) {
        PLOG_INFO << "收到停止信号，取消发送批次 " << batchNumber;
        setStatus(BATCH_IDLE);
        m_currentBatchNumber = -1;
        return;
    }

    try {
        PLOG_INFO << "开始发送批次 " << batchNumber << " 到共享内存";

        // 执行实际发送
        bool success = m_jsonSender.sendJsonWithLogging(jsonString, batchNumber);

        if (success) {
            PLOG_INFO << "批次 " << batchNumber << " 发送成功，等待接收方读取";
            setStatus(BATCH_WAITING_RECEIVE);

            // 启动超时定时器
            m_timeoutTimer->start(m_timeoutMs.load());
        } else {
            PLOG_ERROR << "批次 " << batchNumber << " 发送失败";
            setStatus(BATCH_ERROR);
            emit sendCompleted(batchNumber, false);

            // 回到空闲状态
            QTimer::singleShot(100, [this]() {
                setStatus(BATCH_IDLE);
                m_currentBatchNumber = -1;
            });
        }

    } catch (const std::exception& e) {
        PLOG_ERROR << "发送批次 " << batchNumber << " 时发生异常: " << e.what();
        setStatus(BATCH_ERROR);
        emit sendCompleted(batchNumber, false);

        // 回到空闲状态
        QTimer::singleShot(100, [this]() {
            setStatus(BATCH_IDLE);
            m_currentBatchNumber = -1;
        });
    }
}

/**
 * @brief 超时处理槽函数
 * @details 当等待接收方超时时调用，清理共享内存状态，发出超时信号
 */
void AsyncJsonSender::onTimeout()
{
    int batchNumber = m_currentBatchNumber.load();

    PLOG_WARNING << "批次 " << batchNumber << " 等待接收方超时";

    setStatus(BATCH_TIMEOUT);
    emit timeoutOccurred(batchNumber);

    // 清理 JsonSender 的状态
    try {
        // 这里可能需要调用 JsonSender 的清理方法
        // 如果 JsonSender 没有提供清理方法，可能需要添加
        PLOG_INFO << "强制清理共享内存状态";
    } catch (const std::exception& e) {
        PLOG_ERROR << "清理共享内存状态时出错: " << e.what();
    }

    // 回到空闲状态
    QTimer::singleShot(100, [this]() {
        setStatus(BATCH_IDLE);
        m_currentBatchNumber = -1;
    });
}

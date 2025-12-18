#include "json_sender.h"
#include <QCoreApplication>
#include <QDebug>
#include <cstring>
#include <plog/Log.h>

JsonSender::JsonSender()
    : m_sharedMemory(nullptr)
    , m_dataAvailableSemaphore(nullptr)
    , m_dataReadSemaphore(nullptr)
{
    // 初始化信号量和共享内存
    initializeSyncObjects();
}

JsonSender::~JsonSender()
{
    cleanup();
}

void JsonSender::initializeSyncObjects()
{
    // 生成固定的进程相关名称
    QString processId = QString::number(QCoreApplication::applicationPid());

    // 创建信号量
    // 数据可用信号量：初始值为0，表示初始状态下数据不可用
    m_dataAvailableSemaphore = new QSystemSemaphore("JSON_Semaphore_" + processId + "_DataAvailable", 0, QSystemSemaphore::Create);
    // 数据已读信号量：初始值为1，表示允许发送方写入数据
    m_dataReadSemaphore = new QSystemSemaphore("JSON_Semaphore_" + processId + "_DataRead", 1, QSystemSemaphore::Create);

    // 创建共享内存，使用固定名称
    m_sharedMemory = new QSharedMemory("JSON_Shared_Memory_" + processId);

    // 尝试分离之前可能存在的共享内存
    if (m_sharedMemory->isAttached())
    {
        m_sharedMemory->detach();
    }
}

void JsonSender::cleanup()
{
    if (m_sharedMemory)
    {
        if (m_sharedMemory->isAttached())
        {
            m_sharedMemory->detach();
        }
        delete m_sharedMemory;
        m_sharedMemory = nullptr;
    }

    if (m_dataAvailableSemaphore)
    {
        delete m_dataAvailableSemaphore;
        m_dataAvailableSemaphore = nullptr;
    }

    if (m_dataReadSemaphore)
    {
        delete m_dataReadSemaphore;
        m_dataReadSemaphore = nullptr;
    }
}

bool JsonSender::sendJson(const std::string& jsonString, int batchNumber)
{
    if (jsonString.empty())
    {
        PLOG_ERROR << "JSON字符串为空，跳过发送";
        return false;
    }

    QMutexLocker locker(&m_mutex);

    // 等待接收方确认数据已读（如果有之前的数据）
    PLOG_INFO << "等待写入权限...";
    if (!m_dataReadSemaphore->acquire())
    {
        PLOG_WARNING << "无法获取写入权限:" << m_dataReadSemaphore->errorString();
        return false;
    }

    try
    {
        // 如果共享内存已附加，先分离它
        if (m_sharedMemory->isAttached())
        {
            m_sharedMemory->detach();
        }

        // 计算所需共享内存大小
        size_t requiredSize = sizeof(JsonSharedHeader) + jsonString.length();

        // 限制最大内存大小，避免系统拒绝
        const size_t MAX_ALLOWED_SIZE = 50 * 1024 * 1024; // 50MB
        if (requiredSize > MAX_ALLOWED_SIZE)
        {
            PLOG_WARNING << "Required memory size" << requiredSize << "exceeds maximum allowed" << MAX_ALLOWED_SIZE;
            m_dataReadSemaphore->release(); // 释放信号量
            return false;
        }

        // 创建共享内存
        if (!m_sharedMemory->create(requiredSize))
        {
            // 如果创建失败，检查是否是因为已存在
            if (m_sharedMemory->error() == QSharedMemory::AlreadyExists)
            {
                // 尝试附加到现有共享内存
                if (!m_sharedMemory->attach())
                {
                    PLOG_ERROR << "Failed to attach to existing shared memory:" << m_sharedMemory->errorString();
                    m_dataReadSemaphore->release(); // 释放信号量
                    return false;
                }

                // 检查现有共享内存大小是否足够
                if (m_sharedMemory->size() < static_cast<int>(requiredSize))
                {
                    PLOG_WARNING << "Existing shared memory size" << m_sharedMemory->size()
                               << "is smaller than required size" << requiredSize;
                    // 分离并尝试重新创建
                    m_sharedMemory->detach();

                    // 尝试创建新共享内存
                    if (!m_sharedMemory->create(requiredSize))
                    {
                        PLOG_ERROR << "Failed to recreate shared memory:" << m_sharedMemory->errorString();
                        m_dataReadSemaphore->release(); // 释放信号量
                        return false;
                    }
                }
            }
            else
            {
                PLOG_ERROR << "Failed to create shared memory:" << m_sharedMemory->errorString();
                m_dataReadSemaphore->release(); // 释放信号量
                return false;
            }
        }

        // 确保能够连接
        if (!m_sharedMemory->isAttached())
        {
            if (!m_sharedMemory->attach())
            {
                PLOG_ERROR << "Failed to attach after creation:" << m_sharedMemory->errorString();
                m_dataReadSemaphore->release(); // 释放信号量
                return false;
            }
        }

        // 设置共享内存中的数据指针
        JsonSharedHeader* header = reinterpret_cast<JsonSharedHeader*>(m_sharedMemory->data());
        char* jsonData = reinterpret_cast<char*>(header) + sizeof(JsonSharedHeader);

        // 设置头部信息
        header->batchNumber = batchNumber;
        header->jsonSize = jsonString.length();
        header->dataReady = false;

        // 复制JSON数据
        memcpy(jsonData, jsonString.c_str(), jsonString.length());

        // 标记数据准备好
        header->dataReady = true;

        PLOG_DEBUG << "Successfully sent JSON batch" << header->batchNumber
                 << "to shared memory (size:" << header->jsonSize << "bytes)";
        PLOG_DEBUG << "Shared memory key:" << m_sharedMemory->key();
        PLOG_DEBUG << "Data Available Semaphore:" << m_dataAvailableSemaphore->key();
        PLOG_DEBUG << "Data Read Semaphore:" << m_dataReadSemaphore->key();

        // 通知接收方数据可用
        PLOG_DEBUG << "通知接收方数据已准备就绪...";
        m_dataAvailableSemaphore->release();

        // 等待接收方读取完成信号
        PLOG_DEBUG << "等待接收方读取数据...";

        // 使用定时器和事件循环实现超时机制
        bool readComplete = false;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        QEventLoop loop;

        // 连接超时信号
        QObject::connect(&timeoutTimer, &QTimer::timeout, [&]() {
            PLOG_WARNING << "等待接收方读取完成超时";
            loop.quit();
        });

        // 启动超时定时器（10分钟超时）
        timeoutTimer.start(600000); // 10分钟

        // 在单独的线程中尝试获取信号量
        QThread workerThread;
        QObject::connect(&workerThread, &QThread::started, [&]() {
            // 尝试获取信号量
            if (m_dataReadSemaphore->acquire()) {
                readComplete = true;
                // 重新释放信号量，保持状态一致性
                m_dataReadSemaphore->release();
            }
            loop.quit();
        });

        workerThread.start();
        loop.exec(); // 等待获取信号量或超时

        // 停止定时器（如果仍在运行）
        if (timeoutTimer.isActive())
        {
            timeoutTimer.stop();
        }

        // 等待工作线程结束
        if (workerThread.isRunning())
        {
            workerThread.quit();
            workerThread.wait();
        }

        if (!readComplete)
        {
            PLOG_WARNING << "接收方未在指定时间内读取数据，超时退出";
            // 超时情况下，确保信号量状态正确
            m_dataReadSemaphore->release();
        }
        else
        {
            PLOG_INFO << "接收到接收方读取完成信号";
            PLOG_INFO << "接收方已成功读取数据";
        }

        return true;
    }
    catch (const std::exception& e)
    {
        PLOG_ERROR << "Exception during shared memory operations:" << e.what();
        m_dataReadSemaphore->release(); // 释放信号量
        return false;
    }
}

QString JsonSender::getSharedMemoryKey() const
{
    return m_sharedMemory ? m_sharedMemory->key() : "No memory created";
}

QString JsonSender::getDataAvailableSemaphoreKey() const
{
    return m_dataAvailableSemaphore ? m_dataAvailableSemaphore->key() : "No semaphore created";
}

QString JsonSender::getDataReadSemaphoreKey() const
{
    return m_dataReadSemaphore ? m_dataReadSemaphore->key() : "No semaphore created";
}

/**
 * @brief 发送JSON数据到共享内存并记录详细日志
 * @param jsonString JSON字符串
 * @param batchNumber 批次号
 * @return 发送是否成功
 * @details
 *   在调用基础 sendJson 方法的基础上，添加 ResultProcessor 原有的日志输出逻辑
 *   包括发送前的信息提示、共享内存和信号量的详细信息输出
 */
bool JsonSender::sendJsonWithLogging(const std::string& jsonString, int batchNumber)
{
    try {
        PLOG_INFO << "发送JSON批次 " << batchNumber << " 到共享内存...";

        if (sendJson(jsonString, batchNumber)) {
            PLOG_INFO << "JSON批次 " << batchNumber << " 成功发送到共享内存";
            PLOG_INFO << "共享内存信息:";
            PLOG_INFO << "- 共享内存名称: " << getSharedMemoryKey().toStdString();
            PLOG_INFO << "- 数据可用信号量: " << getDataAvailableSemaphoreKey().toStdString();
            PLOG_INFO << "- 数据已读信号量: " << getDataReadSemaphoreKey().toStdString();
            return true;
        } else {
            PLOG_ERROR << "JSON批次 " << batchNumber << " 发送到共享内存失败";
            return false;
        }
    } catch (const std::exception& e) {
        PLOG_ERROR << "发送JSON到共享内存时出错: " << e.what();
        return false;
    }
}

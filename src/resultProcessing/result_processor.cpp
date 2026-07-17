#include <plog/Log.h>
#include <QtConcurrent>
#include <QFutureSynchronizer>

#include "result_processor.h"
#include "src/utils/scoped_timer.h"

ResultProcessor::ResultProcessor(QObject *parent)
    : QObject(parent), m_dxfSaver(std::make_shared<DXFSaver>()),
      m_jsonTransformer(nullptr),
      m_asyncJsonSender(std::make_unique<AsyncJsonSender>()),
      m_jsonSaver(std::make_unique<JsonSaver>())
{
    // 连接异步发送器的信号
    connect(m_asyncJsonSender.get(), &AsyncJsonSender::sendCompleted,
            this, &ResultProcessor::onSendCompleted);
    connect(m_asyncJsonSender.get(), &AsyncJsonSender::timeoutOccurred,
            this, &ResultProcessor::onTimeoutOccurred);
    connect(m_asyncJsonSender.get(), &AsyncJsonSender::earlyBatchArrived,
            this, &ResultProcessor::onEarlyBatchArrived);
}

void ResultProcessor::whenEdgeAssemblyFinished(const std::map<int, std::vector<int>>& combinationResult,
                                               const std::map<int, ProcessedROIInfo>& processedRoiInfos)
{
    PLOG_INFO << "ResultProcessor: 接收到EdgeAssembly完成信号";
    PLOG_INFO << "工件数量: " << combinationResult.size();
    PLOG_INFO << "ROI数量: " << processedRoiInfos.size();

    try {
        // 1. 数据准备阶段
        auto workpieceToRoiInfos = WorkpieceRoiMapper::buildWorkpieceRoiMapping(combinationResult, processedRoiInfos);

        // 计算工件外接矩形中心，并建立工件→对位平台映射（最近原则）
        auto workpieceCenters = WorkpiecePlatformMapper::computeWorkpieceCenters(
            combinationResult, processedRoiInfos);

        std::vector<PlatformAxis> platforms;
        loadPlatformAxes("./data/calibration_config/platform_pose.json", platforms);

        auto workpieceToPlatform = WorkpiecePlatformMapper::buildMapping(workpieceCenters, platforms);

        m_jsonTransformer = std::make_unique<JsonTransformer>(processedRoiInfos);
        std::string jsonString = m_jsonTransformer->generateJson(combinationResult, workpieceToPlatform, 0);

        // 2. 并行输出阶段
        QFutureSynchronizer<void> synchronizer;
        // 并行保存DXF文件
        QFuture<void> dxfFuture = QtConcurrent::run([this,
                                                     &workpieceToRoiInfos,
                                                     &processedRoiInfos,
                                                     &workpieceCenters,
                                                     &workpieceToPlatform,
                                                     &platforms]() {
            try {
                SCOPED_TIMER("DXF文件保存");
                m_dxfSaver->whenAllImagesProcessed(workpieceToRoiInfos, processedRoiInfos,
                                                   workpieceCenters, workpieceToPlatform,
                                                   platforms);
                PLOG_INFO << "DXF文件保存完成";
            } catch (const std::exception& e) {
                PLOG_ERROR << "DXF文件保存失败: " << e.what();
            }
        });
        synchronizer.addFuture(dxfFuture);

        // 并行保存JSON文件
        QFuture<void> jsonFileFuture = QtConcurrent::run([this, &jsonString]() {
            try {
                SCOPED_TIMER("JSON文件保存");
                m_jsonSaver->saveJsonToFile(jsonString, 0);
                PLOG_INFO << "JSON文件保存完成";
            } catch (const std::exception& e) {
                PLOG_ERROR << "JSON文件保存失败: " << e.what();
            }
        });
        synchronizer.addFuture(jsonFileFuture);

        // 3、异步发送JSON到共享内存（已经在独立线程中，不需要QtConcurrent）
        m_asyncJsonSender->sendJsonAsync(jsonString, 0);

        // 等待文件保存完成（但不等待共享内存发送）
        synchronizer.waitForFinished();
        PLOG_INFO << "dxf和json文件保存完成，共享内存发送在后台进行";
    } catch (const std::exception& e) {
        PLOG_ERROR << "处理时出错: " << e.what();
    }
}

/**
 * @brief 发送完成槽函数
 * @param batchNumber 批次号
 * @param success 是否成功
 * @details 当异步发送完成时调用，记录发送结果
 */
void ResultProcessor::onSendCompleted(int batchNumber, bool success)
{
    if (success) {
        PLOG_INFO << "批次 " << batchNumber << " 发送完成并被接收方读取";
    } else {
        PLOG_ERROR << "批次 " << batchNumber << " 发送失败";
    }
}

/**
 * @brief 超时发生槽函数
 * @param batchNumber 批次号
 * @details  当等待接收方超时时调用，记录超时信息
 */
void ResultProcessor::onTimeoutOccurred(int batchNumber)
{
    PLOG_WARNING << "批次 " << batchNumber << " 发送超时，接收方未及时读取";
    PLOG_INFO << "共享内存状态已清理，可以发送新批次";
}

/**
 * @brief 新批次提前到达槽函数
 * @param currentBatchNumber 当前正在处理的批次号
 * @param newBatchNumber 新到达的批次号
 * @details  当新批次在上一批次未完成时到达时调用，记录警告信息
 */
void ResultProcessor::onEarlyBatchArrived(int currentBatchNumber, int newBatchNumber)
{
    PLOG_WARNING << "新批次 " << newBatchNumber << " 提前到达，当前批次 "
                 << currentBatchNumber << " 尚未完成";
    PLOG_WARNING << "建议增加批次间隔或检查接收方处理速度";
}



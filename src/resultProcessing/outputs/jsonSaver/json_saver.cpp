#include "json_saver.h"
#include <plog/Log.h>
#include <fstream>
#include <filesystem>

/**
 * @brief JsonSaver 构造函数
 * @param baseDirectory 基础目录路径，JSON文件将保存在此目录下
 * @details 默认使用 "./data/seamEndpointInfos/" 作为保存目录
 */
JsonSaver::JsonSaver(const std::string& baseDirectory)
    : m_baseDirectory(baseDirectory)
{
}

/**
 * @brief 保存JSON数据到文件
 * @param jsonString 要保存的JSON字符串数据
 * @param batchNumber 批次号，用于生成文件名
 * @param fileName 自定义文件名（可选），如果不提供则使用默认命名规则
 * @return 保存是否成功
 */
bool JsonSaver::saveJsonToFile(const std::string& jsonString,
                              int batchNumber,
                              const std::string& fileName)
{
    try {
        // 1. 生成文件名
        std::string finalFileName = fileName.empty() ? generateFileName(batchNumber) : fileName;
        std::string fullPath = m_baseDirectory + "/" + finalFileName;

        // 2. 确保目录存在
        if (!ensureDirectoryExists(m_baseDirectory)) {
            PLOG_ERROR << "无法创建或访问目录: " << m_baseDirectory;
            return false;
        }

        // 3. 打开文件进行写入
        std::ofstream jsonFile(fullPath);
        if (!jsonFile.is_open()) {
            PLOG_ERROR << "无法打开JSON文件进行写入: " << fullPath;
            return false;
        }

        // 4. 写入JSON内容
        jsonFile << jsonString;

        // 5. 确保缓冲区被刷新并关闭文件
        jsonFile.flush();
        jsonFile.close();

        PLOG_INFO << "JSON文件保存成功: " << fullPath;
        return true;

    } catch (const std::exception& e) {
        PLOG_ERROR << "保存JSON文件时出错: " << e.what();
        return false;
    }
}

/**
 * @brief 设置基础目录
 * @param baseDirectory 新的基础目录路径
 * @details 更新JSON文件的保存目标目录
 */
void JsonSaver::setBaseDirectory(const std::string& baseDirectory)
{
    m_baseDirectory = baseDirectory;
}

/**
 * @brief 获取当前基础目录
 * @return 当前设置的基础目录路径
 */
std::string JsonSaver::getBaseDirectory() const
{
    return m_baseDirectory;
}

/**
 * @brief 生成默认文件名
 * @param batchNumber 批次号
 * @return 格式化的文件名，如 "seam_result_batch_0.json"
 * @details 使用固定的命名规则：seam_result_batch_[批次号].json
 */
std::string JsonSaver::generateFileName(int batchNumber) const
{
    return "seam_result_batch_" + std::to_string(batchNumber) + ".json";
}

/**
 * @brief 检查目录是否存在，不存在则创建
 * @param directoryPath 要检查/创建的目录路径
 * @return 目录是否可用（存在或创建成功）
 */
bool JsonSaver::ensureDirectoryExists(const std::string& directoryPath)
{
    try {
        std::filesystem::path dirPath(directoryPath);

        if (std::filesystem::exists(dirPath)) {
            return std::filesystem::is_directory(dirPath);
        }

        std::error_code ec;
        bool created = std::filesystem::create_directories(dirPath, ec);

        if (created) {
            PLOG_INFO << "成功创建目录: " << dirPath.string();
            return true;
        } else {
            PLOG_ERROR << "无法创建目录 " << dirPath.string() << ": " << ec.message();
            return false;
        }

    } catch (const std::exception& e) {
        PLOG_ERROR << "检查目录时出错: " << e.what();
        return false;
    }
}
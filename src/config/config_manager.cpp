#include "config_manager.h"
#include <cereal/archives/json.hpp>
#include <fstream>

const std::string ConfigManager::CONFIG_PATHS_FILE = "./data/config_paths.json";


ConfigManager &ConfigManager::getInstance()
{
    static ConfigManager instance;
    return instance;
}

AppConfig ConfigManager::getConfig() const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_config;
}

bool ConfigManager::loadAllConfigs()
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    bool loadStatus = true;

    // 首先加载各模块配置文件路径
    if (!loadConfig(m_config.paths, CONFIG_PATHS_FILE, "config_paths")) {
        PLOG_WARNING << "[Config] Failed to load config paths, using default paths";
    }

    // 加载图像路径配置
    if (!loadConfig(m_config.image_path_config, m_config.paths.image_path_config_file, "image_path_config")) {
        loadStatus = false;
    }

    // 加载标定配置数据（包含文件路径）
    if (loadConfig(m_config.calibration_config, m_config.paths.calibration_config_file, "calibration_config"))
    {
        PLOG_INFO << "[Config] Calibration config metadata loaded successfully";

        // 通过原有加载方法加载实际的标定数据
        if (!m_config.calibration_config.camera_calibration.load(m_config.calibration_config.camera_calib_file)) {
            loadStatus = false;
        }

        if (!m_config.calibration_config.platform_calibration.load(m_config.calibration_config.platform_calib_file)) {
            loadStatus = false;
        }
    }

    if(loadStatus) {
        PLOG_INFO << "Load all configs done";
    }
    else {
        PLOG_ERROR << "Load configs failed";
    }

    return true;
}

bool ConfigManager::saveAllConfigs()
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    bool success = true;

    // 保存各模块配置文件路径
    if (!saveConfig(m_config.paths, CONFIG_PATHS_FILE, "config_paths"))
    {
        PLOG_ERROR << "[Config] Failed to save config paths";
        success = false;
    }

    // 保存图像路径配置
    if (!saveConfig(m_config.image_path_config, m_config.paths.image_path_config_file, "image_path_config"))
    {
        PLOG_ERROR << "[Config] Failed to save image path config";
        success = false;
    }

    // 保存标定配置元数据（包含文件路径和当前配置状态）
    if (!saveConfig(m_config.calibration_config, m_config.paths.calibration_config_file, "calibration_config"))
    {
        PLOG_ERROR << "[Config] Failed to save calibration config";
        success = false;
    }

    return success;
}

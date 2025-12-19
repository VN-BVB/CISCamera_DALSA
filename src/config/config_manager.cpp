#include "config_manager.h"
#include <cereal/archives/json.hpp>
#include <fstream>

const std::string ConfigManager::IMAGE_PATH_CONFIG_FILE = "./data/imagePathconfig/image_path_config.json";
const std::string ConfigManager::CALIBRATION_CONFIG_FILE = "./data/calibrationconfig/calibration_config.json";

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
    // 加载图像路径配置
    if (!loadConfig(m_config.image_path_config, IMAGE_PATH_CONFIG_FILE, "image_path_config"))   loadStatus = false;

    // 加载标定配置数据（包含文件路径）
    if (loadConfig(m_config.calibration_config, CALIBRATION_CONFIG_FILE, "calibration_config"))
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

    // 保存图像路径配置
    if (!saveConfig(m_config.image_path_config, IMAGE_PATH_CONFIG_FILE, "image_path_config"))
    {
        PLOG_ERROR << "[Config] Failed to save image path config";
        success = false;
    }

    // 保存标定配置元数据（包含文件路径和当前配置状态）
    if (!saveConfig(m_config.calibration_config, CALIBRATION_CONFIG_FILE, "calibration_config"))
    {
        PLOG_ERROR << "[Config] Failed to save calibration config";
        success = false;
    }

    return success;
}

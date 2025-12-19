#include "config_manager.h"
#include <cereal/archives/json.hpp>
#include <fstream>

const std::string ConfigManager::IMAGE_PATH_CONFIG_FILE = "./data/imagePathconfig/image_path_config.json";

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
    if (loadConfig(m_config.image_path_config, IMAGE_PATH_CONFIG_FILE))
    {
        PLOG_INFO << "[Config] Image path config loaded successfully";
    }
    else
    {
        PLOG_WARNING << "[Config] Failed to load image path config, using default values";
    }

    return true;
}

bool ConfigManager::saveAllConfigs()
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    bool success = true;
    if (!saveConfig(m_config.image_path_config, IMAGE_PATH_CONFIG_FILE))
    {
        PLOG_ERROR << "[Config] Failed to save image path config";
        success = false;
    }

    return success;
}

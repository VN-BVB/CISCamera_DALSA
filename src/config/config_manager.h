#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <memory>
#include <shared_mutex>
#include "app_config.h"
#include "plog/Log.h"

class ConfigManager {
public:
    static ConfigManager& getInstance();

    AppConfig getConfig() const;
    bool loadAllConfigs();
    bool saveAllConfigs();

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    template<typename T>
    bool saveConfig(const T& config, const std::string& file_path, const std::string& config_name = "config") {
        try {
            std::ofstream os(file_path);
            cereal::JSONOutputArchive archive(os);
            archive(cereal::make_nvp(config_name, config));
            PLOG_DEBUG << "[Config] Config saved successfully: " << file_path;
            return true;
        } catch (const std::exception& e) {
            PLOG_ERROR << "[Config] Failed to save config: " << e.what();
            return false;
        }
    }

    template<typename T>
    bool loadConfig(T& config, const std::string& file_path, const std::string& config_name = "config") {
        try {
            std::ifstream is(file_path);
            cereal::JSONInputArchive archive(is);
            archive(cereal::make_nvp(config_name, config));
            PLOG_DEBUG << "[Config] Config loaded successfully: " << file_path;
            return true;
        } catch (const std::exception& e) {
            PLOG_ERROR << "[Config] Failed to load config: " << e.what();
            return false;
        }
    }

    mutable std::shared_mutex m_mutex;
    AppConfig m_config;

    static const std::string IMAGE_PATH_CONFIG_FILE;
    static const std::string CALIBRATION_CONFIG_FILE;
};

#endif  // CONFIG_MANAGER_H

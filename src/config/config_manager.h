#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <memory>
#include <shared_mutex>
#include "app_config.h"
#include "plog/Log.h"

/*
 * @details 现在添加一个新模块需要四步：
 *  1、添加对应的module_config.h
 *  2、在ConfigPath添加对应module的配置文件路径，并在config_path.json中手动增加对应module的字段（否则就运行一遍saveConfig）
 *  3、在AppConfig中添加对应module成员，以方便后续使用
 *  4、在ConfigManager中的loadAllConfigs和saveAllConfigs分别添加对应的load和save步骤
 */

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

    static const std::string CONFIG_PATHS_FILE;
};

#endif  // CONFIG_MANAGER_H

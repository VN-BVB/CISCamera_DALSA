#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "image_path_config.h"
#include "calibration_config.h"
#include "config_paths.h"

struct AppConfig {
    // 各模块配置
    ImagePathConfig image_path_config;
    CalibrationConfig calibration_config;

    // 配置文件路径（由ConfigManager单独管理）
    ConfigPaths paths;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(image_path_config),
           CEREAL_NVP(calibration_config));
    }
};

#endif  // APP_CONFIG_H
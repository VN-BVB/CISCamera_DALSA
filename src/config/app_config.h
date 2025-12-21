#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "config_paths.h"
#include "image_path_config.h"
#include "calibration_data_io.h"

struct AppConfig {
    // 配置文件路径（由ConfigManager单独管理）
    ConfigPaths paths;
    // 各模块配置
    ImagePathConfig image_path_config;
    CalibrationData camera_calibration;
    PlatformPoseData platform_calibration;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(image_path_config),
           CEREAL_NVP(camera_calibration),
           CEREAL_NVP(platform_calibration));
    }
};

#endif  // APP_CONFIG_H

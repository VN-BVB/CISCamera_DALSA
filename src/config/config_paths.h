#ifndef CONFIG_PATHS_H
#define CONFIG_PATHS_H

#include <string>

/**
 * @brief 配置文件路径管理
 *
 * 统一管理所有配置文件的路径
 */
struct ConfigPaths {
    // 图像路径配置文件
    std::string image_path_config_file = "./data/imagePathconfig/image_path_config.json";
    // 标定配置文件
    std::string calibration_config_file = "./data/calibrationconfig/calibration_config.json";
    // 数据根目录
    std::string data_root_dir = "./data";

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(image_path_config_file),
           CEREAL_NVP(calibration_config_file),
           CEREAL_NVP(data_root_dir));
    }
};

#endif  // CONFIG_PATHS_H

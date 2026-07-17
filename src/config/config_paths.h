#ifndef CONFIG_PATHS_H
#define CONFIG_PATHS_H

#include <string>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>

/**
 * @brief 配置文件路径管理
 *
 * 统一管理所有配置文件的路径
 */
struct ConfigPaths {
    std::string data_root_dir = "./data";                                                           // 数据根目录
    std::string image_path_config_file = "./data/config/image_path_config.json";                    // 图像路径配置文件
    std::string camera_calibration_file = "./data/calibration_config/optimized_calib_data.json";    // 相机标定配置文件
    std::string platform_calibration_file = "./data/calibration_config/platform_pose.json";         // 平台标定配置文件

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(data_root_dir),
           CEREAL_NVP(image_path_config_file),
           CEREAL_NVP(camera_calibration_file),
           CEREAL_NVP(platform_calibration_file));
    }
};

#endif  // CONFIG_PATHS_H

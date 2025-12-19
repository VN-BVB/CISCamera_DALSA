#ifndef CALIBRATION_CONFIG_H
#define CALIBRATION_CONFIG_H

#include <string>
#include "src/config/calibration_data_io.h"

/**
 * @brief 标定配置适配器
 *
 * 将相机标定和平台标定数据适配到统一的配置管理框架中。
 * 保持原有CalibrationData和PlatformPoseData类不变，通过适配器模式集成。
 */
struct CalibrationConfig {
    CalibrationData camera_calibration;
    PlatformPoseData platform_calibration;

    // 配置文件路径
    std::string camera_calib_file = "./data/calibration_config/optimized_calib_data.json";
    std::string platform_calib_file = "./data/calibration_config/platform_pose.json";

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(camera_calibration),
           CEREAL_NVP(platform_calibration),
           CEREAL_NVP(camera_calib_file),
           CEREAL_NVP(platform_calib_file));
    }
};

#endif  // CALIBRATION_CONFIG_H

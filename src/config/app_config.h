#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "image_path_config.h"
#include "calibration_config.h"

struct AppConfig {
    ImagePathConfig image_path_config;
    CalibrationConfig calibration_config;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(image_path_config),
           CEREAL_NVP(calibration_config));
    }
};

#endif  // APP_CONFIG_H
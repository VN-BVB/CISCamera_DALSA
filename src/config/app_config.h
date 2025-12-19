#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "image_path_config.h"

struct AppConfig {
    ImagePathConfig image_path_config;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(image_path_config));
    }
};

#endif  // APP_CONFIG_H
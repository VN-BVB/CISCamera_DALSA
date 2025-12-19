#ifndef IMAGE_PATH_CONFIG_H
#define IMAGE_PATH_CONFIG_H

#include <string>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>

struct ImagePathConfig {
    std::string default_image_folder = "E:/work/车门门环拼接/image/背面打光/9/1";
    std::string last_opened_folder = "";
    std::string shared_memory_images_output = "E:/work/车门门环拼接/image/背面打光/5/1/test/";

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(default_image_folder),
           CEREAL_NVP(last_opened_folder),
           CEREAL_NVP(shared_memory_images_output));
    }
};

#endif  // IMAGE_PATH_CONFIG_H
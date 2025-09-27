#include "abstract_camera_factory.h"

std::shared_ptr<AbstractCamera> AbstractCameraFactory::createCamera(CameraType type) {
    switch (type) {
        case CameraType::DALSA:
            return std::make_shared<DalsaCamera>(nullptr);
        // case CameraType::OTHER:
        //     return std::make_shared<OtherCamera>(parent);
        default:
            return nullptr;
    }
}

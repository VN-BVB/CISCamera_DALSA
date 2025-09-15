#include "AbstractCameraFactory.h"

std::shared_ptr<AbstractCamera> AbstractCameraFactory::createCamera(CameraType type, QObject* parent) {
    switch (type) {
        case CameraType::DALSA:
            return std::make_shared<DalsaCamera>(parent);
        // case CameraType::OTHER:
        //     return std::make_shared<OtherCamera>(parent);
        default:
            return nullptr;
    }
}

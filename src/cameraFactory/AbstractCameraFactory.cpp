#include "AbstractCameraFactory.h"

AbstractCameraFactory::AbstractCameraFactory(QObject *parent) { (void)parent; }

AbstractCameraFactory::~AbstractCameraFactory() {}

std::shared_ptr<AbstractCamera> AbstractCameraFactory::createCamera(CameraType type) {
    switch (type) {
        case CameraType::DALSA:
            return std::make_shared<DalsaCamera>();
        default:
            return nullptr;
    }
}

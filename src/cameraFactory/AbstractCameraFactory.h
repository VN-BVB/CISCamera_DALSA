#ifndef ABSTRACTCAMERAFACTORY_H
#define ABSTRACTCAMERAFACTORY_H

#include <memory>

#include "AbstractCamera.h"
#include "dalsaCameraLink/DalsaCamera.h"

enum class CameraType {
    DALSA,
    // 以后可增加其他相机类型
};

class AbstractCameraFactory {
public:
    AbstractCameraFactory(QObject* parent = nullptr);
    ~AbstractCameraFactory();

    static std::shared_ptr<AbstractCamera> createCamera(CameraType type);
};

#endif  // ABSTRACTCAMERAFACTORY_H

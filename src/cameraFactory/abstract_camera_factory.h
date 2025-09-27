#ifndef ABSTRACTCAMERAFACTORY_H
#define ABSTRACTCAMERAFACTORY_H

#include <memory>

#include "AbstractCamera.h"
#include "dalsaCameraLink/DalsaCamera.h"

enum class CameraType {
    DALSA,
    // 可扩展其他相机类型
};

class AbstractCameraFactory {
public:
    AbstractCameraFactory() = delete;  // 工厂不需要实例化
    ~AbstractCameraFactory() = delete;

    // 静态方法创建相机实例
    static std::shared_ptr<AbstractCamera> createCamera(CameraType type);
};

#endif  // ABSTRACTCAMERAFACTORY_H

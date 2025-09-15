#include "AbstractCamera.h"

AbstractCamera::AbstractCamera(QObject* parent) : QObject(parent) {
    // 可以做一些公共初始化，比如日志、计数器等
}

AbstractCamera::~AbstractCamera() {
    // 可以做一些公共清理
}

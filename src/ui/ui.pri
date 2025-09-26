# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/CISCameraImage.cpp \
    $$PWD/ImageViewWindow.cpp \
    $$PWD/imageWidget/interactiveImageItem.cpp \
    $$PWD/imageWidget/interactiveScene.cpp \
    $$PWD/imageWidget/interactiveView.cpp \
    $$PWD/imageWidget/openGLImageWidget.cpp


HEADERS += \
    $$PWD/CISCameraImage.h \
    $$PWD/ImageViewWindow.h \
    $$PWD/imageWidget/interactiveGlobal.h \
    $$PWD/imageWidget/interactiveImageItem.h \
    $$PWD/imageWidget/interactiveScene.h \
    $$PWD/imageWidget/interactiveView.h \
    $$PWD/imageWidget/openGLImageWidget.h

FORMS += \
    $$PWD/CISCameraImage.ui \
    $$PWD/ImageViewWindow.ui
    $$PWD/imageWidget/openGLImageWidget.cpp





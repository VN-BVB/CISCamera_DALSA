# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/cis_camera_image.cpp \
    $$PWD/ImageViewWindow.cpp \
    $$PWD/imageWidget/baseWidget.cpp \
    $$PWD/imageWidget/frmVisionDisplay.cpp \
    $$PWD/imageWidget/interactiveDisplayManager.cpp \
    $$PWD/imageWidget/interactiveImageItem.cpp \
    $$PWD/imageWidget/interactiveScene.cpp \
    $$PWD/imageWidget/interactiveView.cpp \
    $$PWD/utils/imageWidget/openGLImageWidget.cpp



HEADERS += \
    $$PWD/cis_camera_image.h \
    $$PWD/ImageViewWindow.h \
    $$PWD/imageWidget/baseWidget.h \
    $$PWD/imageWidget/frmVisionDisplay.h \
    $$PWD/imageWidget/interactiveDisplayManager.h \
    $$PWD/imageWidget/interactiveGlobal.h \
    $$PWD/imageWidget/interactiveImageItem.h \
    $$PWD/imageWidget/interactiveScene.h \
    $$PWD/imageWidget/interactiveView.h \
    $$PWD/utils/stateLight/StateLight.h \
    $$PWD/utils/imageWidget/openGLImageWidget.h

FORMS += \
    $$PWD/cis_camera_image.ui \
    $$PWD/ImageViewWindow.ui
    $$PWD/imageWidget/openGLImageWidget.cpp





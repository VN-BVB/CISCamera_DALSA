# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/cis_camera_image.cpp \
    $$PWD/ImageViewWindow.cpp \
    $$PWD/test_frmVisionDisplay.cpp \
    $$PWD/utils/imageWidget/baseWidget.cpp \
    $$PWD/utils/imageWidget/frmVisionDisplay.cpp \
    $$PWD/utils/imageWidget/interactiveDisplayManager.cpp \
    $$PWD/utils/imageWidget/interactiveImageItem.cpp \
    $$PWD/utils/imageWidget/interactiveScene.cpp \
    $$PWD/utils/imageWidget/interactiveView.cpp \
    $$PWD/utils/imageWidget/openGLImageWidget.cpp



HEADERS += \
    $$PWD/cis_camera_image.h \
    $$PWD/ImageViewWindow.h \
    $$PWD/test_frmVisionDisplay.h \
    $$PWD/utils/imageWidget/baseWidget.h \
    $$PWD/utils/imageWidget/frmVisionDisplay.h \
    $$PWD/utils/imageWidget/interactiveDisplayManager.h \
    $$PWD/utils/imageWidget/interactiveGlobal.h \
    $$PWD/utils/imageWidget/interactiveImageItem.h \
    $$PWD/utils/imageWidget/interactiveScene.h \
    $$PWD/utils/imageWidget/interactiveView.h \
    $$PWD/utils/stateLight/StateLight.h \
    $$PWD/utils/imageWidget/openGLImageWidget.h

FORMS += \
    $$PWD/cis_camera_image.ui \
    $$PWD/ImageViewWindow.ui \
    $$PWD/test_frmVisionDisplay.ui
    $$PWD/utils/imageWidget/openGLImageWidget.cpp





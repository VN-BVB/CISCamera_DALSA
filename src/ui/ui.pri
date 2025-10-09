# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/CISCameraImage.cpp \
    $$PWD/ImageViewWindow.cpp \
    $$PWD/imageWidget/baseWidget.cpp \
    $$PWD/imageWidget/frmVisionDisplay.cpp \
    $$PWD/imageWidget/interactiveDisplayManager.cpp \
    $$PWD/imageWidget/interactiveImageItem.cpp \
    $$PWD/imageWidget/interactiveScene.cpp \
    $$PWD/imageWidget/interactiveView.cpp \
    $$PWD/imageWidget/openGLImageWidget.cpp \
    $$PWD/test_frmVisionDisplay.cpp


HEADERS += \
    $$PWD/CISCameraImage.h \
    $$PWD/ImageViewWindow.h \
    $$PWD/imageWidget/baseWidget.h \
    $$PWD/imageWidget/frmVisionDisplay.h \
    $$PWD/imageWidget/interactiveDisplayManager.h \
    $$PWD/imageWidget/interactiveGlobal.h \
    $$PWD/imageWidget/interactiveImageItem.h \
    $$PWD/imageWidget/interactiveScene.h \
    $$PWD/imageWidget/interactiveView.h \
    $$PWD/imageWidget/openGLImageWidget.h \
    $$PWD/test_frmVisionDisplay.h

FORMS += \
    $$PWD/CISCameraImage.ui \
    $$PWD/ImageViewWindow.ui \
    $$PWD/test_frmVisionDisplay.ui
    $$PWD/imageWidget/openGLImageWidget.cpp





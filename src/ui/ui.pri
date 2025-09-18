# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/CISCameraImage.cpp \
    $$PWD/imageviewwindow.cpp \
    $$PWD/interactiveview.cpp  \
    $$PWD/imageWidget/openGLImageWidget.cpp
HEADERS += \
    $$PWD/CISCameraImage.h \
    $$PWD/imageviewwindow.h \
    $$PWD/interactiveview.h  \
    $$PWD/imageWidget/openGLImageWidget.h

FORMS += \
    $$PWD/CISCameraImage.ui \
    $$PWD/imageviewwindow.ui
    $$PWD/imageWidget/openGLImageWidget.cpp





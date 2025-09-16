# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/CISCameraImage.cpp \
    $$PWD/imageWidget/openGLImageWidget.cpp
HEADERS += \
    $$PWD/CISCameraImage.h \
    $$PWD/imageWidget/openGLImageWidget.h

FORMS += \
    $$PWD/CISCameraImage.ui


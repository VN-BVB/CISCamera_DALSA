# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/cis_camera_image.cpp \
    $$PWD/utils/imageWidget/openGLImageWidget.cpp

HEADERS += \
    $$PWD/cis_camera_image.h \
    $$PWD/utils/stateLight/StateLight.h \
    $$PWD/utils/imageWidget/openGLImageWidget.h

FORMS += \
    $$PWD/cis_camera_image.ui


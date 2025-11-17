# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/CISCamera_imageGrab/cameraImage_processor.cpp \
    $$PWD/CISCamera_imageGrab/cis_camera_image.cpp \
    $$PWD/jointView/joint_view.cpp



HEADERS += \
    $$PWD/CISCamera_imageGrab/cameraImage_processor.h \
    $$PWD/CISCamera_imageGrab/cis_camera_image.h \
    $$PWD/jointView/joint_view.h

FORMS += \
    $$PWD/CISCamera_imageGrab/cis_camera_image.ui \
    $$PWD/jointView/joint_view.ui

include(./utils/utils.pri)

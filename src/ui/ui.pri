# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/CISCamera_imageGrab/cameraImage_processor.cpp \
    $$PWD/CISCamera_imageGrab/cis_camera_image.cpp \
    $$PWD/jointView/joint_view.cpp \
    $$PWD/utils/display/base_widget.cpp \
    $$PWD/utils/display/display_image_item.cpp \
    $$PWD/utils/display/display_manager.cpp \
    $$PWD/utils/display/display_scene.cpp \
    $$PWD/utils/display/display_view.cpp \
    $$PWD/utils/display/frm_display.cpp \
    $$PWD/utils/display/openGLImageWidget.cpp



HEADERS += \
    $$PWD/CISCamera_imageGrab/cameraImage_processor.h \
    $$PWD/CISCamera_imageGrab/cis_camera_image.h \
    $$PWD/jointView/joint_view.h \
    $$PWD/utils/display/base_widget.h \
    $$PWD/utils/display/display_global.h \
    $$PWD/utils/display/display_image_item.h \
    $$PWD/utils/display/display_manager.h \
    $$PWD/utils/display/display_scene.h \
    $$PWD/utils/display/display_view.h \
    $$PWD/utils/display/frm_display.h \
    $$PWD/utils/display/openGLImageWidget.h \
    $$PWD/utils/stateLight/StateLight.h

FORMS += \
    $$PWD/CISCamera_imageGrab/cis_camera_image.ui \
    $$PWD/jointView/joint_view.ui



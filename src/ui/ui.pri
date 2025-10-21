# Use Precompiled headers (PCH)
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/ui/stable.h

SOURCES += \
    $$PWD/CISCamera_imageGrab/cameraImage_processor.cpp \
    $$PWD/CISCamera_imageGrab/cis_camera_image.cpp \
    $$PWD/jointView/joint_view.cpp \
    $$PWD/utils/imageWidget/base_widget.cpp \
    $$PWD/utils/imageWidget/frm_vision_display.cpp \
    $$PWD/utils/imageWidget/interactive_display_manager.cpp \
    $$PWD/utils/imageWidget/interactive_image_item.cpp \
    $$PWD/utils/imageWidget/interactive_scene.cpp \
    $$PWD/utils/imageWidget/interactive_view.cpp \
    $$PWD/utils/imageWidget/openGLImageWidget.cpp



HEADERS += \
    $$PWD/CISCamera_imageGrab/cameraImage_processor.h \
    $$PWD/CISCamera_imageGrab/cis_camera_image.h \
    $$PWD/jointView/joint_view.h \
    $$PWD/utils/imageWidget/base_widget.h \
    $$PWD/utils/imageWidget/frm_vision_display.h \
    $$PWD/utils/imageWidget/interactive_display_manager.h \
    $$PWD/utils/imageWidget/interactive_global.h \
    $$PWD/utils/imageWidget/interactive_image_item.h \
    $$PWD/utils/imageWidget/interactive_scene.h \
    $$PWD/utils/imageWidget/interactive_view.h \
    $$PWD/utils/imageWidget/openGLImageWidget.h \
    $$PWD/utils/stateLight/StateLight.h

FORMS += \
    $$PWD/CISCamera_imageGrab/cis_camera_image.ui \
    $$PWD/jointView/joint_view.ui



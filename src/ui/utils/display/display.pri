HEADERS += \
    $$PWD/base_widget.h \
    $$PWD/display_global.h \
    $$PWD/display_image_item.h \
    $$PWD/display_manager.h \
    $$PWD/display_scene.h \
    $$PWD/display_view.h \
    $$PWD/frm_display.h \
    $$PWD/openGLImageWidget.h

SOURCES += \
    $$PWD/base_widget.cpp \
    $$PWD/display_image_item.cpp \
    $$PWD/display_manager.cpp \
    $$PWD/display_scene.cpp \
    $$PWD/display_view.cpp \
    $$PWD/frm_display.cpp \
    $$PWD/openGLImageWidget.cpp

include(./graphicItems/graphicItems.pri)

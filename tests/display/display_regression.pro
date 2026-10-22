QT += widgets testlib
CONFIG += console testcase c++17 release
CONFIG -= app_bundle debug
TARGET = display_regression
msvc: QMAKE_CXXFLAGS += /utf-8
PROJECT_ROOT = $$clean_path($$PWD/../..)
DISPLAY_ROOT = $$PROJECT_ROOT/src/ui/utils/display
INCLUDEPATH += $$PROJECT_ROOT $$DISPLAY_ROOT \
    $$PROJECT_ROOT/3rdParty/plog/include \
    $$PROJECT_ROOT/3rdParty/tinyspline/release/include \
    D:/ProgramData/opencv/build/include
LIBS += -LD:/ProgramData/opencv/build/x64/vc15/lib -lopencv_world440
SOURCES += $$PWD/display_regression.cpp \
    $$DISPLAY_ROOT/base_widget.cpp \
    $$DISPLAY_ROOT/frm_display.cpp \
    $$DISPLAY_ROOT/display_manager.cpp \
    $$DISPLAY_ROOT/display_view.cpp \
    $$DISPLAY_ROOT/display_scene.cpp \
    $$DISPLAY_ROOT/display_image_item.cpp \
    $$DISPLAY_ROOT/display_text_item.cpp \
    $$DISPLAY_ROOT/graphicItems/graphic_item_component.cpp \
    $$DISPLAY_ROOT/graphicItems/graphic_item_composite.cpp
HEADERS += $$DISPLAY_ROOT/base_widget.h \
    $$DISPLAY_ROOT/frm_display.h \
    $$DISPLAY_ROOT/display_manager.h \
    $$DISPLAY_ROOT/display_view.h \
    $$DISPLAY_ROOT/display_scene.h \
    $$DISPLAY_ROOT/display_image_item.h \
    $$DISPLAY_ROOT/display_text_item.h

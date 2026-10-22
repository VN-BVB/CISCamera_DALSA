QT = core gui widgets
CONFIG += console c++17
CONFIG -= app_bundle
TARGET = motion_table_move_check
DEFINES += DLLBUILD
msvc: QMAKE_CXXFLAGS += /utf-8

INCLUDEPATH += $$PWD/../../motion \
               $$PWD/../../../3rdParty/libmodbus/include \
               $$PWD/../../../3rdParty/cereal/include
SOURCES += $$PWD/check.cpp \
           $$PWD/../../motion/motion_widget.cpp \
           $$PWD/../../motion/motion_controller.cpp \
           $$PWD/../../motion/plccommunication.cpp
HEADERS += $$PWD/../../motion/motion_widget.h \
           $$PWD/../../motion/motion_controller.h
FORMS += $$PWD/../../motion/motion_widget.ui

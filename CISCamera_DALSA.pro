QT       += core gui widgets network charts printsupport concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += release debug_info

qmake

# All project sources are UTF-8.  Keep the compiler's source and execution
# character sets independent from the Windows system ANSI code page.
msvc {
    QMAKE_CXXFLAGS += /MP /bigobj /utf-8
}

mingw {
    QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
}
QMAKE_CXXFLAGS_RELEASE = -ZI -MD
QMAKE_LFLAGS_RELEASE = /DEBUG
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp
# HEADERS += \
# FORMS += \



include(./src/src.pri)
include(./3rdParty/3rdParty.pri)
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

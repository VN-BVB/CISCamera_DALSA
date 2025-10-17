HEADERS += \
    $$PWD/ImageProcessWorker.h \
    $$PWD/ImageReadWorker.h \
    $$PWD/imageTools.h

SOURCES += \
    $$PWD/ImageProcessWorker.cpp \
    $$PWD/ImageReadWorker.cpp \
    $$PWD/imageTools.cpp

include(./edgeDetection/edgeDetection.pri)
include(./contourSegment/contourSegment.pri)


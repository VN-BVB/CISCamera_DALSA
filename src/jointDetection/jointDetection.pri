HEADERS += \
    $$PWD/ImageProcessWorker.h \
    $$PWD/ImageReadWorker.h

SOURCES += \
    $$PWD/ImageProcessWorker.cpp \
    $$PWD/ImageReadWorker.cpp

include( ./edgeDetection/edgeDetection.pri )


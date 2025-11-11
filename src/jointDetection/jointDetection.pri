HEADERS += \
    $$PWD/image_process_worker.h \
    $$PWD/image_read_worker.h \
    $$PWD/image_tools.h \
    $$PWD/joint_seam.h

SOURCES += \
    $$PWD/image_process_worker.cpp \
    $$PWD/image_read_worker.cpp \
    $$PWD/image_tools.cpp \
    $$PWD/joint_seam.cpp \

include(./edgeDetection/edgeDetection.pri)
include(./contourProcess/contourProcess.pri)


HEADERS += \
    $$PWD/telecentric_line_calibrator.h \
    $$PWD/telecentric_lm_optimizer.h \
    $$PWD/py_telecentric_optimizer.h


SOURCES += \
    $$PWD/telecentric_line_calibrator.cpp \
    $$PWD/telecentric_lm_optimizer.cpp
include( ./libcbdetect/libcbdetect.pri)

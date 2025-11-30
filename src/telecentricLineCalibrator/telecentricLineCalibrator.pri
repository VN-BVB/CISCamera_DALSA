HEADERS += \
    $$PWD/calib_utils.h \
    $$PWD/telecentric_line_calibrator.h \
    $$PWD/telecentric_lm_optimizer.h \
    $$PWD/py_telecentric_optimizer.h \
    $$PWD/telecentricplatform_calib.h


SOURCES += \
    $$PWD/py_telecentric_optimizer.cpp \
    $$PWD/telecentric_line_calibrator.cpp \
    $$PWD/telecentric_lm_optimizer.cpp \
    $$PWD/telecentricplatform_calib.cpp
include( ./libcbdetect/libcbdetect.pri)

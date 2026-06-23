HEADERS += \
    $$PWD/contour_processing_pipeline.h \
    $$PWD/contour_processing_strategy.h \
    $$PWD/contour_processor.h


SOURCES += \
    $$PWD/contour_processing_pipeline.cpp \
    $$PWD/contour_processing_strategy.cpp \
    $$PWD/contour_processor.cpp

include(./methods/methods.pri)
include(./strategies/strategies.pri)

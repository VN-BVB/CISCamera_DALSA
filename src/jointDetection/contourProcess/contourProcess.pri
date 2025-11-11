HEADERS += \
    $$PWD/contour_processing_pipeline.h \
    $$PWD/contour_processing_strategy.h \
    $$PWD/contour_processor.h \
    $$PWD/corner_detection_strategy.h \
    $$PWD/curve_fitting_strategy.h \
    $$PWD/deduplication_strategy.h \
    $$PWD/direction_calculation_strategy.h \
    $$PWD/segmentation_strategy.h \
    $$PWD/sorting_strategy.h


SOURCES += \
    $$PWD/contour_processing_pipeline.cpp \
    $$PWD/contour_processing_strategy.cpp \
    $$PWD/contour_processor.cpp \
    $$PWD/corner_detection_strategy.cpp \
    $$PWD/curve_fitting_strategy.cpp \
    $$PWD/deduplication_strategy.cpp \
    $$PWD/direction_calculation_strategy.cpp \
    $$PWD/segmentation_strategy.cpp \
    $$PWD/sorting_strategy.cpp

include(./methods/methods.pri)

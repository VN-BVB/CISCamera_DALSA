SOURCES += \
    $$PWD/lib_cb_detecor.cpp \
    $$PWD/libmain.cpp \
    $$PWD/src/example.cc \
    $$PWD/src/libcbdetect/board_energy.cc \
    $$PWD/src/libcbdetect/boards_from_corners.cc \
    $$PWD/src/libcbdetect/create_correlation_patch.cc \
    $$PWD/src/libcbdetect/filter_board.cc \
    $$PWD/src/libcbdetect/filter_corners.cc \
    $$PWD/src/libcbdetect/find_corners.cc \
    $$PWD/src/libcbdetect/find_modes_meanshift.cc \
    $$PWD/src/libcbdetect/get_image_patch.cc \
    $$PWD/src/libcbdetect/get_init_location.cc \
    $$PWD/src/libcbdetect/grow_board.cc \
    $$PWD/src/libcbdetect/image_normalization_and_gradients.cc \
    $$PWD/src/libcbdetect/init_board.cc \
    $$PWD/src/libcbdetect/non_maximum_suppression.cc \
    $$PWD/src/libcbdetect/plot_boards.cc \
    $$PWD/src/libcbdetect/plot_corners.cc \
    $$PWD/src/libcbdetect/polynomial_fit.cc \
    $$PWD/src/libcbdetect/refine_corners.cc \
    $$PWD/src/libcbdetect/score_corners.cc \
    $$PWD/src/libcbdetect/weight_mask.cc

HEADERS += \
    $$PWD/lib_cb_detecor.h \
    $$PWD/libcbdetect/board_energy.h \
     $$PWD/libcbdetect/boards_from_corners.h \
     $$PWD/libcbdetect/config.h \
     $$PWD/libcbdetect/create_correlation_patch.h \
     $$PWD/libcbdetect/filter_board.h \
     $$PWD/libcbdetect/filter_corners.h \
     $$PWD/libcbdetect/find_corners.h \
     $$PWD/libcbdetect/find_modes_meanshift.h \
     $$PWD/libcbdetect/get_image_patch.h \
     $$PWD/libcbdetect/get_init_location.h \
     $$PWD/libcbdetect/grow_board.h \
     $$PWD/libcbdetect/image_normalization_and_gradients.h \
     $$PWD/libcbdetect/init_board.h \
     $$PWD/libcbdetect/non_maximum_suppression.h \
     $$PWD/libcbdetect/plot_boards.h \
     $$PWD/libcbdetect/plot_corners.h \
     $$PWD/libcbdetect/polynomial_fit.h \
     $$PWD/libcbdetect/refine_corners.h \
     $$PWD/libcbdetect/score_corners.h \
     $$PWD/libcbdetect/weight_mask.h
FORMS += \


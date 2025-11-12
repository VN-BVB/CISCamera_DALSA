# ------------------DALSA库-------------------
# 头文件路径
INCLUDEPATH += "C:/Program Files/Teledyne DALSA/Sapera/Include"
DEPENDPATH  += "C:/Program Files/Teledyne DALSA/Sapera/Include"

INCLUDEPATH += "C:/Program Files/Teledyne DALSA/Sapera/Classes"
DEPENDPATH  += "C:/Program Files/Teledyne DALSA/Sapera/Classes"

INCLUDEPATH += "C:/Program Files/Teledyne DALSA/Sapera/Classes/Basic"
DEPENDPATH  += "C:/Program Files/Teledyne DALSA/Sapera/Classes/Basic"

# 库文件路径
LIBS += -L"C:/Program Files/Teledyne DALSA/Sapera/Lib/Win64" \
        -lSapClassBasic \
        -lcorapi
# ------------------OpenCV库-------------------
INCLUDEPATH += D:/ProgramData/opencv/build/include/
INCLUDEPATH += D:/ProgramData/opencv/build/include/opencv2/
LIBS += -LD:/ProgramData/opencv/build/x64/vc15/lib/ -lopencv_world440

# ------------------PLOG库---------------------
INCLUDEPATH += ./3rdParty/plog/include
# ------------------LIBMODBUS库------------------
INCLUDEPATH +=./3rdparty/libmodbus/include
LIBS +=./3rdparty/libmodbus/X64/*.lib
# ------------------Eigen矩阵运算库------------------
INCLUDEPATH += D:/ProgramData/eigen-git-mirror-master
# ------------------cereal序列化反序列化库------------------
INCLUDEPATH += ./3rdparty/cereal/include
# ------------------Pybind11C++与Python双向绑定库------------------
# ---------- Python 环境 ----------
PYTHON_VER = 39
PYTHON_ROOT = D:/anaconda/envs/Telecentric-Calibration
INCLUDEPATH += $$PYTHON_ROOT/include
LIBS += -L$$PYTHON_ROOT/libs -lpython$$PYTHON_VER
INCLUDEPATH += D:/ProgramData/extern/pybind11/include
# ------------------tinyspline库------------------
CONFIG(debug, debug|release) {
    INCLUDEPATH += $$PWD/tinyspline/debug/include
    LIBS += -L$$PWD/tinyspline/debug/lib64/ -ltinysplinecxx
} else:CONFIG(release, debug|release) {
    INCLUDEPATH += $$PWD/tinyspline/release/include
    LIBS += -L$$PWD/tinyspline/release/lib64/ -ltinysplinecxx
}

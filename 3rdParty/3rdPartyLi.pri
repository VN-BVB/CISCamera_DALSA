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

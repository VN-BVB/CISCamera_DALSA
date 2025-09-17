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
INCLUDEPATH += D:/apps/Opencv/opencv/build/include/
INCLUDEPATH += D:/apps/Opencv/opencv/build/include/opencv2/
LIBS += -LD:/apps/Opencv/opencv/build/x64/vc16/lib/ -lopencv_world4100


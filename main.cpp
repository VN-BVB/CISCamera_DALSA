#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>

#include <QApplication>

<<<<<<< HEAD
#include "src/crashHandler/CrashHandler.h"
#include "src/ui/cis_camera_image.h"
=======
#include "src/ui/CISCameraImage.h"
#include "src/ui/ImageViewWindow.h"
#include "src/ui/test_frmVisionDisplay.h"
>>>>>>> origin/jointDetection

void initPlog();  // 初始化日志类
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
<<<<<<< HEAD
    CrashHandler::Init(L"data/debug");  // 初始化Mini转储
    initPlog();                         // 初始化日志类
    CISWidget w;
=======
    initPlog();
    // ImageViewWindow w;
    // w.show();

    test_FrmVisionDisplay w;
>>>>>>> origin/jointDetection
    w.show();
    // QImage image("C:/Users/Zhang/Pictures/头像.png");

    QImage image("C:/Users/Zhang/Pictures/pix.png");
    w.displayImage(image);
    return a.exec();
}
// 初始化日志类
void initPlog() {
    // 日志信息分类等级: none = 0, fatal = 1, error = 2, warning = 3, info = 4, debug = 5, verbose = 6
    // 设置初始化等级后, 等级『数值大于』设置值的日志信息就会被『忽略』
    plog::init(plog::debug, "./data/log/log.csv", 1000000000, 100);
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::get()->addAppender(&consoleAppender);  // Also add logging to the console.
}

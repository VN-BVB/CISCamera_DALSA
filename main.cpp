#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>

#include <QApplication>
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include "src/crashHandler/CrashHandler.h"
#include "src/ui/CISCamera_imageGrab/cis_camera_image.h"
#include "src/ui/jointView/joint_view.h"
#include "src/test/test_frmVisionDisplay.h"

#include "src/test/test_tiny_spline.h"

void initPlog(); // 初始化日志类
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CrashHandler::Init(L"data/debug");  // 初始化Mini转储
    initPlog();                         // 初始化日志类

    // TinySplineqqq testSpline;
    // testSpline.runcv();

    test_FrmVisionDisplay testDisplay;
    testDisplay.show();
    testDisplay.displayImage("E:/work/车门门环拼接/image/背面打光/Splice_20251027_092312509.bmp");

    // JointView w;
    // w.show();

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

// #include <plog/Init.h>
// #include <plog/Initializers/ConsoleInitializer.h>
// #include <plog/Initializers/RollingFileInitializer.h>
// #include <plog/Log.h>

// #include <QApplication>

// #include "src/ui/cis_camera_image.h"

// void initPlog();  // 初始化日志类
// int main(int argc, char *argv[]) {
//     QApplication a(argc, argv);
//     initPlog();
//     CISWidget w;
//     w.show();
//     return a.exec();
// }
// // 初始化日志类
// void initPlog() {
//     // 日志信息分类等级: none = 0, fatal = 1, error = 2, warning = 3, info = 4, debug = 5, verbose = 6
//     // 设置初始化等级后, 等级『数值大于』设置值的日志信息就会被『忽略』
//     plog::init(plog::debug, "./data/log/log.csv", 1000000000, 100);
//     static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
//     plog::get()->addAppender(&consoleAppender);  // Also add logging to the console.
// }
#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>

#include <QCoreApplication>
#include <QDebug>
#include <memory>

#include "src/cameraFactory/dalsaCameralink/external_exe_runner.h"
void initPlog();  // 初始化日志类
int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // 初始化 plog 日志
    initPlog();

    // 用智能指针管理 ExternalExeRunner
    std::shared_ptr<ExternalExeRunner> runner = std::make_shared<ExternalExeRunner>();

    // 启动外部 exe（示例 mytool.exe）
    if (runner->start("./data/CISConfig/externExE/ConfigCIS.exe", {"--help"})) {
        runner->writeInput("some command");
    }

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

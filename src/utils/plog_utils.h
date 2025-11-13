#ifndef PLOG_UTILS_H
#define PLOG_UTILS_H

#include <plog/Init.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>
#include <plog/Formatters/MessageOnlyFormatter.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace PlogUtils {

/**
 * @brief 初始化plog日志系统
 *
 * 配置日志系统，包括：
 * - 文件日志：保存到带日期的txt文件中，包含完整的时间戳等信息
 * - 控制台日志：只显示消息内容，不包含时间戳等前缀信息
 *
 * @param logLevel 日志级别 (debug=5, info=4, warning=3, error=2, fatal=1, none=0)
 * @details 使用内联函数可避免多重定义错误：在头文件中定义普通函数，并且这个头文件被多个源文件包含时，
 *                                     每个源文件都会有一份该函数的定义，链接时会产生"多重定义"错误。内联函数可以避免这个问题。
 *                                     内联函数允许在头文件中定义函数体，而不会引起链接错误
 */
inline void initPlog(plog::Severity logLevel = plog::debug) {
    // 生成带日期的日志文件名
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time_t);

    std::ostringstream filename;
    filename << "./data/log/log_"
             << std::put_time(&tm, "%Y%m%d")
             << ".txt";

    // 初始化文件日志
    plog::init(logLevel, filename.str().c_str(), 1000000, 100);

    // 添加控制台日志
    static plog::ColorConsoleAppender<plog::MessageOnlyFormatter> consoleAppender;
    plog::get()->addAppender(&consoleAppender);
}

} // namespace plog_utils

#endif // PLOG_UTILS_H

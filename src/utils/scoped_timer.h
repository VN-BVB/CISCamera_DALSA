#ifndef SCOPED_TIMER_H
#define SCOPED_TIMER_H

#include <string>
#include <chrono>
#include <plog/Log.h>

class ScopedTimer
{
public:
    explicit ScopedTimer(const std::string& operationName)
        : m_operationName(operationName),
        m_startTime(std::chrono::high_resolution_clock::now()){}

    ~ScopedTimer() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime).count();

        PLOG_INFO << m_operationName << "耗时：" << duration << "毫秒";
    }

private:
    std::string m_operationName;
    std::chrono::high_resolution_clock::time_point m_startTime;
};

#endif // SCOPED_TIMER_H

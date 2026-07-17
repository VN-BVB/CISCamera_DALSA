// #ifndef PLOG_IDE_APPENDER_H
// #define PLOG_IDE_APPENDER_H

// #include <cstdio>

// #include <QByteArray>
// #include <QString>

// #include <plog/Appenders/IAppender.h>
// #include <plog/Formatters/TxtFormatter.h>
// #include <plog/Util.h>

// // Qt Creator decodes its captured stdout using the Windows ANSI code page.
// // Convert plog's UTF-8 output at this process boundary only.
// class PlogIdeAppender : public plog::IAppender {
// public:
//     void write(const plog::Record& record) override {
//         const plog::util::nstring formatted = plog::TxtFormatter::format(record);
// #if PLOG_CHAR_IS_UTF8
//         const QByteArray local = QString::fromUtf8(formatted.c_str()).toLocal8Bit();
// #else
//         const QByteArray local = QString::fromWCharArray(formatted.c_str()).toLocal8Bit();
// #endif

//         plog::util::MutexLock lock(m_mutex);
//         std::fwrite(local.constData(), 1, static_cast<size_t>(local.size()), stdout);
//         std::fflush(stdout);
//     }

// private:
//     plog::util::Mutex m_mutex;
// };

// #endif  // PLOG_IDE_APPENDER_H

#include "plog_qt_appender.h"

#include <plog/Record.h>
#include <plog/Severity.h>
#include <plog/Util.h>

PlogQtAppender& PlogQtAppender::instance() {
    static PlogQtAppender s_instance;
    return s_instance;
}

PlogQtAppender::PlogQtAppender(QObject* parent) : QObject(parent) {}

void PlogQtAppender::write(const plog::Record& record) {
    if (record.getSeverity() > plog::info) {
        return;
    }

    const plog::util::nchar* raw = record.getMessage();
#if PLOG_CHAR_IS_UTF8
    QString qmsg = QString::fromUtf8(raw);
#else
    QString qmsg = QString::fromWCharArray(raw);
#endif

    emit logMessage(static_cast<int>(record.getSeverity()), qmsg);
}

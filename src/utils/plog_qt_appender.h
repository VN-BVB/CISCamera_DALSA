#ifndef PLOG_QT_APPENDER_H
#define PLOG_QT_APPENDER_H

#include <QObject>
#include <QString>

#include <plog/Appenders/IAppender.h>

class PlogQtAppender : public QObject, public plog::IAppender {
    Q_OBJECT

public:
    static PlogQtAppender& instance();

    void write(const plog::Record& record) override;

signals:
    void logMessage(int severity, const QString& message);

private:
    PlogQtAppender(QObject* parent = nullptr);
};

#endif  // PLOG_QT_APPENDER_H

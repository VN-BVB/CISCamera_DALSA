#ifndef LOG_PANEL_H
#define LOG_PANEL_H

#include <QTextEdit>
#include <QWidget>

#include "src/utils/plog_qt_appender.h"

class LogPanel : public QWidget {
    Q_OBJECT

public:
    enum class LogLevel { Info, Warning, Error, Success };

    explicit LogPanel(QWidget *parent = nullptr);

    void clear();

public slots:
    void onLogReceived(const QString &message);

private:
    void appendLog(const QString &message, LogLevel level = LogLevel::Info);
    void setupUi();
    QString getTimestamp() const;
    QString getLevelString(LogLevel level) const;
    QString getLevelColor(LogLevel level) const;

    static constexpr const char* kColorInfo = "#2196F3";
    static constexpr const char* kColorWarning = "#FFC107";
    static constexpr const char* kColorError = "#F44336";
    static constexpr const char* kColorSuccess = "#4CAF50";

private slots:
    void onPlogMessage(int severity, const QString &message);

private:
    QTextEdit* m_logTextEdit;
};

#endif  // LOG_PANEL_H

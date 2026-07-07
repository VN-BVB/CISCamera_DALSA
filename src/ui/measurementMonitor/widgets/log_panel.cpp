#include "log_panel.h"

#include <QBoxLayout>
#include <QDateTime>
#include <QLabel>
#include <QScrollBar>

#include <plog/Severity.h>

LogPanel::LogPanel(QWidget *parent)
    : QWidget(parent)
    , m_logTextEdit(nullptr)
{
    setupUi();

    connect(&PlogQtAppender::instance(), &PlogQtAppender::logMessage,
            this, &LogPanel::onPlogMessage);
}

void LogPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 5, 10, 10);
    mainLayout->setSpacing(5);

    QLabel* titleLabel = new QLabel(QString::fromUtf8("日志"), this);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);

    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_logTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_logTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mainLayout->addWidget(m_logTextEdit);
}

void LogPanel::appendLog(const QString &message, LogLevel level) {
    QString timestamp = getTimestamp();
    QString levelStr = getLevelString(level);
    QString color = getLevelColor(level);

    QString htmlMessage = QString("<span style=\"color: #808080;\">[%1]</span> "
                                  "<span style=\"color: %2;\">[%3]</span> "
                                  "<span style=\"color: #2D2D2D;\">%4</span>")
                              .arg(timestamp, color, levelStr, message);

    m_logTextEdit->append(htmlMessage);

    QScrollBar* sb = m_logTextEdit->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void LogPanel::clear() {
    m_logTextEdit->clear();
}

void LogPanel::onLogReceived(const QString &message) {
    appendLog(message, LogLevel::Info);
}

void LogPanel::onPlogMessage(int severity, const QString &message) {
    LogLevel level;
    switch (static_cast<plog::Severity>(severity)) {
        case plog::fatal:
        case plog::error:
            level = LogLevel::Error;
            break;
        case plog::warning:
            level = LogLevel::Warning;
            break;
        case plog::info:
        default:
            level = LogLevel::Info;
            break;
    }
    appendLog(message, level);
}

QString LogPanel::getTimestamp() const {
    return QDateTime::currentDateTime().toString("hh:mm:ss");
}

QString LogPanel::getLevelString(LogLevel level) const {
    switch (level) {
        case LogLevel::Info:
            return QString::fromUtf8("INFO");
        case LogLevel::Warning:
            return QString::fromUtf8("WARN");
        case LogLevel::Error:
            return QString::fromUtf8("ERROR");
        case LogLevel::Success:
            return QString::fromUtf8("OK");
        default:
            return QString::fromUtf8("INFO");
    }
}

QString LogPanel::getLevelColor(LogLevel level) const {
    switch (level) {
        case LogLevel::Info:
            return kColorInfo;
        case LogLevel::Warning:
            return kColorWarning;
        case LogLevel::Error:
            return kColorError;
        case LogLevel::Success:
            return kColorSuccess;
        default:
            return kColorInfo;
    }
}

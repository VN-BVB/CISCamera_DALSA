#include "log_panel.h"

#include <QBoxLayout>
#include <QDateTime>
#include <QLabel>
#include <QScrollBar>

LogPanel::LogPanel(QWidget *parent)
    : QWidget(parent)
    , m_logTextEdit(nullptr)
{
    setupUi();
}

void LogPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 5, 10, 10);
    mainLayout->setSpacing(5);

    QLabel* titleLabel = new QLabel(QString::fromUtf8("日志"), this);
    titleLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #AAAAAA;");
    mainLayout->addWidget(titleLabel);

    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_logTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_logTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mainLayout->addWidget(m_logTextEdit);

    QString style = R"(
        LogPanel {
            background-color: #1E1E1E;
            border-top: 1px solid #3D3D3D;
        }
        QTextEdit {
            background-color: #252526;
            color: #D4D4D4;
            border: 1px solid #3D3D3D;
            border-radius: 4px;
            font-family: Consolas, 'Courier New', monospace;
            font-size: 12px;
            padding: 5px;
        }
    )";
    this->setStyleSheet(style);
}

void LogPanel::appendLog(const QString &message, LogLevel level) {
    QString timestamp = getTimestamp();
    QString levelStr = getLevelString(level);
    QString color = getLevelColor(level);

    QString htmlMessage = QString("<span style=\"color: #808080;\">[%1]</span> "
                                  "<span style=\"color: %2;\">[%3]</span> "
                                  "<span style=\"color: #D4D4D4;\">%4</span>")
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
            return "#2196F3";
        case LogLevel::Warning:
            return "#FFC107";
        case LogLevel::Error:
            return "#F44336";
        case LogLevel::Success:
            return "#4CAF50";
        default:
            return "#2196F3";
    }
}

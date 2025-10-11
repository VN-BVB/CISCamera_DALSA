#pragma once

#include <QDir>
#include <QFileInfoList>
#include <QLibrary>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>

#include "plog/Log.h"

class ExternalExeRunner : public QObject {
    Q_OBJECT

public:
    explicit ExternalExeRunner(QObject *parent = nullptr);
    ~ExternalExeRunner();

    void addDllDirToPath(const QString &dllDir);
    bool start(const QString &exePath, const QStringList &args = {});
    void stop();
    void writeInput(const QString &input);

signals:
    void sendMessage2UI(const QString &msg);

private slots:
    void handleStdOut();
    void handleStdErr();
    void handleError(QProcess::ProcessError error);
    void handleFinished(int exitCode, QProcess::ExitStatus status);

private:
    QProcess *process;
};

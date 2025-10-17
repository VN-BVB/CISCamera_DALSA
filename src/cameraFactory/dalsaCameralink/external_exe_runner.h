#pragma once

// clang-format off
#include <windows.h>
#pragma comment(lib, "User32.lib")
#include <tlhelp32.h>
// clang-format on
#include <QDir>
#include <QFileInfoList>
#include <QLibrary>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QThread>
#include <QWidget>

#include "plog/Log.h"

class ExternalExeRunner : public QObject {
    Q_OBJECT

public:
    explicit ExternalExeRunner(QObject *parent = nullptr);
    ~ExternalExeRunner();

    void addDllDirToPath(const QString &dllDir);
    bool startEmbedded(const QString &exePath, const QStringList &args = {}, WId parentWinId = 0);
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
#ifdef Q_OS_WIN
    HWND findWindowByPid(DWORD pid);
#endif
};

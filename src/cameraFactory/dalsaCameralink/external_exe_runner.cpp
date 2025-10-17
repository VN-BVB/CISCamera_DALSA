#include "external_exe_runner.h"

ExternalExeRunner::ExternalExeRunner(QObject *parent) : QObject(parent), process(new QProcess(this)) {
    connect(process, &QProcess::readyReadStandardOutput, this, &ExternalExeRunner::handleStdOut);
    connect(process, &QProcess::readyReadStandardError, this, &ExternalExeRunner::handleStdErr);
    connect(process, &QProcess::errorOccurred, this, &ExternalExeRunner::handleError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ExternalExeRunner::handleFinished);
}

ExternalExeRunner::~ExternalExeRunner() { stop(); }

void ExternalExeRunner::addDllDirToPath(const QString &dllDir) {
    QString absPath = QDir(dllDir).absolutePath();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString path = env.value("PATH");

    if (!path.contains(absPath, Qt::CaseInsensitive)) {
        path = absPath + ";" + path;
    }

    qputenv("PATH", path.toUtf8());
    PLOGD << "PATH updated with: " << absPath.toStdString();
}

bool ExternalExeRunner::startEmbedded(const QString &exePath, const QStringList &args, WId parentWinId) {
    if (process->state() != QProcess::NotRunning) {
        PLOGE << "Process already running.";
        return false;
    }

    process->setProgram(exePath);
    process->setArguments(args);
    process->start();

    if (!process->waitForStarted(5000)) {
        PLOGE << "Failed to start process.";
        return false;
    }

    // 获取进程PID
    qint64 pid = process->processId();
    PLOGD << "Started PID=" << pid;

    // 等待窗口创建（可能需要延时）
    HWND hwnd = nullptr;
    for (int i = 0; i < 30; ++i) {
        hwnd = this->findWindowByPid(pid);
        if (hwnd) break;
        QThread::msleep(200);
    }

    if (hwnd) {
        PLOGD << "Embedding HWND=" << hwnd;
        // 设置父窗口（嵌入Qt控件中）
        SetParent(hwnd, (HWND)parentWinId);
        // 调整样式（去掉标题栏）
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style &= ~(WS_CAPTION | WS_THICKFRAME);
        SetWindowLong(hwnd, GWL_STYLE, style);
        // 调整位置
        RECT rc;
        GetClientRect((HWND)parentWinId, &rc);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, rc.right, rc.bottom, SWP_SHOWWINDOW);
    } else {
        PLOGE << "Cannot find window for process PID=" << pid;
    }

    return true;
}

void ExternalExeRunner::stop() {
    if (process->state() != QProcess::NotRunning) {
        process->terminate();
        if (!process->waitForFinished(3000)) {
            process->kill();
            process->waitForFinished();
        }
        PLOGD << "Exe stopped.";
    }
}

void ExternalExeRunner::writeInput(const QString &input) {
    if (process->state() == QProcess::Running) {
        process->write(input.toUtf8() + "\n");
        PLOGD << "Input written: " << input.toStdString();
    } else {
        PLOGE << "Process not running. Cannot write input.";
    }
}

void ExternalExeRunner::handleStdOut() {
    QString output = QString::fromUtf8(process->readAllStandardOutput());
    PLOGD << "STDOUT: " << output.toStdString();
    emit sendMessage2UI(output);
}

void ExternalExeRunner::handleStdErr() {
    QString errorOutput = QString::fromUtf8(process->readAllStandardError());
    PLOGE << "STDERR: " << errorOutput.toStdString();
    emit sendMessage2UI(errorOutput);
}

void ExternalExeRunner::handleError(QProcess::ProcessError error) {
    PLOGE << "Process error: " << error;
    emit sendMessage2UI(QString("Process error: %1").arg(error));
}

void ExternalExeRunner::handleFinished(int exitCode, QProcess::ExitStatus status) {
    PLOGD << "Process finished. ExitCode=" << exitCode
          << " Status=" << (status == QProcess::NormalExit ? "NormalExit" : "CrashExit");
    emit sendMessage2UI(QString("Process finished. ExitCode=%1").arg(exitCode));
}
// 辅助函数：根据PID找到顶层窗口句柄
HWND ExternalExeRunner::findWindowByPid(DWORD pid) {
    HWND hwnd = GetTopWindow(nullptr);
    while (hwnd) {
        DWORD windowPid;
        GetWindowThreadProcessId(hwnd, &windowPid);
        if (windowPid == pid && IsWindowVisible(hwnd)) return hwnd;
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return nullptr;
}

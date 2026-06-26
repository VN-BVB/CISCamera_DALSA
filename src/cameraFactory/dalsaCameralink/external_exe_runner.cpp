#include "external_exe_runner.h"

#include <QDebug>
#include <QMetaObject>

#ifdef Q_OS_WIN
#pragma comment(lib, "User32.lib")
#endif

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
#ifdef Q_OS_WIN
        path = absPath + ";" + path;
#else
        path = absPath + ":" + path;
#endif
    }
    qputenv("PATH", path.toUtf8());
    qInfo() << "[ExternalExeRunner] PATH updated with:" << absPath;
}

bool ExternalExeRunner::start(const QString &exePath, const QStringList &args) {
    if (process->state() != QProcess::NotRunning) {
        qWarning() << "[ExternalExeRunner] Process already running.";
        return false;
    }

    process->setProgram(exePath);
    process->setArguments(args);
    process->start();

    if (!process->waitForStarted(5000)) {
        qWarning() << "[ExternalExeRunner] waitForStarted timeout. Exe:" << exePath << "Error:" << process->errorString();
        return false;
    }

    qInfo() << "[ExternalExeRunner] Exe started (detached):" << exePath;
    return true;
}

void ExternalExeRunner::stop() {
#ifdef Q_OS_WIN
    // 先解除嵌入，防止 Qt UI 继续引用无效 HWND
    detachEmbeddedWindow();
    targetPid = 0;
    hostWinId = 0;
    if (embedCheckTimer) {
        embedCheckTimer->stop();
        embedCheckTimer->deleteLater();
        embedCheckTimer = nullptr;
    }
#endif

    if (process->state() != QProcess::NotRunning) {
        process->terminate();
        if (!process->waitForFinished(3000)) {
            process->kill();
            process->waitForFinished();
        }
        qInfo() << "[ExternalExeRunner] Exe stopped.";
    }
}

void ExternalExeRunner::writeInput(const QString &input) {
    if (process->state() == QProcess::Running) {
        process->write(input.toUtf8() + "\n");
        qInfo() << "[ExternalExeRunner] Input written:" << input;
    } else {
        qWarning() << "[ExternalExeRunner] Process not running. Cannot write input.";
    }
}

bool ExternalExeRunner::startEmbedded(const QString &exePath, const QStringList &args, WId parentWinId) {
#ifndef Q_OS_WIN
    Q_UNUSED(exePath)
    Q_UNUSED(args)
    Q_UNUSED(parentWinId)
    qWarning() << "[ExternalExeRunner] startEmbedded only supported on Windows.";
    return false;
#else
    if (!parentWinId) {
        qWarning() << "[ExternalExeRunner] startEmbedded requires valid hostWinId.";
        return false;
    }
    if (process->state() != QProcess::NotRunning) {
        qWarning() << "[ExternalExeRunner] Process already running.";
        return false;
    }

    // 清理旧状态
    detachEmbeddedWindow();
    targetPid = 0;
    hostWinId = parentWinId;

    process->setProgram(exePath);
    process->setArguments(args);
    process->start();

    if (!process->waitForStarted(5000)) {
        qWarning() << "[ExternalExeRunner] waitForStarted timeout. Exe:" << exePath << "Error:" << process->errorString();
        return false;
    }

    targetPid = static_cast<DWORD>(process->processId());
    qInfo() << "[ExternalExeRunner] Exe started (embedded), PID =" << targetPid;

    // // 等待窗口出现并嵌入
    // HWND hwnd = nullptr;
    // for (int i = 0; i < 40; ++i) {  // 最长约 8s
    //     hwnd = findWindowByPid(targetPid);
    //     if (isValidWindow(hwnd)) break;
    //     QThread::msleep(200);
    // }

    // if (isValidWindow(hwnd)) {
    //     embedWindowIntoHost(hwnd, reinterpret_cast<HWND>(hostWinId));
    //     embeddedHwnd = hwnd;
    //     fitEmbeddedToHost();
    // } else {
    //     qWarning() << "[ExternalExeRunner] Cannot find window for PID =" << targetPid;
    //     // 不立即返回 false：某些程序晚些时候才创建主窗体，交给定时器处理
    // }

    // 启动定时轮询：监控窗口是否被替换（例如按 OK 后弹新窗口）
    // if (!embedCheckTimer) {
    //     embedCheckTimer = new QTimer(this);
    //     connect(embedCheckTimer, &QTimer::timeout, this, &ExternalExeRunner::onEmbedCheckTimer);
    // }
    // embedCheckTimer->start(500);

    return true;
#endif
}

void ExternalExeRunner::resizeEmbeddedToHost() {
#ifdef Q_OS_WIN
    fitEmbeddedToHost();
#endif
}

void ExternalExeRunner::handleStdOut() {
    const QString output = QString::fromUtf8(process->readAllStandardOutput());
    qInfo() << "[ExternalExeRunner][STDOUT]\n" << output;
    emit sendMessage2UI(output);
}

void ExternalExeRunner::handleStdErr() {
    const QString errorOutput = QString::fromUtf8(process->readAllStandardError());
    qWarning() << "[ExternalExeRunner][STDERR]\n" << errorOutput;
    emit sendMessage2UI(errorOutput);
}

void ExternalExeRunner::handleError(QProcess::ProcessError error) {
    qWarning() << "[ExternalExeRunner] Process error:" << error;
#ifdef Q_OS_WIN
    // 进程异常时，立即解除嵌入，避免 Qt 后续访问无效 HWND
    detachEmbeddedWindow();
#endif
    emit sendMessage2UI(QString("Process error: %1").arg(error));
}

void ExternalExeRunner::handleFinished(int exitCode, QProcess::ExitStatus status) {
    qInfo() << "[ExternalExeRunner] Process finished. ExitCode=" << exitCode
            << " Status=" << (status == QProcess::NormalExit ? "NormalExit" : "CrashExit");
#ifdef Q_OS_WIN
    // 进程退出：解除嵌入，停轮询
    detachEmbeddedWindow();
    targetPid = 0;
    if (embedCheckTimer) {
        embedCheckTimer->stop();
    }
#endif
    emit sendMessage2UI(QString("Process finished. ExitCode=%1").arg(exitCode));
}

#ifdef Q_OS_WIN

// —— 辅助：枚举找到 PID 对应的“合适”顶层窗口 —— //
static BOOL CALLBACK EnumWindowsProcFindByPid(HWND hwnd, LPARAM lParam) {
    DWORD wpid = 0;
    GetWindowThreadProcessId(hwnd, &wpid);
    if (wpid != (DWORD)lParam) return TRUE;

    // 顶层可见 + 有窗口（过滤工具窗/不可见/子窗）
    LONG ex = GetWindowLong(hwnd, GWL_EXSTYLE);
    LONG st = GetWindowLong(hwnd, GWL_STYLE);
    if (IsWindowVisible(hwnd) && !(ex & WS_EX_TOOLWINDOW) && (st & WS_CAPTION || st & WS_POPUP)) {
        // 找到就把句柄存到 GWLP_USERDATA 不是好主意；直接通过一个静态变量传出
        SetLastError((DWORD)(ULONG_PTR)hwnd);  // small hack to pass back via GetLastError
        return FALSE;                          // stop enum
    }
    return TRUE;  // continue
}

HWND ExternalExeRunner::findWindowByPid(DWORD pid) const {
    HWND h = GetTopWindow(NULL);
    while (h) {
        DWORD wpid = 0;
        GetWindowThreadProcessId(h, &wpid);
        if (wpid == pid) {
            LONG ex = GetWindowLong(h, GWL_EXSTYLE);
            if (IsWindowVisible(h) && !(ex & WS_EX_TOOLWINDOW)) {
                return h;  // 找到第一个可见的顶层窗口
            }
        }
        h = GetNextWindow(h, GW_HWNDNEXT);
    }
    return nullptr;
}

void ExternalExeRunner::embedWindowIntoHost(HWND child, HWND host) {
    // 设为子窗口
    SetParent(child, host);

    // 去掉外部窗口的边框/标题栏等，设为子窗口风格
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_POPUP);
    style |= WS_CHILD;
    SetWindowLong(child, GWL_STYLE, style);

    LONG exStyle = GetWindowLong(child, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_APPWINDOW | WS_EX_TOOLWINDOW);
    SetWindowLong(child, GWL_EXSTYLE, exStyle);

    // 显示到最前并放到合适位置
    SetWindowPos(child, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

void ExternalExeRunner::detachEmbeddedWindow() {
    if (!isValidWindow(embeddedHwnd)) {
        embeddedHwnd = nullptr;
        return;
    }
    // 恢复为顶层窗口
    SetParent(embeddedHwnd, NULL);

    // 尝试恢复常见风格（非必须，但更友好）
    LONG style = GetWindowLong(embeddedHwnd, GWL_STYLE);
    style |= (WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
    style &= ~WS_CHILD;
    SetWindowLong(embeddedHwnd, GWL_STYLE, style);

    SetWindowPos(embeddedHwnd, HWND_TOP, 100, 100, 800, 600, SWP_SHOWWINDOW);
    embeddedHwnd = nullptr;
}

void ExternalExeRunner::fitEmbeddedToHost() const {
    if (!hostWinId || !isValidWindow(embeddedHwnd)) return;
    RECT rc{};
    GetClientRect(reinterpret_cast<HWND>(hostWinId), &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    if (w < 0 || h < 0) return;
    SetWindowPos(embeddedHwnd, HWND_TOP, 0, 0, w, h, SWP_SHOWWINDOW | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

void ExternalExeRunner::onEmbedCheckTimer() {
    if (!targetPid || !hostWinId) return;

    // 如果当前嵌入窗口已失效，尝试重新找到新窗口
    if (!isValidWindow(embeddedHwnd)) {
        HWND newHwnd = findWindowByPid(targetPid);
        if (isValidWindow(newHwnd)) {
            embedWindowIntoHost(newHwnd, reinterpret_cast<HWND>(hostWinId));
            embeddedHwnd = newHwnd;
            fitEmbeddedToHost();
        }
        return;
    }

    // 检查当前窗口是否仍属于外部进程
    DWORD hwndPid = 0;
    GetWindowThreadProcessId(embeddedHwnd, &hwndPid);
    if (hwndPid != targetPid) {
        HWND newHwnd = findWindowByPid(targetPid);
        if (isValidWindow(newHwnd) && newHwnd != embeddedHwnd) {
            embedWindowIntoHost(newHwnd, reinterpret_cast<HWND>(hostWinId));
            embeddedHwnd = newHwnd;
            fitEmbeddedToHost();
        }
        return;
    }

    // 如果当前窗口仍有效，周期性同步尺寸
    fitEmbeddedToHost();
}

#endif  // Q_OS_WIN

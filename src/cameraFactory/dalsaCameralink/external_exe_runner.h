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
#include <QTimer>
#include <QWidget>

#include "plog/Log.h"
class ExternalExeRunner : public QObject {
    Q_OBJECT
public:
    explicit ExternalExeRunner(QObject *parent = nullptr);
    ~ExternalExeRunner();

    // 环境 & 基础控制
    void addDllDirToPath(const QString &dllDir);
    bool start(const QString &exePath, const QStringList &args = {});
    void stop();
    void writeInput(const QString &input);

    // ✅ 嵌入式启动：将外部 EXE 的主窗口嵌入到 parentWinId 对应的 QWidget 中
    bool startEmbedded(const QString &exePath, const QStringList &args = {}, WId parentWinId = 0);

    // ✅ 宿主控件尺寸变化时调用，保持外部窗口铺满宿主
    void resizeEmbeddedToHost();

signals:
    void sendMessage2UI(const QString &msg);

private slots:
    void handleStdOut();
    void handleStdErr();
    void handleError(QProcess::ProcessError error);
    void handleFinished(int exitCode, QProcess::ExitStatus status);

#ifdef Q_OS_WIN
    void onEmbedCheckTimer();  // 定时检测外部窗口是否被替换/销毁
#endif

private:
    QProcess *process{nullptr};

#ifdef Q_OS_WIN
    // —— Windows 嵌入相关 —— //
    HWND embeddedHwnd{nullptr};  // 当前已嵌入的窗口
    DWORD targetPid{0};          // 外部进程 PID
    WId hostWinId{0};            // 宿主 QWidget 的 winId
    QTimer *embedCheckTimer{nullptr};

    // 根据 PID 寻找最合适的顶层主窗口（可见、非工具窗等）
    HWND findWindowByPid(DWORD pid) const;

    // 把外部窗口样式调整为子窗口并嵌入 host
    void embedWindowIntoHost(HWND child, HWND host);

    // 解除嵌入，安全恢复样式，防止 Qt 继续访问已销毁窗口
    void detachEmbeddedWindow();

    // 将当前 embeddedHwnd 调整到铺满 host
    void fitEmbeddedToHost() const;

    // 判断窗口是否有效
    static bool isValidWindow(HWND h) { return h && ::IsWindow(h); }
#endif
};

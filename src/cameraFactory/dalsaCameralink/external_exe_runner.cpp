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

bool ExternalExeRunner::start(const QString &exePath, const QStringList &args) {
    if (process->state() != QProcess::NotRunning) {
        PLOGE << "Process already running.";
        return false;
    }

    process->setProgram(exePath);
    process->setArguments(args);
    process->start();

    if (!process->waitForStarted(5000)) {
        PLOGE << "Failed to start exe: " << exePath.toStdString();
        return false;
    }

    PLOGD << "Exe started: " << exePath.toStdString();
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
    PLOGD << "Process finished. ExitCode=" << exitCode << " Status=" << (status == QProcess::NormalExit ? "NormalExit" : "CrashExit");
    emit sendMessage2UI(QString("Process finished. ExitCode=%1").arg(exitCode));
}

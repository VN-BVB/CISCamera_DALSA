#ifndef DALSACAMERA_H
#define DALSACAMERA_H

#include <SapClassBasic.h>

#include <QImage>
#include <QObject>
#include <atomic>
#include <mutex>
#include <thread>

#include "../AbstractCamera.h"

class DalsaCamera : public AbstractCamera {
    Q_OBJECT
public:
    enum class TriggerMode { AutoFromCCF, Internal, External };

    explicit DalsaCamera(QObject* parent = nullptr);
    ~DalsaCamera();

    bool initCamera(const QString& configPath) override;
public slots:
    void startGrab() override;
    void stopGrab() override;
    void freezeGrab(bool freeze) override;
    void saveFrames(bool enable, int maxFrames = 1) override;

    // 可选：覆盖 CCF 的触发模式
    void setTriggerMode(TriggerMode mode) { m_triggerMode = mode; }

signals:
    // 由 AbstractCamera 声明（重复声明不会冲突，但不是必需）
    // void newImageReady(const cv::Mat& image);
    // void grabFinished();

private:
    // Sapera 用的静态回调（传入 context）
    static void XferCallBack(SapXferCallbackInfo* pInfo);

    // 实例方法，安全在 Qt 主线程或通过 invokeMethod 调用
    Q_SLOT void handleImageFromCallback(const cv::Mat& img);

private:
    // Sapera objects
    SapAcquisition* m_Acquisition;
    SapBufferWithTrash* m_Buffers;
    SapTransfer* m_Xfer;
    SapView* m_View;
    BYTE* m_pData;

    // 状态
    std::atomic<bool> m_running;
    std::atomic<bool> m_freeze;
    std::atomic<bool> m_saveEnabled;
    int m_maxFrames;
    int m_frameCount;

    // thread for internal mode
    std::thread m_worker;
    std::mutex m_mutex;

    // image params (read from CCF / buffers)
    int m_width;
    int m_height;
    QString m_ccfPath;

    // trigger mode
    TriggerMode m_triggerMode;
};

#endif  // DALSACAMERA_H

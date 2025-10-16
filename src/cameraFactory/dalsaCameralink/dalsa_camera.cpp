#include "dalsa_camera.h"

#include <QDebug>
#include <QMetaObject>
#include <sstream>
DalsaCamera::DalsaCamera(QObject* parent)
    : AbstractCamera(parent),
      m_Acquisition(nullptr),
      m_Buffers(nullptr),
      m_Xfer(nullptr),
      m_View(nullptr),
      m_pData(nullptr),
      m_running(false),
      m_freeze(false),
      m_saveEnabled(false),
      m_maxFrames(0),
      m_frameCount(0),
      m_width(0),
      m_height(0),
      m_triggerMode(TriggerMode::External) {}

DalsaCamera::~DalsaCamera() {
    // stopGrab();

    if (m_worker.joinable()) m_worker.join();

    if (m_Xfer && *m_Xfer) m_Xfer->Destroy();
    if (m_Buffers && *m_Buffers) m_Buffers->Destroy();
    // if (m_View && *m_View) m_View->Destroy();
    if (m_Acquisition && *m_Acquisition) m_Acquisition->Destroy();

    delete m_Xfer;
    delete m_Buffers;
    delete m_View;
    delete m_Acquisition;
}
bool DalsaCamera::initCamera(const QString& configPath, int resourceIndex) {
    PLOGD << "DALSA采集卡初始化中...";
    // emit sendText(QString(u8"DALSA采集卡初始化中..."));
    m_ccfPath = configPath;
    cameraIndex = resourceIndex;

    char serverName[MAX_PATH];
    if (!SapManager::GetServerName(0, SapManager::ResourceAcq, serverName)) {
        PLOGE << "SapManager::GetServerName 获取失败";
        // emit sendText(QString(u8"SapManager::GetServerName 获取失败"));
        return false;
    }
    PLOGD << "ServerName = " << serverName;
    // emit sendText(QString(u8"ServerName = %1").arg(serverName));

    SapLocation loc(serverName, resourceIndex);
    m_Acquisition = new SapAcquisition(loc, m_ccfPath.toStdString().c_str());
    m_Buffers = new SapBufferWithTrash(2, m_Acquisition);
    m_View = new SapView(m_Buffers, SapHwndAutomatic);
    m_Xfer = new SapAcqToBuf(m_Acquisition, m_Buffers, XferCallBack, this);
    m_pAcqDevice = new SapAcqDevice(loc);

    // ---- Acquisition ----
    if (!*m_Acquisition) {
        PLOGD << "Acquisition 对象未创建，尝试 Create()...";
        // emit sendText(QString(u8"Acquisition 对象未创建，尝试 Create()..."));
        if (!m_Acquisition->Create()) {
            PLOGE << "m_Acquisition->Create() 失败";
            // emit sendText(QString(u8"m_Acquisition->Create() 失败"));
            return false;
        }
    }
    PLOGD << "Acquisition 创建成功";
    // emit sendText(QString(u8"Acquisition 创建成功"));

    // ---- Buffers ----
    if (!*m_Buffers) {
        PLOGD << "Buffers 对象未创建，尝试 Create()...";
        // emit sendText(QString(u8"Buffers 对象未创建，尝试 Create()..."));
        if (!m_Buffers->Create()) {
            PLOGE << "m_Buffers->Create() 失败";
            // emit sendText(QString(u8"m_Buffers->Create() 失败"));
            return false;
        }
    }
    PLOGD << "Buffers 创建成功";
    // emit sendText(QString(u8"Buffers 创建成功"));
    // ---- View ---- (可选)
    /*
    if (!*m_View) {
        PLOGD << "View 对象未创建，尝试 Create()...";
        // emit sendText(QString(u8"View 对象未创建，尝试 Create()..."));
        if (!m_View->Create()) {
            PLOGE << "m_View->Create() 失败";
            // emit sendText(QString(u8"m_View->Create() 失败"));
            return false;
        }
    }
    PLOGD << "View 创建成功";
    // emit sendText(QString(u8"View 创建成功"));
    */
    // ---- Xfer ----
    if (!*m_Xfer) {
        PLOGD << "Xfer 对象未创建，尝试 Create()...";
        // emit sendText(QString(u8"Xfer 对象未创建，尝试 Create()..."));
        if (!m_Xfer->Create()) {
            PLOGE << "m_Xfer->Create() 失败";
            // emit sendText(QString(u8"m_Xfer->Create() 失败"));
            return false;
        }
    }
    PLOGD << "Xfer 创建成功";
    // emit sendText(QString(u8"Xfer 创建成功"));
    // ---- AcqDevice ----
    // if (m_pAcqDevice && !*m_pAcqDevice) {
    //     if (!m_pAcqDevice->Create()) {
    //         PLOGE << "m_pAcqDevice->Create() 失败";
    // emit sendText(QString(u8"m_pAcqDevice->Create() 失败"));
    //         delete m_pAcqDevice;
    //         m_pAcqDevice = nullptr;
    //     } else {
    //         PLOGD << "AcqDevice 创建成功";
    // emit sendText(QString(u8"AcqDevice 创建成功"));
    //     }
    // }

    if (m_Xfer && m_Xfer->GetPair(0)) {
        m_Xfer->GetPair(0)->SetCycleMode(SapXferPair::CycleNextWithTrash);
        PLOGD << "XferPair 设置为 CycleNextWithTrash";
        // emit sendText(QString(u8"XferPair 设置为 CycleNextWithTrash"));
    }

    m_width = m_Buffers->GetWidth();
    m_height = m_Buffers->GetHeight();
    PLOGD << "DALSA采集卡初始化完成, 分辨率 = " << m_width << " x " << m_height;
    // emit sendText(QString(u8"DALSA采集卡初始化完成, 分辨率 = %1 x %2").arg(m_width).arg(m_height));

    int serverCount = SapManager::GetServerCount();
    for (int i = 0; i < serverCount; i++) {
        char name[CORSERVER_MAX_STRLEN] = {0};
        if (SapManager::GetServerName(i, name, sizeof(name))) {
            int resCount = SapManager::GetResourceCount(i, SapManager::ResourceAcq);
            PLOGD << "Server[" << i << "] = " << name << ", ResourceCount = " << resCount;
            // emit sendText(QString(u8"Server[%1] = %2, ResourceCount = %3").arg(i).arg(name).arg(resCount));
        }
    }
    return true;
}

void DalsaCamera::startGrab() {
    if (m_running) {
        emit sendText(QString(u8"已开始采集"));
        return;
    }

    m_running = true;
    m_freeze = false;
    m_frameCount = 0;

    if (!m_Xfer) return;

    // 根据触发模式，初始化 Xfer 的触发类型
    switch (m_triggerMode) {
        case TriggerMode::Internal:
            // 内触发 → 设置硬件内部连续采集

            if (m_Xfer->GetPair(0)) {
                m_Xfer->GetPair(0)->SetCycleMode(SapXferPair::CycleNextWithTrash);
            }
            break;

        case TriggerMode::External:
            break;

        case TriggerMode::AutoFromCCF:
            // 自动 → 按 CCF 文件里配置的触发模式
            break;
    }
    PLOGD << "相机开始采集图像";
    emit sendText(QString(u8"相机开始采集图像"));
    // 启动采集（无论内/外/CCF）
    if (!m_Xfer->IsGrabbing()) {
        m_Xfer->Grab();
    }
}

void DalsaCamera::stopGrab() {
    m_running = false;
    if (m_Xfer) m_Xfer->Abort();
    if (m_worker.joinable()) m_worker.join();
    m_frameCount = 0;
    PLOGD << " 相机停止采集图像";
    emit sendText(QString(u8"相机停止采集图像"));
    emit grabFinished();
}

void DalsaCamera::freezeGrab(bool freeze) {
    m_freeze = freeze;
    if (m_Xfer) {
        if (freeze) {
            m_Xfer->Freeze();  // 停采集
            PLOGD << "采集已冻结";
            emit sendText(QString(u8"采集已冻结"));
        } else {
            m_Xfer->Grab();  // 继续采集
            PLOGD << "采集继续";
            emit sendText(QString(u8"采集继续"));
        }
    }
    m_frameCount = 0;
}

void DalsaCamera::saveFrames(bool enable, int maxFrames) {
    m_saveEnabled = enable;
    m_maxFrames = maxFrames;
    m_frameCount = 0;
}
// 触发一次采集
bool DalsaCamera::softwareTrigger() {
    if (!m_Acquisition) {
        PLOGE << "Acquisition 未初始化";
        emit sendText(QString(u8"Acquisition 未初始化"));
        return false;
    }
    // 注意：要在 CCF 配置里设置好 External Trigger Source = Software
    // 否则这个调用不会真正触发
    if (!m_Acquisition->SoftwareTrigger(SapAcquisition::SoftwareTriggerExtFrame)) {
        PLOGE << "SoftwareTrigger 调用失败";
        emit sendText(QString(u8"SoftwareTrigger 调用失败"));
        return false;
    }

    PLOGD << "SoftwareTrigger 触发成功";
    emit sendText(QString(u8"SoftwareTrigger 触发成功"));
    return true;
}
// =================== 回调部分 ===================
void DalsaCamera::XferCallBack(SapXferCallbackInfo* pInfo) {
    if (!pInfo) return;

    auto* cam = reinterpret_cast<DalsaCamera*>(pInfo->GetContext());
    if (!cam) return;
    if (cam->m_freeze) {
        return;
    }

    int bufferIndex = pInfo->GetPairIndex();
    void* data = nullptr;

    if (cam->m_Buffers && bufferIndex >= 0) {
        cam->m_Buffers->GetAddress(bufferIndex, &data);
    }

    if (!data) return;

    static cv::Mat mat;
    // 按照相机数据格式转换成 cv::Mat
    if (cam->m_Buffers->GetFormat() == SapFormatRGB888) {
        mat = cv::Mat(cam->m_height, cam->m_width, CV_8UC3, data);
    } else if (cam->m_Buffers->GetFormat() == SapFormatMono8) {
        mat = cv::Mat(cam->m_height, cam->m_width, CV_8UC1, data);
    } else if (cam->m_Buffers->GetFormat() == SapFormatMono16) {
        mat = cv::Mat(cam->m_height, cam->m_width, CV_16UC1, data);
    } else {
        std::cout << "none mode for converting to img " << std::endl;
        // 不支持的格式
        return;
    }
    // cv::imshow("aaaa", mat);
    // cv::waitKey(1);

    QMetaObject::invokeMethod(cam, "handleImageFromCallback", Qt::QueuedConnection, Q_ARG(cv::Mat, mat));

    // 保存逻辑（依然用 SapBuffer 保存，避免 OpenCV 再写一次大图）
    if (cam->m_saveEnabled) {
        std::stringstream ss;
        ss << "D:\\test\\bmp\\" << cam->m_frameCount << ".bmp";
        cam->m_Buffers->Save(ss.str().c_str(), "-format bmp");
        cam->m_frameCount++;
        if (cam->m_maxFrames > 0 && cam->m_frameCount >= cam->m_maxFrames) {
            cam->m_saveEnabled = false;
        }
    }
    cam->m_frameCount++;
}

// =================== 信号 ===================
void DalsaCamera::handleImageFromCallback(const cv::Mat& mat) {
    // PLOGD << "发送图像帧";
    emit sendText(QString(u8"相机%1发送图像帧").arg(cameraIndex));
    emit sendNewImageReady(mat.clone());
}

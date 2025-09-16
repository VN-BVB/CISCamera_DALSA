#include "DalsaCamera.h"

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
      m_qformat(QImage::Format_Invalid),
      m_triggerMode(TriggerMode::Internal) {}

DalsaCamera::~DalsaCamera() {
    // stopGrab();

    if (m_worker.joinable()) m_worker.join();

    if (m_Xfer && *m_Xfer) m_Xfer->Destroy();
    if (m_Buffers && *m_Buffers) m_Buffers->Destroy();
    if (m_View && *m_View) m_View->Destroy();
    if (m_Acquisition && *m_Acquisition) m_Acquisition->Destroy();

    delete m_Xfer;
    delete m_Buffers;
    delete m_View;
    delete m_Acquisition;
}

bool DalsaCamera::initCamera(const QString& configPath) {
    m_ccfPath = configPath;

    char serverName[MAX_PATH];
    SapManager::GetServerName(0, SapManager::ResourceAcq, serverName);
    SapLocation loc(serverName, 0);

    m_Acquisition = new SapAcquisition(loc, m_ccfPath.toStdString().c_str());
    m_Buffers = new SapBufferWithTrash(2, m_Acquisition);
    m_View = new SapView(m_Buffers, SapHwndAutomatic);

    // 注意传 this 作为 context
    m_Xfer = new SapAcqToBuf(m_Acquisition, m_Buffers, XferCallBack, this);

    if (!*m_Acquisition && !m_Acquisition->Create()) return false;
    if (!*m_Buffers && !m_Buffers->Create()) return false;
    if (!*m_View && !m_View->Create()) return false;
    if (!*m_Xfer && !m_Xfer->Create()) return false;

    if (m_Xfer && m_Xfer->GetPair(0)) m_Xfer->GetPair(0)->SetCycleMode(SapXferPair::CycleNextWithTrash);

    m_width = m_Buffers->GetWidth();
    m_height = m_Buffers->GetHeight();

    m_qformat = mapSapFormatToQImage(m_Buffers->GetFormat());

    return true;
}

void DalsaCamera::startGrab() {
    if (m_running) return;

    m_running = true;
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

    // 启动采集（无论内/外/CCF）
    if (!m_Xfer->IsGrabbing()) {
        m_Xfer->Grab();
    }
}

void DalsaCamera::stopGrab() {
    m_running = false;
    if (m_Xfer) m_Xfer->Abort();
    if (m_worker.joinable()) m_worker.join();
    emit grabFinished();
}

void DalsaCamera::freezeGrab(bool freeze) { m_freeze = freeze; }

void DalsaCamera::saveFrames(bool enable, int maxFrames) {
    m_saveEnabled = enable;
    m_maxFrames = maxFrames;
    m_frameCount = 0;
}
// 将 Sapera 图像格式映射到 QImage 格式
QImage::Format DalsaCamera::mapSapFormatToQImage(SapFormat fmt) const {
    switch (fmt) {
        case SapFormatMono8:
            return QImage::Format_Grayscale8;
        case SapFormatRGB888:
            return QImage::Format_RGB888;
        case SapFormatMono16:
            return QImage::Format_Grayscale16;
        default:
            return QImage::Format_Grayscale8;
    }
}

// =================== 回调部分 ===================
void DalsaCamera::XferCallBack(SapXferCallbackInfo* pInfo) {
    if (!pInfo) return;

    auto* cam = reinterpret_cast<DalsaCamera*>(pInfo->GetContext());
    if (!cam) return;

    int bufferIndex = pInfo->GetPairIndex();
    void* data = nullptr;

    if (cam->m_Buffers && bufferIndex >= 0) {
        cam->m_Buffers->GetAddress(bufferIndex, &data);
    }

    if (!data) return;

    cv::Mat mat;

    // 按照相机数据格式转换成 cv::Mat
    if (cam->m_qformat == QImage::Format_RGB888) {
        // OpenCV 默认是 BGR，这里如果要保持颜色一致，需要后续 cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        mat = cv::Mat(cam->m_height, cam->m_width, CV_8UC3, data).clone();
    } else if (cam->m_qformat == QImage::Format_Grayscale8) {
        mat = cv::Mat(cam->m_height, cam->m_width, CV_8UC1, data).clone();
    } else if (cam->m_qformat == QImage::Format_Grayscale16) {
        mat = cv::Mat(cam->m_height, cam->m_width, CV_16UC1, data).clone();
    } else {
        std::cout << "none mode for converting to img " << std::endl;
        // 不支持的格式
        return;
    }

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
}

// =================== 槽函数 ===================
void DalsaCamera::handleImageFromCallback(const cv::Mat& mat) { emit newImageReady(mat); }

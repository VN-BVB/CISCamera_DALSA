#include "DalsaCamera.h"

#include <QDebug>
#include <sstream>

DalsaCamera::DalsaCamera(QObject* parent)
    : AbstractCamera(parent),
      m_Acquisition(nullptr),
      m_Buffers(nullptr),
      m_Xfer(nullptr),
      m_View(nullptr),
      pData(nullptr),
      isStop(false),
      isFreeze(false),
      isSave(false),
      maxFrames(0),
      frameCount(0),
      width(0),
      height(0),
      qformat(QImage::Format_Invalid) {}

DalsaCamera::~DalsaCamera() {
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
    ccfPath = configPath;

    char serverName[MAX_PATH];
    SapManager::GetServerName(0, SapManager::ResourceAcq, serverName);
    SapLocation loc(serverName, 0);

    m_Acquisition = new SapAcquisition(loc, ccfPath.toStdString().c_str());
    m_Buffers = new SapBufferWithTrash(2, m_Acquisition);
    m_View = new SapView(m_Buffers, SapHwndAutomatic);
    m_Xfer = new SapAcqToBuf(m_Acquisition, m_Buffers, XferCallBack, m_View);

    if (!*m_Acquisition && !m_Acquisition->Create()) return false;
    if (!*m_Buffers && !m_Buffers->Create()) return false;
    if (!*m_View && !m_View->Create()) return false;
    if (m_Xfer && m_Xfer->GetPair(0)) m_Xfer->GetPair(0)->SetCycleMode(SapXferPair::CycleNextWithTrash);
    if (!*m_Xfer && !m_Xfer->Create()) return false;

    width = m_Buffers->GetWidth();
    height = m_Buffers->GetHeight();

    SapFormat format = m_Buffers->GetFormat();
    switch (format) {
        case SapFormatMono8:
            qformat = QImage::Format_Grayscale8;
            break;
        case SapFormatRGB888:
            qformat = QImage::Format_RGB888;
            break;
        case SapFormatMono16:
            qformat = QImage::Format_Grayscale16;
            break;
        default:
            qformat = QImage::Format_Grayscale8;
            break;
    }

    m_Buffers->GetAddress((void**)&pData);
    return true;
}

void DalsaCamera::startGrab() {
    isStop = false;
    if (!isRunning()) start();
}

void DalsaCamera::stopGrab() { isStop = true; }

void DalsaCamera::freezeGrab(bool freeze) { isFreeze = freeze; }

void DalsaCamera::saveFrames(bool enable, int maxFrames) {
    isSave = enable;
    this->maxFrames = maxFrames;
}
void DalsaCamera::run() {
    while (!isStop) {
        if (!isFreeze) {
            m_Xfer->Grab();
            QImage img(m_Buffers->GetWidth(), m_Buffers->GetHeight(), QImage::Format_Grayscale8);
            m_Buffers->GetAddress((void**)&pData);
            memcpy(img.bits(), pData, img.width() * img.height());
            emit newImageReady(img);

            if (isSave) {
                std::stringstream ss;
                ss << "D:\\test\\bmp\\" << frameCount << ".bmp";
                m_Buffers->Save(ss.str().c_str(), "-format bmp");
                frameCount++;
                if (frameCount >= maxFrames) isSave = false;
            }
        }
        QThread::usleep(1000);
    }
    emit grabFinished();
}
void DalsaCamera::XferCallBack(SapXferCallbackInfo* pInfo) {
    // if (data) {
    //     QImage img((uchar*)data, width, height, QImage::Format_Grayscale8);
    //     emit newImageReady(img);
    // }
}

#ifndef DALSACAMERA_H
#define DALSACAMERA_H

#include <SapClassBasic.h>

#include <QImage>
#include <QThread>

#include "../AbstractCamera.h"

class DalsaCamera : public AbstractCamera, public QThread {
    Q_OBJECT
public:
    explicit DalsaCamera(QObject* parent = nullptr);
    ~DalsaCamera();

    bool initCamera(const QString& configPath) override;
    void startGrab() override;
    void stopGrab() override;
    void freezeGrab(bool freeze) override;
    void saveFrames(bool enable, int maxFrames = 0) override;

protected:
    void run() override;

private:
    static void XferCallBack(SapXferCallbackInfo* pInfo);

    SapAcquisition* m_Acquisition;
    SapBufferWithTrash* m_Buffers;
    SapTransfer* m_Xfer;
    SapView* m_View;
    BYTE* pData;

    bool isStop;
    bool isFreeze;
    bool isSave;
    int maxFrames;
    int frameCount;

    int width;
    int height;
    QImage::Format qformat;
    QString ccfPath;
};

#endif  // DALSACAMERA_H

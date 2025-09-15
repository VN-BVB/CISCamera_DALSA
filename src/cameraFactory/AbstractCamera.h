#ifndef ABSTRACTCAMERA_H
#define ABSTRACTCAMERA_H

#include <QImage>
#include <QObject>
#include <QString>

class AbstractCamera : public QObject {
    Q_OBJECT
public:
    explicit AbstractCamera(QObject* parent = nullptr);
    virtual ~AbstractCamera();

    virtual bool initCamera(const QString& configPath) = 0;
    virtual void startGrab() = 0;
    virtual void stopGrab() = 0;
    virtual void freezeGrab(bool freeze) = 0;
    virtual void saveFrames(bool enable, int maxFrames = 0) = 0;

signals:
    void newImageReady(const QImage& image);  // 输出 Qt 图像
    void grabFinished();
};

#endif  // ABSTRACTCAMERA_H

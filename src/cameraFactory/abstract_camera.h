#ifndef ABSTRACT_CAMERA_H
#define ABSTRACT_CAMERA_H
// clang-format off
#include <plog/Log.h>
#include <QImage>
#include <QObject>
#include <QString>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
// clang-format off
class AbstractCamera : public QObject {
    Q_OBJECT
public:
    explicit AbstractCamera(QObject* parent = nullptr);
    virtual ~AbstractCamera();
    virtual bool initCamera(const QString& configPath) = 0;
public slots:
    virtual void startGrab() = 0;
    virtual void stopGrab() = 0;
    virtual void freezeGrab(bool freeze) = 0;
    virtual void saveFrames(bool enable, int maxFrames = 0) = 0;

signals:
    void sendNewImageReady(const cv::Mat& image);  // 输出 Qt 图像
    void grabFinished();
};

#endif  // ABSTRACT_CAMERA_H

#ifndef IMAGEPROCESSWORKER_H
#define IMAGEPROCESSWORKER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class ImageProcessWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessWorker(QObject *parent = nullptr);

public slots:
    void processImage(cv::Mat image);

signals:
    void imageProcessed(cv::Mat processedImage);
    void errorOccurred(const QString &error);
};

#endif // IMAGEPROCESSWORKER_H

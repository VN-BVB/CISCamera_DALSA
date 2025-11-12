#ifndef IMAGE_READ_WORKER_H
#define IMAGE_READ_WORKER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class ImageReadWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImageReadWorker(QObject *parent = nullptr);

public slots:
    void readImage(const QString &path);

signals:
    void imageRead(std::shared_ptr<cv::Mat> image);
    void errorOccurred(const QString &error);
};

#endif // IMAGE_READ_WORKER_H

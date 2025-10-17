#ifndef IMAGEREADWORKER_H
#define IMAGEREADWORKER_H

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

#endif // IMAGEREADWORKER_H

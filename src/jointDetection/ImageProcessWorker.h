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
    void imageProcessed(cv::Mat processedImage, std::vector<cv::Point2f> subpixelContour,
                        std::vector<std::vector<cv::Point>> pixelContour);
    void errorOccurred(const QString &error);

private:
    cv::Point2f zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius);
    std::vector<cv::Point2f> getSubpixelContourZernike(const cv::Mat &src,
                                                       const std::vector<cv::Point> &contour);

    std::vector<cv::Point2f> m_subpixelContour;
};

#endif // IMAGEPROCESSWORKER_H

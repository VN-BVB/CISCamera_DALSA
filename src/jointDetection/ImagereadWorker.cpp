#include "ImageReadWorker.h"

ImageReadWorker::ImageReadWorker(QObject *parent)
    : QObject{parent}
{}

void ImageReadWorker::readImage(const QString &path) {
    try{
        cv::Mat img = cv::imread(path.toStdString(), cv::IMREAD_GRAYSCALE);
        if (img.empty()) {
            emit errorOccurred("无法加载图像");
            return;
        }
        auto imagePtr = std::make_shared<cv::Mat>(img);
        emit imageRead(imagePtr);
    } 
    catch(const std::exception& e){
        emit errorOccurred(QString("读取图像出错：") + e.what());
    }
    
}

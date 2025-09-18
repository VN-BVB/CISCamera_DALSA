#include "ImageProcessWorker.h"

ImageProcessWorker::ImageProcessWorker(QObject *parent)
    : QObject{parent}
{}

void ImageProcessWorker::processImage(cv::Mat image) {
    try {
        cv::Mat cropped_img = image(cv::Rect(16000, 16000, 1900, 1900));
        cv::imwrite("D:/Cpp_Project/WeldseamMeasurement/tests/image/cropped_img.bmp", cropped_img);
        
        cv::Mat edge;
        cv::Canny(cropped_img, edge, 20, 40);
        
        emit imageProcessed(edge);
    }
    catch (const cv::Exception& e) {
        emit errorOccurred(QString("处理图像时出错: ") + e.what());
    }
}

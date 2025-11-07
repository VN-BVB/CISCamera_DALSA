#include "image_process_worker.h"
#include "image_tools.h"
#include <QDebug>

#include "src/test/test_curve_seg.cpp"

ImageProcessWorker::ImageProcessWorker(QObject *parent) : QObject{parent} {}


void ImageProcessWorker::processImage(std::shared_ptr<cv::Mat> image) {
    try {

        ImageTools imageTools;
        // cv::Mat croppedImg = (*image)(cv::Rect(5696, 7273, 180, 2083));
        // cv::imwrite("D:/Cpp_Project/WeldseamMeasurement/tests/image/cropped_img.bmp", croppedImg);
        cv::Mat croppedImg = *image; // 直接读裁剪后的图，不用再裁剪

        auto jointSeam = std::make_shared<JointSeam>(croppedImg);
        jointSeam->run();
        emit imageProcessed(image, jointSeam);
    } catch (const cv::Exception &e) {
        emit errorOccurred(QString("处理图像时出错: ") + e.what());
    }
}

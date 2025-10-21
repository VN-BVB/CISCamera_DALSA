#include "image_process_worker.h"
#include "image_tools.h"
#include "contourProcess/contour_curve.h"
#include "contourProcess/joint_seam.h"
#include <QDebug>

ImageProcessWorker::ImageProcessWorker(QObject *parent) : QObject{parent} {}


void ImageProcessWorker::processImage(std::shared_ptr<cv::Mat> image) {
    try {

        ImageTools imageTools;
        // cv::Mat croppedImg = image(cv::Rect(16000, 16000, 1900, 1900));
        // cv::imwrite("D:/Cpp_Project/WeldseamMeasurement/tests/image/cropped_img.bmp", croppedImg);
        cv::Mat croppedImg = *image; // 直接读裁剪后的图，不用再裁剪

        JointSeam jointSeam = JointSeam{croppedImg};
        jointSeam.run();

        // @TODO:在图上画出角点和拟合直线
        std::vector<ContourCurve> contourCurves = jointSeam.getContourCurves();
        std::vector<std::vector<cv::Point2f>> subpixelContours;
        subpixelContours.push_back(contourCurves[1].getSubpixelContours());
        std::vector<std::vector<cv::Point>> pixelContour;
        pixelContour.push_back(contourCurves[1].getPixelContour());
        std::vector<cv::Vec4f> fitlines;
        for (auto lineSegment :  contourCurves[1].getLineSegments())
        {
            fitlines.push_back(lineSegment.getLineEquation());
        }
        auto resultImage = std::make_shared<cv::Mat>(croppedImg);
        emit imageProcessed(resultImage, subpixelContours, pixelContour, fitlines);
    } catch (const cv::Exception &e) {
        emit errorOccurred(QString("处理图像时出错: ") + e.what());

    }
}

// 去除轮廓两端的一部分
std::vector<cv::Point2f> ImageProcessWorker::trimContourEnds(const std::vector<cv::Point2f>& contour, float trimRatio) {
    std::vector<cv::Point2f> trimmedContour;

    if (contour.empty() || trimRatio <= 0.0f || trimRatio >= 0.5f) {
        // 如果轮廓为空或trimRatio不在有效范围内，返回原始轮廓
        return contour;
    }

    // 计算需要去除的点数
    int totalPoints = static_cast<int>(contour.size());
    int pointsToRemove = static_cast<int>(totalPoints * trimRatio);

    if (pointsToRemove * 2 >= totalPoints) {
        // 如果要去除的点数过多，返回空轮廓
        return trimmedContour;
    }

    // 保留中间部分，去除两端
    int startIndex = pointsToRemove;
    int endIndex = totalPoints - pointsToRemove;

    for (int i = startIndex; i < endIndex; ++i) {
        trimmedContour.push_back(contour[i]);
    }

    return trimmedContour;
}

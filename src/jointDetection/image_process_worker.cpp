#include "image_process_worker.h"
#include "image_tools.h"
#include "contourProcess/contour_curve.h"
#include <QDebug>

#include "src/test/test_curve_seg.cpp"

ImageProcessWorker::ImageProcessWorker(QObject *parent) : QObject{parent} {}


void ImageProcessWorker::processImage(std::shared_ptr<cv::Mat> image) {
    try {

        ImageTools imageTools;
        // cv::Mat croppedImg = image(cv::Rect(16000, 16000, 1900, 1900));
        // cv::imwrite("D:/Cpp_Project/WeldseamMeasurement/tests/image/cropped_img.bmp", croppedImg);
        cv::Mat croppedImg = *image; // 直接读裁剪后的图，不用再裁剪

        auto jointSeam = std::make_shared<JointSeam>(croppedImg);
        jointSeam->run();


        // @TODO:在图上画出角点和拟合直线
        std::vector<ContourCurve> contourCurves = jointSeam->getContourCurves();
        std::vector<std::vector<cv::Point2f>> subpixelContours;
        subpixelContours.push_back(contourCurves[1].getSubpixelContours());

        std::vector<cv::Point2f> fitPoints0;
        std::vector<std::vector<cv::Point2f>> segments0;
        segments0 = contourCurves[0].getSegmentedSubpixelContours();
        fitPoints0 = segments0[1];
        std::vector<cv::Point2f> fitPoints1;
        std::vector<std::vector<cv::Point2f>> segments1;
        segments1 = contourCurves[1].getSegmentedSubpixelContours();
        fitPoints1 = segments1[1];
        // 将fitPoints0和fitPoints1合并成一个vector
        std::vector<cv::Point2f> fitPoints;
        fitPoints.reserve(fitPoints0.size() + fitPoints1.size()); // 预分配内存以提高效率
        fitPoints.insert(fitPoints.end(), fitPoints0.begin(), fitPoints0.end());
        fitPoints.insert(fitPoints.end(), fitPoints1.begin(), fitPoints1.end());

        std::vector<std::vector<cv::Point>> pixelContour;
        pixelContour.push_back(contourCurves[1].getPixelContour());
        std::vector<cv::Vec4f> fitlines;
        for (auto& contourCurve :  contourCurves)
        {
            for (auto& line : contourCurve.getLines())
            fitlines.push_back(line);
        }
        auto resultImage = std::make_shared<cv::Mat>(croppedImg);
        emit imageProcessed(resultImage, jointSeam);
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

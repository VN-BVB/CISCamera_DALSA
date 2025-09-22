#include "ImageProcessWorker.h"
#include <QDebug>

ImageProcessWorker::ImageProcessWorker(QObject *parent)
    : QObject{parent}
{}

// Zernike亚像素偏移计算辅助函数
cv::Point2f ImageProcessWorker::zernikeSubpixel(const cv::Mat &gray, const cv::Point2f &edgePoint, int radius)
{
    // 提取边缘点邻域
    cv::Rect roi(cv::Point(std::max(0, int(edgePoint.x - radius)), std::max(0, int(edgePoint.y - radius))),
                 cv::Size(2 * radius + 1, 2 * radius + 1));
    roi &= cv::Rect(0, 0, gray.cols, gray.rows);
    cv::Mat roiImg = gray(roi);

    // 计算Zernike矩（简化实现，实际需根据论文公式计算）
    // 此处为示例，实际应用需实现完整的Zernike多项式计算
    double m00 = cv::moments(roiImg).m00;
    double m10 = cv::moments(roiImg).m10;
    double m01 = cv::moments(roiImg).m01;

    if (m00 < 1e-6)
        return edgePoint;

    // 亚像素偏移计算（基于Zernike矩特性）
    float dx = (m10 / m00) - radius;
    float dy = (m01 / m00) - radius;

    return cv::Point2f(edgePoint.x + dx, edgePoint.y + dy);
}

std::vector<cv::Point2f> ImageProcessWorker::getSubpixelContourZernike(const cv::Mat &src,
                                                   const std::vector<cv::Point> &contour)
{
    cv::Mat gray;
    if (src.channels() > 1)
    {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = src.clone();
    }

    // 高斯平滑预处理
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 1.0);

    std::vector<cv::Point2f> subpixelContour;
    for (const auto &p : contour)
    {
        // 使用Zernike矩法计算亚像素坐标
        cv::Point2f subpixel = zernikeSubpixel(gray, p, 3);
        subpixelContour.push_back(subpixel);
    }

    std::cout << "Zernike矩法提取亚像素坐标完成" << std::endl;
    return subpixelContour;
}


void ImageProcessWorker::processImage(cv::Mat image) {
    try {
        cv::Mat cropped_img = image(cv::Rect(16000, 16000, 1900, 1900));
        cv::imwrite("D:/Cpp_Project/WeldseamMeasurement/tests/image/cropped_img.bmp", cropped_img);
        
        cv::Mat edge;
        cv::Canny(cropped_img, edge, 20, 40);

        // 提取轮廓
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(edge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // 过滤轮廓
        std::vector<std::vector<cv::Point>> filtered_contours;
        for (auto &contour : contours)
        {
            if (contour.empty())
                continue;
            double length = cv::arcLength(contour, false);
            cv::Rect bbox = cv::boundingRect(contour);

            // 根据长度和宽高比过滤小噪声
            if (length > 100 &&     // 最小轮廓长度
                bbox.height > 10 && // 最小高度
                bbox.width > 10 &&  // 最小宽度
                (bbox.height * 1.0 / bbox.width < 3.0))
            { // 宽高比限制
                filtered_contours.push_back(contour);
            }
        }

        // 提取亚像素轮廓
        m_subpixelContour = getSubpixelContourZernike(cropped_img, filtered_contours[0]);

        if (filtered_contours.empty())
        {
            qDebug() << "未找到合适的轮廓";
            return;
        }

        emit imageProcessed(cropped_img, m_subpixelContour);
    }
    catch (const cv::Exception& e) {
        emit errorOccurred(QString("处理图像时出错: ") + e.what());
    }
}

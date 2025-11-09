#ifndef IMAGE_TOOLS_H
#define IMAGE_TOOLS_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

/*
 * @brief 工具类，先不整理一个模块用于存放工具类
 */
class ImageTools
{
public:
    ImageTools();
    // 绘制彩色轮廓并保存图像
    void drawColorfulContoursAndSave(const cv::Mat &src,
                                     const std::vector<std::vector<cv::Point>> &contours,
                                     const std::string &savePath);

    // 轮廓点去重
    std::vector<std::vector<cv::Point>> removeDuplicateContourPoints(const std::vector<std::vector<cv::Point>> &contours);
    // 过滤轮廓
    std::vector<std::vector<cv::Point>> filterContours(const std::vector<std::vector<cv::Point>>& contours,
                                                       double minLength = 1000.0,
                                                       int minHeight = 0,
                                                       int minWidth = 0,
                                                       double maxAspectRatio = 0);
    // 去除轮廓两端的一部分
    std::vector<cv::Point2f> trimContourEnds(const std::vector<cv::Point2f>& contour, float trimRatio);
};

#endif // IMAGE_TOOLS_H

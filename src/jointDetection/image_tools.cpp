#include "image_tools.h"

ImageTools::ImageTools() {}

/**
* @brief drawColorfulContoursAndSave 绘制彩色轮廓并保存图像
* @param src 输入源图像
* @param contours 输入轮廓点集
* @param savePath 输出图像保存路径
*/
void ImageTools::drawColorfulContoursAndSave(const cv::Mat &src,
                                 const std::vector<std::vector<cv::Point>> &contours,
                                 const std::string &savePath)
{
    // 绘制不同颜色的分割轮廓
    cv::Mat coloredSegmentsImage = cv::Mat::zeros(src.size(), CV_8UC3);
    std::vector<cv::Scalar> colors = {
        cv::Scalar(255, 0, 0),    // 蓝色
        cv::Scalar(0, 255, 0),    // 绿色
        cv::Scalar(0, 0, 255),    // 红色
        cv::Scalar(255, 255, 0),  // 青色
        cv::Scalar(255, 0, 255),  // 品红
        cv::Scalar(0, 255, 255),  // 黄色
        cv::Scalar(128, 0, 128),  // 紫色
        cv::Scalar(255, 165, 0)   // 橙色
    };

    for (size_t i = 0; i < contours.size(); ++i) {
        cv::Scalar color = colors[i % colors.size()];
        for (auto &point : contours[i]) {
            if (point.x >= 0 && point.x < coloredSegmentsImage.cols &&
                point.y >= 0 && point.y < coloredSegmentsImage.rows) {
                coloredSegmentsImage.at<cv::Vec3b>(point.y, point.x) = cv::Vec3b(
                    static_cast<uchar>(color[0]),
                    static_cast<uchar>(color[1]),
                    static_cast<uchar>(color[2])
                    );
            }
        }
    }
    cv::imwrite(savePath,coloredSegmentsImage);
}

#include <unordered_set>
#include <cmath>

// 自定义判等器（KeyEqual）：定义何时两个点被视为“相同”
struct PointEqual {
    bool operator()(const cv::Point2f& a, const cv::Point2f& b) const {
        // 设置一个允许的误差范围，例如 1e-5
        const float epsilon = 1e-5f;
        return std::abs(a.x - b.x) < epsilon && std::abs(a.y - b.y) < epsilon;
    }
};

// 自定义哈希器（Hash）：为点生成一个唯一的哈希值
struct PointHash {
    std::size_t operator()(const cv::Point2f& p) const {
        return std::hash<float>()(p.x) ^ (std::hash<float>()(p.y) << 1);
    }
};

// 使用自定义的哈希和判等类型定义 unordered_set
using PointSet = std::unordered_set<cv::Point2f, PointHash, PointEqual>;

/**
* @brief removeDuplicateContourPoints 轮廓点去重
* @param contours 输入轮廓点集
* @return 去重后的轮廓点集
*/
std::vector<std::vector<cv::Point>> ImageTools::removeDuplicateContourPoints(
    const std::vector<std::vector<cv::Point>>& contours)
{
    std::vector<std::vector<cv::Point>> unique_contours;

    // 定义PointSet类型（基于std::set的cv::Point比较）
    struct PointCompare {
        bool operator()(const cv::Point& a, const cv::Point& b) const {
            if (a.x == b.x) return a.y < b.y;
            return a.x < b.x;
        }
    };
    using PointSet = std::set<cv::Point, PointCompare>;

    for (auto &contour : contours)
    {
        // 用于记录已出现点的集合
        PointSet seen;
        std::vector<cv::Point> unique_points;

        for (const auto& point : contour) {
            // 尝试将点插入集合。如果插入成功，说明是第一次出现。
            if (seen.insert(point).second) {
                unique_points.push_back(point);
            }
        }
        unique_contours.push_back(unique_points);
    }

    return unique_contours;
}

/**
* @brief filterContours 过滤轮廓
* @param contours 输入轮廓点集
* @param minLength 最小轮廓长度
* @param minHeight 最小高度
* @param minWidth 最小宽度
* @param maxAspectRatio 最大宽高比
* @return 过滤后的轮廓点集
*/
std::vector<std::vector<cv::Point>> ImageTools::filterContours(const std::vector<std::vector<cv::Point>>& contours,
                                                               double minLength,
                                                               int minHeight,
                                                               int minWidth,
                                                               double maxAspectRatio)
{
    std::vector<std::vector<cv::Point>> filteredContours;

    for (auto &contour : contours)
    {
        if (contour.empty())
            continue;

        double length = cv::arcLength(contour, false);
        cv::Rect bbox = cv::boundingRect(contour);

        // 根据长度和宽高比过滤小噪声
        if (length > minLength &&     // 最小轮廓长度
            bbox.height > minHeight && // 最小高度
            bbox.width > minWidth &&  // 最小宽度
            (bbox.height * 1.0 / bbox.width > maxAspectRatio)) // 宽高比限制
        {
            filteredContours.push_back(contour);
        }
    }

    return filteredContours;
}


// 去除轮廓两端的一部分
std::vector<cv::Point2f> ImageTools::trimContourEnds(const std::vector<cv::Point2f>& contour, float trimRatio) {
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

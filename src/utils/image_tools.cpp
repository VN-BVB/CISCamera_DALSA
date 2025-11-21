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
    // 创建原图的副本，直接在原图上绘制彩色轮廓
    cv::Mat resultImage;
    if (src.channels() == 1) {
        // 如果是灰度图，转换为彩色图
        cv::cvtColor(src, resultImage, cv::COLOR_GRAY2BGR);
    } else {
        // 如果是彩色图，直接复制
        resultImage = src.clone();
    }

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
            if (point.x >= 0 && point.x < resultImage.cols &&
                point.y >= 0 && point.y < resultImage.rows) {
                resultImage.at<cv::Vec3b>(point.y, point.x) = cv::Vec3b(
                    static_cast<uchar>(color[0]),
                    static_cast<uchar>(color[1]),
                    static_cast<uchar>(color[2])
                    );
            }
        }
    }
    cv::imwrite(savePath, resultImage);
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

// 根据方向向量格式 (vx, vy, x0, y0) 绘制直线并保存图像
void ImageTools::drawLineAndSave(const cv::Mat& image, const cv::Vec4f& directionVector, const std::string& savePath) {
    // 创建图像的副本，避免修改原始图像
    cv::Mat resultImage = image.clone();

    // 如果输入是灰度图，转换为彩色图以便绘制彩色标记
    if (resultImage.channels() == 1) {
        cv::cvtColor(resultImage, resultImage, cv::COLOR_GRAY2BGR);
    }

    // 提取直线参数
    float vx = directionVector[0]; // 方向向量x分量
    float vy = directionVector[1]; // 方向向量y分量
    float x0 = directionVector[2]; // 直线上的点x坐标
    float y0 = directionVector[3]; // 直线上的点y坐标

    // 检查方向向量是否有效
    float length = std::sqrt(vx * vx + vy * vy);
    if (length < 1e-6) {
        std::cout << "无效的方向向量" << std::endl;
        return;
    }

    // 归一化方向向量
    float nx = vx / length;
    float ny = vy / length;

    // 计算直线与图像边界的交点
    std::vector<cv::Point2f> intersections;

    // 使用参数方程：x = x0 + t * nx, y = y0 + t * ny
    // 计算与图像边界的交点

    // 与左边界 (x=0) 的交点
    if (std::abs(nx) > 1e-6) {
        float t_left = (0 - x0) / nx;
        float y_left = y0 + t_left * ny;
        if (y_left >= 0 && y_left < resultImage.rows) {
            intersections.push_back(cv::Point2f(0, y_left));
        }
    }

    // 与右边界 (x=resultImage.cols-1) 的交点
    if (std::abs(nx) > 1e-6) {
        float t_right = (resultImage.cols - 1 - x0) / nx;
        float y_right = y0 + t_right * ny;
        if (y_right >= 0 && y_right < resultImage.rows) {
            intersections.push_back(cv::Point2f(resultImage.cols - 1, y_right));
        }
    }

    // 与上边界 (y=0) 的交点
    if (std::abs(ny) > 1e-6) {
        float t_top = (0 - y0) / ny;
        float x_top = x0 + t_top * nx;
        if (x_top >= 0 && x_top < resultImage.cols) {
            intersections.push_back(cv::Point2f(x_top, 0));
        }
    }

    // 与下边界 (y=resultImage.rows-1) 的交点
    if (std::abs(ny) > 1e-6) {
        float t_bottom = (resultImage.rows - 1 - y0) / ny;
        float x_bottom = x0 + t_bottom * nx;
        if (x_bottom >= 0 && x_bottom < resultImage.cols) {
            intersections.push_back(cv::Point2f(x_bottom, resultImage.rows - 1));
        }
    }

    // 去重并确保有两个不同的交点
    if (intersections.size() >= 2) {
        // 去除重复点
        std::vector<cv::Point2f> unique_intersections;
        for (const auto& point : intersections) {
            bool is_duplicate = false;
            for (const auto& existing : unique_intersections) {
                if (cv::norm(point - existing) < 1.0) {
                    is_duplicate = true;
                    break;
                }
            }
            if (!is_duplicate) {
                unique_intersections.push_back(point);
            }
        }

        if (unique_intersections.size() >= 2) {
            // 绘制直线（红色，线宽3像素）
            cv::line(resultImage, unique_intersections[0], unique_intersections[1], cv::Scalar(0, 0, 255), 1);

            // 绘制端点（绿色圆圈）
            cv::circle(resultImage, unique_intersections[0], 5, cv::Scalar(0, 255, 0), -1);
            cv::circle(resultImage, unique_intersections[1], 5, cv::Scalar(0, 255, 0), -1);

            // 绘制直线上的参考点（蓝色圆圈）
            cv::Point2f referencePoint(x0, y0);
            cv::circle(resultImage, referencePoint, 3, cv::Scalar(255, 0, 0), -1);

            // 添加文字标注
            std::string lineInfo = "Line: vx=" + std::to_string(vx).substr(0, 6) +
                                   ", vy=" + std::to_string(vy).substr(0, 6) +
                                   ", x0=" + std::to_string(x0).substr(0, 6) +
                                   ", y0=" + std::to_string(y0).substr(0, 6);
            cv::putText(resultImage, lineInfo, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

            // 如果提供了保存路径，则保存图像
            if (!savePath.empty()) {
                bool success = cv::imwrite(savePath, resultImage);
            }

            return;
        }
    }

    // 如果无法找到两个边界交点，使用默认方法：在直线上取两个距离较远的点
    float half_diag = std::sqrt(resultImage.cols * resultImage.cols + resultImage.rows * resultImage.rows) / 2.0f;

    cv::Point2f p1(x0 - half_diag * nx, y0 - half_diag * ny);
    cv::Point2f p2(x0 + half_diag * nx, y0 + half_diag * ny);

    // 确保点在图像范围内
    p1.x = std::max(0.0f, std::min(static_cast<float>(resultImage.cols - 1), p1.x));
    p1.y = std::max(0.0f, std::min(static_cast<float>(resultImage.rows - 1), p1.y));
    p2.x = std::max(0.0f, std::min(static_cast<float>(resultImage.cols - 1), p2.x));
    p2.y = std::max(0.0f, std::min(static_cast<float>(resultImage.rows - 1), p2.y));

    // 绘制直线（红色，线宽1像素）
    cv::line(resultImage, p1, p2, cv::Scalar(0, 0, 255), 1);

    // 绘制端点（绿色圆圈）
    cv::circle(resultImage, p1, 5, cv::Scalar(0, 255, 0), -1);
    cv::circle(resultImage, p2, 5, cv::Scalar(0, 255, 0), -1);

    // 绘制直线上的参考点（蓝色圆圈）
    cv::Point2f referencePoint(x0, y0);
    cv::circle(resultImage, referencePoint, 3, cv::Scalar(255, 0, 0), -1);

    // 添加文字标注
    std::string lineInfo = "Line: vx=" + std::to_string(vx).substr(0, 6) +
                           ", vy=" + std::to_string(vy).substr(0, 6) +
                           ", x0=" + std::to_string(x0).substr(0, 6) +
                           ", y0=" + std::to_string(y0).substr(0, 6);
    cv::putText(resultImage, lineInfo, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

    // 如果提供了保存路径，则保存图像
    if (!savePath.empty()) {
        bool success = cv::imwrite(savePath, resultImage);
    }

    // 将结果图像复制回原始图像
    resultImage.copyTo(image);
}

#ifndef CONTOUR_UTILS_H
#define CONTOUR_UTILS_H


#include <opencv2/opencv.hpp>
#include <math.h>

/**
 * @brief 开口方向枚举
 */
enum class OpeningDirection : int {
    UNKNOWN = 0,
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4
};

// 自定义哈希和判等器
struct Point2fHash {
    std::size_t operator()(const cv::Point2f& p) const {
        return std::hash<float>()(p.x) ^ (std::hash<float>()(p.y) << 1);
    }
};

struct Point2fEqual {
    bool operator()(const cv::Point2f& a, const cv::Point2f& b) const {
        const float epsilon = 1e-5f;
        return std::abs(a.x - b.x) < epsilon && std::abs(a.y - b.y) < epsilon;
    }
};

// 工具函数命名空间
namespace ContourUtils {
std::string openingDirectionToString(OpeningDirection direction);
int findPointIndex(const cv::Point2f& point, const std::vector<cv::Point2f>& contour, float tolerance = 1e-5f);
cv::Point2f calculateCentroid(const std::vector<cv::Point2f>& contour);
}

#endif // CONTOUR_UTILS_H

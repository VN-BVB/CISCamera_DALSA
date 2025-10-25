#ifndef CONTOUR_DATA_H
#define CONTOUR_DATA_H

#include <opencv2/opencv.hpp>

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

/**
 * @brief 轮廓数据容器类 - 只负责数据存储
 */
class ContourData {
public:
    ContourData();

    // 数据设置
    void setPixelContour(const std::vector<cv::Point>& contour);
    void setSubpixelContour(const std::vector<cv::Point2f>& contour);
    void clear();
    bool isValid() const;

    // 数据获取
    std::vector<cv::Point> getPixelContour() const { return m_pixelContour; }
    std::vector<cv::Point2f> getSubpixelContour() const { return m_subpixelContour; }
    OpeningDirection getOpeningDirection() const { return m_openingDirection; }
    std::vector<cv::Point2f> getSortedContour() const { return m_sortedSubpixelContour; }
    std::vector<cv::Point2f> getCornerPoints() const { return m_cornerPoints; }
    std::vector<std::vector<cv::Point2f>> getSegmentedContours() const { return m_segmentedSubpixelContours; }
    std::map<int, std::vector<cv::Point2f>> getSortedSegments() const { return m_counterClockwiseContours; }

    // 设置计算后的特征
    void setOpeningDirection(OpeningDirection direction) { m_openingDirection = direction; }
    void setSortedContour(const std::vector<cv::Point2f>& contour) { m_sortedSubpixelContour = contour; }
    void setCornerPoints(const std::vector<cv::Point2f>& points) { m_cornerPoints = points; }
    void setSegmentedContours(const std::vector<std::vector<cv::Point2f>>& contours) {
        m_segmentedSubpixelContours = contours;
    }
    void setSortedSegments(const std::map<int, std::vector<cv::Point2f>>& segments) {
        m_counterClockwiseContours = segments;
    }

private:
    std::vector<cv::Point> m_pixelContour;
    std::vector<cv::Point2f> m_subpixelContour;
    OpeningDirection m_openingDirection;
    std::vector<cv::Point2f> m_sortedSubpixelContour;
    std::vector<cv::Point2f> m_cornerPoints;
    std::vector<std::vector<cv::Point2f>> m_segmentedSubpixelContours;
    std::map<int, std::vector<cv::Point2f>> m_counterClockwiseContours;
};


#endif // CONTOUR_DATA_H

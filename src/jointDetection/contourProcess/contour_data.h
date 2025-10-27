#ifndef CONTOUR_DATA_H
#define CONTOUR_DATA_H

#include "contour_utils.h"

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
    std::vector<cv::Point> m_pixelContour;                                  // 像素级坐标轮廓
    std::vector<cv::Point2f> m_subpixelContour;                             // 亚像素级坐标轮廓
    OpeningDirection m_openingDirection;                                    // 轮廓开口方向
    std::vector<cv::Point2f> m_sortedSubpixelContour;                       // 点相对于重心逆时针排序后的轮廓
    std::vector<cv::Point2f> m_cornerPoints;                                // 轮廓多边形拟合后的角点
    std::vector<std::vector<cv::Point2f>> m_segmentedSubpixelContours;      // 分割后的轮廓
    std::map<int, std::vector<cv::Point2f>> m_counterClockwiseContours;     // 逆时针排序分割后的轮廓（一段一段的）
};


#endif // CONTOUR_DATA_H

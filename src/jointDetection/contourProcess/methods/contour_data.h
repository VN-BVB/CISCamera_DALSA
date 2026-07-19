#ifndef CONTOUR_DATA_H
#define CONTOUR_DATA_H

#include "contour_utils.h"
#include <map>
#include "curve_seg.h"
#include "line_seg.h"

/**
 * @brief 轮廓交点结构体
 */
struct ContourIntersection {
    int id;                           // 交点ID
    cv::Point2f coordinates;          // 交点坐标
    int contourId;                    // 交点所属轮廓ID
    bool isCollision;                 // 该端点是否来自碰撞路径

    ContourIntersection() : id(-1), contourId(-1), isCollision(false) {}
    ContourIntersection(int id, const cv::Point2f& coords, int _contourId = -1, bool _isCollision = false)
        : id(id), coordinates(coords), contourId(_contourId), isCollision(_isCollision) {}
};

/**
 * @brief 轮廓数据容器类 - 扩展支持策略模式
 */
class ContourData {
public:
    ContourData();

    // 数据设置
    void setId(int id) { m_id = id; }
    void setIsCollision(bool isCollision) { m_isCollision = isCollision; }
    void setCenterLine(const cv::Vec4f& line) { m_centerLine = line; }
    void setPixelContour(const std::vector<cv::Point>& contour);
    void setSubpixelContour(const std::vector<cv::Point2f>& contour);
    void clear();
    bool isValid() const;

    // 数据获取
    int getId() const { return m_id; }
    bool isCollision() const { return m_isCollision; }
    cv::Vec4f getCenterLine() const { return m_centerLine; }
    std::vector<cv::Point> getPixelContour() const { return m_pixelContour; }
    std::vector<cv::Point2f> getSubpixelContour() const { return m_subpixelContour; }
    cv::Point2f getOpeningDirection() const { return m_openingDirection; }
    std::vector<cv::Point2f> getSortedContour() const { return m_sortedSubpixelContour; }
    std::vector<cv::Point2f> getCornerPoints() const { return m_cornerPoints; }
    std::vector<std::vector<cv::Point2f>> getSegmentedContours() const { return m_segmentedSubpixelContours; }
    std::map<int, std::vector<cv::Point2f>> getSortedSegments() const { return m_counterClockwiseContours; }
    std::map<int, CurveSeg> getCurveSegments() const { return m_curveSegments; }
    std::map<int, LineSeg> getLineSegments() const { return m_lineSegments; }
    std::vector<ContourIntersection> getIntersections() const { return m_intersections; }
    std::vector<cv::Vec4f> getTangentLines() const { return m_tangentLines; }

    // 设置计算后的特征
    void setOpeningDirection(const cv::Point2f& direction) { m_openingDirection = direction; }
    void setSortedContour(const std::vector<cv::Point2f>& contour) { m_sortedSubpixelContour = contour; }
    void setCornerPoints(const std::vector<cv::Point2f>& points) { m_cornerPoints = points; }
    void setSegmentedContours(const std::vector<std::vector<cv::Point2f>>& contours) { m_segmentedSubpixelContours = contours; }
    void setSortedSegments(const std::map<int, std::vector<cv::Point2f>>& segments) { m_counterClockwiseContours = segments; }

    // 曲线和直线拟合结果存储
    void setCurveSegments(const std::map<int, CurveSeg>& segments) { m_curveSegments = segments; }
    void setLineSegments(const std::map<int, LineSeg>& segments) { m_lineSegments = segments; }
    void setIntersections(const std::vector<ContourIntersection>& intersections) { m_intersections = intersections; }
    void setTangentLines(const std::vector<cv::Vec4f>& lines) { m_tangentLines = lines; }

private:
    int m_id;                                                               // 轮廓ID
    std::vector<cv::Point> m_pixelContour;                                  // 像素级坐标轮廓
    std::vector<cv::Point2f> m_subpixelContour;                             // 亚像素级坐标轮廓
    cv::Point2f m_openingDirection;                                         // 轮廓开口方向单位向量，(0,0) 表示未知
    std::vector<cv::Point2f> m_sortedSubpixelContour;                       // 点相对于中心逆时针排序后的轮廓
    std::vector<cv::Point2f> m_cornerPoints;                                // 轮廓多边形拟合后的角点
    std::vector<std::vector<cv::Point2f>> m_segmentedSubpixelContours;      // 分割后的轮廓
    std::map<int, std::vector<cv::Point2f>> m_counterClockwiseContours;     // 逆时针排序分割后的轮廓（一段一段的）

    // 拟合结果
    std::map<int, CurveSeg> m_curveSegments;                // 拟合曲线
    std::map<int, LineSeg> m_lineSegments;                  // 拟合线段
    std::vector<ContourIntersection> m_intersections;       // 拟合直线或曲线切线的交点，也是拼缝的端点
    std::vector<cv::Vec4f> m_tangentLines;                  // 切线
    bool m_isCollision = false;                              // 本次轮廓是否为碰撞情况下检测得到
    cv::Vec4f m_centerLine{};   // 缝隙中心线 (vx, vy, x0, y0)
};

#endif // CONTOUR_DATA_H

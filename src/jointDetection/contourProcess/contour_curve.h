
#ifndef CONTOUR_CURVE_H
#define CONTOUR_CURVE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <unordered_set>
#include <cmath>

#include "line_seg.h"
#include "contour_segment.h"

/**
 * @brief 开口方向枚举
 */
enum class OpeningDirection : int { // enum class:CXX11的强类型枚举，更安全，使用时必须显示指定作用域
    UNKNOWN = 0,    // 未知方向
    UP = 1,         // 向上开口
    DOWN = 2,       // 向下开口
    LEFT = 3,       // 向左开口
    RIGHT = 4       // 向右开口
};

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
 * @brief 轮廓曲线，存储一条轮廓的所有特征
 */
 class ContourCurve
{
 public:
    ContourCurve();
    void initializePixelContour(const std::vector<cv::Point>& contour);
    void initializeSubpixelContour(const std::vector<cv::Point2f>& contour);
    void clear();
    bool isValid() const;
    std::string getSummary() const;
    OpeningDirection getOpeningDirection() const {return m_openingDirection;}
    std::vector<cv::Point> getPixelContour() const  {return m_pixelContour;}
    std::vector<cv::Point2f> getSubpixelContours() const {return m_subpixelContour;}
    std::vector<std::vector<cv::Point>> getSegmentedPixelContours() const   {return m_segmentedPixelContours;}
    std::vector<cv::Point> getDeduplicatedPixelContour() const {return m_deduplicatedPixelContour;}
    std::vector<LineSeg> getLineSegments() const {return m_lineSegments;}


private:
    void calculateBasicFeatures();
    std::string openingDirectionToString(OpeningDirection direction) const;

    template<typename PointType>
    std::vector<PointType> removeDuplicateContourPointsImpl(const std::vector<PointType>& contour) const;
    std::vector<cv::Point> removeDuplicateContourPoints(const std::vector<cv::Point> &contour);
    std::vector<cv::Point2f> removeDuplicateContourPoints(const std::vector<cv::Point2f>& contour);

    template<typename PointType>
    OpeningDirection calculateOpeningDirectionImpl(const std::vector<PointType>& contour) const;
    OpeningDirection calculateOpeningDirection();

    void calculateLines();
    cv::Point2f calculateLineIntersection(const cv::Vec4f &line1, const cv::Vec4f &line2);
    void calculateCornerPoints();

    void segment();

private:
    // 基本轮廓信息
    std::vector<cv::Point> m_pixelContour;            // 轮廓像素级点集
    std::vector<cv::Point2f> m_subpixelContour;       // 轮廓亚像素级点集
    OpeningDirection m_openingDirection;              // 开口方向
    std::vector<cv::Point> m_deduplicatedPixelContour;        // 去重后的像素级点集：由于扫描C字型轮廓时，算法会来回扫描成闭合轮廓，因此会有很多重复点
    std::vector<cv::Point2f> m_deduplicatedSubpixelContour;   // 去重后的亚像素级点集

    // 轮廓分割相关特征
    std::vector<std::vector<cv::Point>> m_segmentedPixelContours;  // 分割后的轮廓段-像素级
    std::vector<std::vector<cv::Point2f>> m_segmentedSubpixelContours; // 分割后的轮廓段-亚像素级
    std::vector<cv::Point2f> m_cornerPoints;          // 角点位置

    // 直线拟合相关特征
    std::vector<LineSeg> m_lineSegments;            // 分割后的各线段拟合特征

    // 几何特征
    cv::Rect m_boundingRect;                           // 轮廓外接矩形
    double m_area;                                     // 轮廓面积
    double m_perimeter;                                // 轮廓周长
    double m_aspectRatio;                              // 长宽比
    cv::Point m_centroid;                              // 轮廓质心

    // 多边形逼近特征
    std::vector<cv::Point> m_approxPolygon;            // 多边形逼近结果
    double m_approxError;                              // 逼近误差
};

/**
* @brief 模板removeDuplicateContourPoints 轮廓点去重
* @param contours 输入轮廓点集
* @return 去重后的轮廓点集
*/
template<typename PointType>
std::vector<PointType> ContourCurve::removeDuplicateContourPointsImpl(const std::vector<PointType>& contour) const
{
    // 定义点比较结构体
    struct PointCompare {
        bool operator()(const PointType& a, const PointType& b) const {
            if (a.x == b.x) return a.y < b.y;
            return a.x < b.x;
        }
    };
    using PointSet = std::set<PointType, PointCompare>;

    // 用于记录已出现点的集合
    PointSet seen;
    std::vector<PointType> unique_points;

    for (const auto& point : contour) {
        // 尝试将点插入集合。如果插入成功，说明是第一次出现。
        if (seen.insert(point).second) {
            unique_points.push_back(point);
        }
    }

    return unique_points;
}

/**
* @brief 模板化的开口方向计算函数
* @tparam PointType 点类型（cv::Point 或 cv::Point2f）
* @param contour 轮廓点集
* @return 开口方向
*/
template<typename PointType>
OpeningDirection ContourCurve::calculateOpeningDirectionImpl(const std::vector<PointType>& contour) const
{
    if (contour.empty()) {
        return OpeningDirection::UNKNOWN;
    }

    // 计算轮廓点的x、y坐标平均值
    float sumX = 0.0f, sumY = 0.0f;
    for (const auto& point : contour) {
        sumX += point.x;
        sumY += point.y;
    }
    float avgX = sumX / contour.size();
    float avgY = sumY / contour.size();

    // 检查四个方向是否存在轮廓点
    bool hasUp = false, hasDown = false, hasLeft = false, hasRight = false;

    for (const auto& point : contour) {
        // 检查正上方（x坐标相同，y坐标更小）
        if (std::abs(point.x - avgX) < 1e-5 && point.y < avgY) {
            hasUp = true;
        }
        // 检查正下方（x坐标相同，y坐标更大）
        if (std::abs(point.x - avgX) < 1e-5 && point.y > avgY) {
            hasDown = true;
        }
        // 检查正左方（y坐标相同，x坐标更小）
        if (std::abs(point.y - avgY) < 1e-5 && point.x < avgX) {
            hasLeft = true;
        }
        // 检查正右方（y坐标相同，x坐标更大）
        if (std::abs(point.y - avgY) < 1e-5 && point.x > avgX) {
            hasRight = true;
        }
    }

    // 判断开口方向
    if (!hasUp) return OpeningDirection::UP;
    if (!hasDown) return OpeningDirection::DOWN;
    if (!hasLeft) return OpeningDirection::LEFT;
    if (!hasRight) return OpeningDirection::RIGHT;

    return OpeningDirection::UNKNOWN;
}

#endif // CONTOUR_CURVE_H

#ifndef CONTOUR_PROCESSOR_H
#define CONTOUR_PROCESSOR_H

#include <opencv2/opencv.hpp>
#include "curve_seg.h"
#include "line_seg.h"

/**
 * @brief 主轮廓处理类 - 协调各个组件工作
 */
class ContourProcessor {
public:
    ContourProcessor();

    void processContour(const std::vector<cv::Point2f>& contour);
    std::string getSummary() const;

    // 结果获取
    std::vector<cv::Point2f> getSortedContour() const { return m_data.getSortedContour(); }
    std::vector<cv::Point2f> getCornerPoints() const { return m_data.getCornerPoints(); }
    std::map<int, LineSeg> getLineSegments() const { return m_lineSegments; }
    std::map<int, CurveSeg> getCurveSegments() const { return m_curveSegments; }
    std::vector<cv::Point2f> getEndPoints() const { return m_endPoints; }
    std::vector<cv::Vec4f> getLines() const { return m_lines; }

private:
    ContourData m_data;
    std::map<int, LineSeg> m_lineSegments;
    std::map<int, CurveSeg> m_curveSegments;
    std::vector<cv::Point2f> m_endPoints;
    std::vector<cv::Vec4f> m_lines;
};

// 工具函数命名空间
namespace ContourUtils {
std::string openingDirectionToString(OpeningDirection direction);
int findPointIndex(const cv::Point2f& point, const std::vector<cv::Point2f>& contour, float tolerance = 1e-5f);
cv::Point2f calculateCentroid(const std::vector<cv::Point2f>& contour);



#endif // CONTOUR_PROCESSOR_H

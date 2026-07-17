#ifndef WORKPIECE_BOUNDING_BOX_H
#define WORKPIECE_BOUNDING_BOX_H

#include "contour_bounding_box.h"

/**
 * @brief The WorkpieceBoundingBox class
 */
class WorkpieceBoundingBox
{
public:
    WorkpieceBoundingBox();

    bool addContourBoundingBox(std::shared_ptr<ContourBoundingBox> cbb);
    void updateOuterBoundingBox();
    bool isLegal(std::shared_ptr<ContourBoundingBox> candidateCbb) const;
    std::vector<int> getContourIds() const;
    cv::RotatedRect getouterBoundingBox() const {return m_outerBoundingBox;}
    std::vector<std::pair<cv::Point2f, cv::Point2f>> getCenterPointConnections() const {return m_centerPointConnections;}


private:
    cv::RotatedRect generateOuterBoundingBox(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs);
    void updateCenterPointConnections();
    void updateWorkpieceBoundary();

private:
    std::vector<std::shared_ptr<ContourBoundingBox>> m_cbbs;     // 属于该工件的轮廓边界框集合
    cv::RotatedRect m_outerBoundingBox;                          // 组合后的最小外接旋转矩形
    std::vector<std::pair<cv::Point2f, cv::Point2f>> m_centerPointConnections; // 任意两轮廓边界框中心点连接形成的线段集合
    std::vector<std::pair<cv::Point2f, cv::Point2f>> m_workpieceBoundaryEdges;  // 工件边界边集（依次连接各 CBB 端点构成的不自交闭合多边形）
    bool m_workpieceBoundaryValid = false;                                       // 工件边界是否成功构造（不含自交）
};

#endif // WORKPIECE_BOUNDING_BOX_H

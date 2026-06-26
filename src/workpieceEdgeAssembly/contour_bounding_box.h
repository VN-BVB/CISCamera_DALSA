#ifndef CONTOUR_BOUNDING_BOX_H
#define CONTOUR_BOUNDING_BOX_H

#include "src/jointDetection/joint_seam.h"
#include "src/jointDetection/contourProcess/methods/contour_utils.h"

/**
 * @brief 轮廓包围盒，接收一条拼缝一边轮廓信息，然后得出其包围盒
 */
class ContourBoundingBox
{
public:
    ContourBoundingBox();
    void initContourData(int id, const ContourData& contourData);
    int getId() const {return m_id;}
    int getOppositeId() const {return m_oppositeId;}
    void setIsPaired(bool paired) {m_isPaired = paired;}
    bool getIsPaired() const { return m_isPaired;}
    cv::RotatedRect getBoundingRect() const {return m_boundingRect;}
    cv::Point2f getCenterPoint() const {return m_centerPoint;}
    OpeningDirection getOpeningDirection() const {return m_openingDirection;}
    std::vector<cv::Point2f> getContourEndpoints() const;

private:
    cv::RotatedRect generateBoundingBox(const ContourData& contourData);
    int setOppositeTo(int id);

private:
    ContourData m_contourData;              // 轮廓数据
    cv::RotatedRect m_boundingRect;         // 轮廓包围框
    cv::Point2f m_topLeft;
    cv::Point2f m_topRight;
    cv::Point2f m_bottomRight;
    cv::Point2f m_bottomLeft;
    cv::Point2f m_centerPoint;              // 最小外接旋转矩形中心点
    int m_id;                               // 轮廓最小包围框id
    int m_oppositeId;                       // 与其相背轮廓的id（轮廓id从0开始，偶数与其id+1相背，奇数与其id-1相背）
    bool m_isPaired = false;                // 是否已被分配到工件
    OpeningDirection m_openingDirection;    // 轮廓开口方向
};

#endif // CONTOUR_BOUNDING_BOX_H

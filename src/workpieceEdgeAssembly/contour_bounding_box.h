#ifndef CONTOUR_BOUNDING_BOX_H
#define CONTOUR_BOUNDING_BOX_H

#include "src/jointDetection/contourProcess/joint_seam.h"

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
    cv::Rect2f getBoundingRect() const {return m_boundingRect;}


private:
    cv::Rect2f generateBoundingBox(const ContourData& contourData);
    int setOppositeTo(int id);

private:
    ContourData m_contourData;         // 轮廓数据
    cv::Rect2f m_boundingRect;                      // 轮廓包围框
    cv::Point2f m_topLeft;
    cv::Point2f m_topRight;
    cv::Point2f m_bottomRight;
    cv::Point2f m_bottomLeft;
    int m_id;                                       // 轮廓最小包围框id
    int m_oppositeId;                               // 与其相背轮廓的id（轮廓id从0开始，偶数与其id+1相背，奇数与其id-1相背）
    bool m_isPaired = false;                                // 是否已被分配到工件

};

#endif // CONTOUR_BOUNDING_BOX_H

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

    void addContourBoundingBox(std::shared_ptr<ContourBoundingBox> cbb);
    void updateOuterBoundingBox();
    bool isLegal(std::shared_ptr<ContourBoundingBox> candidateCbb) const;
    std::vector<int> getContourIds() const;
    cv::RotatedRect getouterBoundingBox() const {return m_outerBoundingBox;}

private:
    cv::RotatedRect generateOuterBoundingBox(const std::vector<std::shared_ptr<ContourBoundingBox>> cbbs);

private:
    std::vector<std::shared_ptr<ContourBoundingBox>> m_cbbs;     // 属于该工件的轮廓边界框集合
    cv::RotatedRect m_outerBoundingBox;                          // 组合后的最小外接旋转矩形
};

#endif // WORKPIECE_BOUNDING_BOX_H

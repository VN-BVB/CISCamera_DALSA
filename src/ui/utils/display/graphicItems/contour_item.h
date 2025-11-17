#ifndef CONTOUR_ITEM_H
#define CONTOUR_ITEM_H

#include <opencv2/core/core.hpp>

#include "graphic_item_component.h"

// 轮廓组件，组合模式中的leaf
class ContourItem : public GraphicsItemComponent
{
public:
    enum ContourType {
        subpixelContour,
        pixelContour
    };


    ContourItem(const std::vector<cv::Point2f>& contour, ContourType type);
    ContourItem(const std::vector<cv::Point> &contour, ContourType type);

    // 仅实现必要的getGraphicsItems方法
    QList<QGraphicsItem*> getGraphicsItems() const override;

private:
    QList<QGraphicsItem*> m_graphicsItems;
    ContourType m_type;
};

#endif // CONTOUR_ITEM_H

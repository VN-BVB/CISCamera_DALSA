#ifndef POINT_ITEM_H
#define POINT_ITEM_H

#include <opencv2/core/core.hpp>
#include <QColor>

#include "graphic_item_component.h"

class PointItem : public GraphicsItemComponent
{
public:
    PointItem(const std::vector<cv::Point2f>& points,
              const QColor& color = Qt::red,
              double size = 10,
              double zValue = 15.0);

    // 仅实现必要的getGraphicsItems方法
    QList<QGraphicsItem*> getGraphicsItems() const override;

private:
    QList<QGraphicsItem*> m_graphicsItems;
};

#endif // POINT_ITEM_H

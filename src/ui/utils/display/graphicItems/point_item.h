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

    QList<QGraphicsItem*> getGraphicsItems() const override;

private:
    double m_size; // 点的大小
    std::vector<cv::Point2f> m_points;
};

#endif // POINT_ITEM_H

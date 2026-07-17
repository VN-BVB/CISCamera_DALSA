#ifndef LINE_ITEM_H
#define LINE_ITEM_H

#include <opencv2/core/core.hpp>

#include "graphic_item_component.h"

class LineItem : public GraphicsItemComponent
{
public:
    LineItem(const cv::Vec4f& line,
             double lineWidth,
             double length = 1.0,
             const QColor& color = Qt::blue);

    QList<QGraphicsItem*> getGraphicsItems() const override;

private:
    double m_length;  // 线段长度
    cv::Vec4f m_line; // 直线参数
};

#endif // LINE_ITEM_H

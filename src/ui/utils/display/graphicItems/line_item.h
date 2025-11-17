#ifndef LINE_ITEM_H
#define LINE_ITEM_H

#include <opencv2/core/core.hpp>

#include "graphic_item_component.h"

class LineItem : public GraphicsItemComponent
{
public:
    LineItem(const cv::Vec4f& line,
             double lenth = 1.0,
             const QColor = Qt::blue);

    void addToScene(QGraphicsScene* scene) override;
    void removeFromScene(QGraphicsScene* scene) override;
    QList<QGraphicsItem*> getGraphicsItems() const override;

private:
    QList<QGraphicsItem*> m_graphicsItems;
};

#endif // LINE_ITEM_H

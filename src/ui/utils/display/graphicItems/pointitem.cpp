#include "point_item.h"
#include <QGraphicsEllipseItem>

PointItem::PointItem(const std::vector<cv::Point2f>& points, const QColor& color, double size, double zValue)
{
    // 遍历所有点，为每个点创建图形图元
    for (const auto& point : points) {
        // 创建圆形标记点
        QGraphicsEllipseItem *pointItem = new QGraphicsEllipseItem(point.x - size/2, point.y - size/2, size, size);
        pointItem->setBrush(QBrush(color));
        pointItem->setPen(QPen(Qt::NoPen));
        pointItem->setZValue(zValue);

        m_graphicsItems.append(pointItem);
    }
}

QList<QGraphicsItem*> PointItem::getGraphicsItems() const
{
    return m_graphicsItems;
}

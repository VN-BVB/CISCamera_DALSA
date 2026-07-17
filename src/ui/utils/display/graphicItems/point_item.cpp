#include "point_item.h"
#include <QGraphicsEllipseItem>

PointItem::PointItem(const std::vector<cv::Point2f>& points,
                     const QColor& color,
                     double size,
                     double zValue)
    : GraphicsItemComponent(color, 0, Qt::NoPen, zValue), // 线宽设为0，线型为NoPen
    m_size(size),
    m_points(points)
{
    // 遍历所有点，为每个点创建图形图元
    for (const auto& point : points) {
        // 创建圆形标记点
        QGraphicsEllipseItem *pointItem = new QGraphicsEllipseItem(point.x - m_size/2, point.y - m_size/2, m_size, m_size);
        pointItem->setBrush(QBrush(color));
        pointItem->setPen(QPen(Qt::NoPen));

        m_graphicsItems.append(pointItem);
    }

    // 应用属性
    applyProperties();
}

QList<QGraphicsItem*> PointItem::getGraphicsItems() const
{
    return m_graphicsItems;
}


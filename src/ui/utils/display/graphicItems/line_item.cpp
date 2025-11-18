#include "line_item.h"
#include <QPainterPath>
#include <QPen>
#include <QGraphicsPathItem>

LineItem::LineItem(const cv::Vec4f& line,
                   double lineWidth,
                   double length,
                   const QColor& color)
    : GraphicsItemComponent(color, lineWidth, Qt::SolidLine, 10.0),
    m_line(line),
    m_length(length)
{
    double vx = line[0];
    double vy = line[1];
    double x0 = line[2];
    double y0 = line[3];

    // 计算直线端点
    double x1 = x0 - length * vx;
    double y1 = y0 - length * vy;
    double x2 = x0 + length * vx;
    double y2 = y0 + length * vy;

    QPainterPath path;
    path.moveTo(x1, y1);
    path.lineTo(x2, y2);

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);

    // 添加到基类的容器中
    m_graphicsItems.append(pathItem);

    applyProperties();
}

QList<QGraphicsItem*> LineItem::getGraphicsItems() const
{
    // 直接返回基类中的容器
    return m_graphicsItems;
}

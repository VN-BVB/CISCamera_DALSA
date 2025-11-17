#include "line_item.h"
#include <QPainterPath>
#include <QPen>
#include <QGraphicsPathItem>

LineItem::LineItem(const cv::Vec4f& line, double length, const QColor color)
{
    // 实现直线绘制逻辑
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
    QPen pen(color);
    pen.setWidthF(0.1);
    pathItem->setPen(pen);
    pathItem->setZValue(10);

    m_graphicsItems.append(pathItem);
}

void LineItem::addToScene(QGraphicsScene* scene)
{
    for (auto item : m_graphicsItems) {
        scene->addItem(item);
    }
}

void LineItem::removeFromScene(QGraphicsScene* scene)
{
    for (auto item : m_graphicsItems) {
        scene->removeItem(item);
        delete item;
    }
    m_graphicsItems.clear();
}

QList<QGraphicsItem*> LineItem::getGraphicsItems() const
{
    return m_graphicsItems;
}

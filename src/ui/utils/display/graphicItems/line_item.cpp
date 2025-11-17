#include "line_item.h"
#include <QPainterPath>
#include <QPen>
#include <QGraphicsPathItem>

LineItem::LineItem(const cv::Vec4f& line, double length, const QColor color)
    : GraphicsItemComponent() // 调用基类构造函数
{
    // 保存特有属性
    m_line = line;
    m_length = length;

    // 使用基类方法设置共同属性
    setColor(color);
    setLineWidth(0.1); // 原代码中设置的线宽
    setZValue(10);    // 原代码中设置的z值

    // 创建图形项并添加到基类的m_graphicsItems中
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

    applyProperties();

    // 添加到基类的容器中
    m_graphicsItems.append(pathItem);
}

QList<QGraphicsItem*> LineItem::getGraphicsItems() const
{
    // 直接返回基类中的容器
    return m_graphicsItems;
}

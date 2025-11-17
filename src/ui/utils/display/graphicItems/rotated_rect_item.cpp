#include "rotated_rect_item.h"
#include <QGraphicsPathItem>
#include <QPainterPath>

RotatedRectItem::RotatedRectItem(const std::vector<cv::RotatedRect>& rotatedRects,
                                 const QColor& color,
                                 double lineWidth,
                                 double zValue)
    : m_rotatedRects(rotatedRects),
    m_color(color),
    m_lineWidth(lineWidth),
    m_zValue(zValue)
{
}

RotatedRectItem::~RotatedRectItem()
{
    // 清理创建的图形项
    for (auto item : m_items) {
        delete item;
    }
    m_items.clear();
}

QList<QGraphicsItem*> RotatedRectItem::getGraphicsItems() const
{
    if (m_items.isEmpty()) {
        // 创建画笔
        QPen pen(m_color);
        pen.setWidthF(m_lineWidth);
        pen.setStyle(Qt::SolidLine);

        // 遍历所有旋转矩形
        for (const auto& rotatedRect : m_rotatedRects) {
            // 获取旋转矩形的四个角点
            cv::Point2f vertices[4];
            rotatedRect.points(vertices);

            // 创建QPainterPath来绘制旋转矩形
            QPainterPath path;
            path.moveTo(vertices[0].x, vertices[0].y);

            // 连接四个角点形成闭合矩形
            for (int i = 1; i < 4; ++i) {
                path.lineTo(vertices[i].x, vertices[i].y);
            }
            path.closeSubpath();  // 闭合路径

            // 创建路径图元
            QGraphicsPathItem *rectItem = new QGraphicsPathItem(path);
            rectItem->setPen(pen);
            rectItem->setZValue(m_zValue);  // 设置Z值，确保显示在图像上方

            m_items.append(rectItem);
        }
    }

    return m_items;
}

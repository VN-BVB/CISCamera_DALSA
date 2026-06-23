#include "rotated_rect_item.h"
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>

RotatedRectItem::RotatedRectItem(const std::vector<cv::RotatedRect>& rotatedRects,
                                 const QColor& color,
                                 double lineWidth,
                                 double zValue)
    : GraphicsItemComponent(color, lineWidth, Qt::SolidLine, zValue), // 调用基类构造函数
    m_rotatedRects(rotatedRects)
{
}

RotatedRectItem::~RotatedRectItem()
{
    // 清理创建的图形项
    for (auto item : m_graphicsItems) {
        delete item;
    }
    m_graphicsItems.clear();
}

QList<QGraphicsItem*> RotatedRectItem::getGraphicsItems() const
{
    if (m_graphicsItems.isEmpty()) {
        // 创建画笔，使用基类提供的属性
        QPen pen(getColor());
        pen.setWidthF(getLineWidth());
        pen.setStyle(getLineStyle());

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
            rectItem->setZValue(getZValue());  // 设置Z值，确保显示在图像上方

            m_graphicsItems.append(rectItem);
        }
    }

    return m_graphicsItems;
}

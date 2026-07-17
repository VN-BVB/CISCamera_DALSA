#include "contour_item.h"
#include <opencv2/opencv.hpp>
#include <QPainterPath>
#include <QPen>
#include <QGraphicsPathItem>

ContourItem::ContourItem(const std::vector<cv::Point2f>& contour,
                         ContourType type,
                         const QColor& color,
                         double lineWidth,
                         Qt::PenStyle lineStyle,
                         double zValue)
    : GraphicsItemComponent(color, lineWidth, lineStyle, zValue),
    m_type(type)
{
    // 实现轮廓绘制逻辑
    if (contour.empty()) return;

    QPainterPath path;
    path.moveTo(contour[0].x, contour[0].y);

    for (size_t i = 1; i < contour.size(); ++i) {
        path.lineTo(contour[i].x, contour[i].y);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    m_graphicsItems.append(pathItem);

    applyProperties();
}

ContourItem::ContourItem(const std::vector<cv::Point>& contour,
                         ContourType type,
                         const QColor& color,
                         double lineWidth,
                         Qt::PenStyle lineStyle,
                         double zValue)
    : GraphicsItemComponent(color, lineWidth, lineStyle, zValue),
    m_type(type)
{
    // 实现像素轮廓绘制逻辑
    if (contour.empty()) return;

    QPainterPath path;
    path.moveTo(contour[0].x, contour[0].y);

    for (size_t i = 1; i < contour.size(); ++i) {
        path.lineTo(contour[i].x, contour[i].y);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    m_graphicsItems.append(pathItem);

    applyProperties();
}

QList<QGraphicsItem*> ContourItem::getGraphicsItems() const
{
    return m_graphicsItems;
}

void ContourItem::applyProperties()
{
    // 先调用基类的applyProperties应用基本属性
    // GraphicsItemComponent::applyProperties();

    for (auto& item : m_graphicsItems) {
        if (QGraphicsPathItem *pathItem = dynamic_cast<QGraphicsPathItem*>(item)) {
            QPen pen;

            if (getColor() != Qt::transparent) {
                pen.setColor(getColor());
            } else {
                pen.setColor(m_type == subpixelContour ? Qt::red : Qt::green);
            }

            if (getLineWidth() >= 0) {
                pen.setWidthF(getLineWidth());
            } else {
                pen.setWidthF(m_type == subpixelContour ? 0.2 : 0.1);
            }

            pen.setStyle(getLineStyle());

            pathItem->setPen(pen);
            pathItem->setZValue(getZValue());
        }
    }
}

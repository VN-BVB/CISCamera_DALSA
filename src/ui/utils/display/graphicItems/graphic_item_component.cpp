#include "graphic_item_component.h"
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPen>

GraphicsItemComponent::GraphicsItemComponent(const QColor& color,
                                             double lineWidth,
                                             Qt::PenStyle lineStyle,
                                             double zValue)
    : m_color(color),
    m_lineWidth(lineWidth),
    m_lineStyle(lineStyle),
    m_zValue(zValue)
{}

void GraphicsItemComponent::setColor(const QColor& color)
{
    m_color = color;
    applyProperties();
}

QColor GraphicsItemComponent::getColor() const
{
    return m_color;
}

void GraphicsItemComponent::setLineWidth(double width)
{
    m_lineWidth = width;
    applyProperties();
}

double GraphicsItemComponent::getLineWidth() const
{
    return m_lineWidth;
}

void GraphicsItemComponent::setLineStyle(Qt::PenStyle style)
{
    m_lineStyle = style;
    applyProperties();
}

Qt::PenStyle GraphicsItemComponent::getLineStyle() const
{
    return m_lineStyle;
}

void GraphicsItemComponent::setZValue(double zValue)
{
    m_zValue = zValue;
    applyProperties();
}

double GraphicsItemComponent::getZValue() const
{
    return m_zValue;
}

void GraphicsItemComponent::applyProperties()
{
    for (auto item : m_graphicsItems) {
        if (item) {
            // 设置Z值
            item->setZValue(m_zValue);

            // 应用颜色、线宽和线型
            if (auto penItem = dynamic_cast<QAbstractGraphicsShapeItem*>(item)) {
                QPen pen = penItem->pen();
                pen.setColor(m_color);
                pen.setWidthF(m_lineWidth);
                pen.setStyle(m_lineStyle);
                penItem->setPen(pen);
            }
        }
    }
}

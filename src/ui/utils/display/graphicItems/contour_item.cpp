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
    : m_type(type),
    m_color(color),
    m_lineWidth(lineWidth),
    m_lineStyle(lineStyle),
    m_zValue(zValue)
{
    // 实现轮廓绘制逻辑
    if (contour.empty()) return;

    QPainterPath path;
    path.moveTo(contour[0].x, contour[0].y);

    for (size_t i = 1; i < contour.size(); ++i) {
        path.lineTo(contour[i].x, contour[i].y);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);

    // 设置默认值（如果未指定）
    QColor finalColor = (color == Qt::transparent) ?
                            (type == subpixelContour ? Qt::red : Qt::green) : color;
    double finalWidth = (lineWidth < 0) ?
                            (type == subpixelContour ? 0.2 : 0.1) : lineWidth;
    Qt::PenStyle finalStyle = (lineStyle == Qt::SolidLine) ? lineStyle :
                                  (type == subpixelContour ? Qt::DashLine : Qt::SolidLine);

    QPen pen(finalColor);
    pen.setWidthF(finalWidth);
    pen.setStyle(finalStyle);
    pathItem->setPen(pen);
    pathItem->setZValue(m_zValue);

    m_graphicsItems.append(pathItem);
}

ContourItem::ContourItem(const std::vector<cv::Point>& contour,
                         ContourType type,
                         const QColor& color,
                         double lineWidth,
                         Qt::PenStyle lineStyle,
                         double zValue)
    : m_type(type),
    m_color(color),
    m_lineWidth(lineWidth),
    m_lineStyle(lineStyle),
    m_zValue(zValue)
{
    // 实现像素轮廓绘制逻辑
    if (contour.empty()) return;

    QPainterPath path;
    path.moveTo(contour[0].x, contour[0].y);

    for (size_t i = 1; i < contour.size(); ++i) {
        path.lineTo(contour[i].x, contour[i].y);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);

    // 设置默认值（如果未指定）
    QColor finalColor = (color == Qt::transparent) ?
                            (type == subpixelContour ? Qt::red : Qt::green) : color;
    double finalWidth = (lineWidth < 0) ?
                            (type == subpixelContour ? 0.2 : 0.1) : lineWidth;
    Qt::PenStyle finalStyle = (lineStyle == Qt::SolidLine) ? lineStyle :
                                  (type == subpixelContour ? Qt::DashLine : Qt::SolidLine);

    QPen pen(finalColor);
    pen.setWidthF(finalWidth);
    pen.setStyle(finalStyle);
    pathItem->setPen(pen);
    pathItem->setZValue(m_zValue);

    m_graphicsItems.append(pathItem);
}

QList<QGraphicsItem*> ContourItem::getGraphicsItems() const
{
    return m_graphicsItems;
}

// 设置颜色
void ContourItem::setColor(const QColor& color)
{
    m_color = color;
    applyProperties();
}

// 设置线宽
void ContourItem::setLineWidth(double width)
{
    m_lineWidth = width;
    applyProperties();
}

// 设置线形
void ContourItem::setLineStyle(Qt::PenStyle style)
{
    m_lineStyle = style;
    applyProperties();
}

// 设置Z值
void ContourItem::setZValue(double zValue)
{
    m_zValue = zValue;
    for (auto& item : m_graphicsItems) {
        item->setZValue(zValue);
    }
}

// 应用属性到所有图形项
void ContourItem::applyProperties()
{
    for (auto& item : m_graphicsItems) {
        if (QGraphicsPathItem *pathItem = dynamic_cast<QGraphicsPathItem*>(item)) {
            QPen pen = pathItem->pen();

            // 应用颜色（如果不为透明）
            if (m_color != Qt::transparent) {
                pen.setColor(m_color);
            } else {
                // 如果颜色为透明，使用默认颜色
                pen.setColor(m_type == subpixelContour ? Qt::red : Qt::green);
            }

            // 应用线宽（如果有效）
            if (m_lineWidth >= 0) {
                pen.setWidthF(m_lineWidth);
            } else {
                // 如果线宽无效，使用默认线宽
                pen.setWidthF(m_type == subpixelContour ? 0.2 : 0.1);
            }

            // 应用线形
            pen.setStyle(m_lineStyle);

            pathItem->setPen(pen);
        }
    }
}

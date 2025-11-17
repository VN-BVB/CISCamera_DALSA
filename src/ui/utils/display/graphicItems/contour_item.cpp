#include "contour_item.h"
#include <opencv2/opencv.hpp>
#include <QPainterPath>
#include <QPen>
#include <QGraphicsPathItem>

ContourItem::ContourItem(const std::vector<cv::Point2f>& contour, ContourType type)
    : m_type(type)
{
    // 实现轮廓绘制逻辑
    if (contour.empty()) return;

    QPainterPath path;
    path.moveTo(contour[0].x, contour[0].y);

    for (size_t i = 1; i < contour.size(); ++i) {
        path.lineTo(contour[i].x, contour[i].y);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    QPen pen(m_type == subpixelContour ? Qt::red : Qt::green);
    pen.setWidthF(m_type == subpixelContour ? 0.2 : 0.1);
    pen.setStyle(m_type == subpixelContour ? Qt::DashLine : Qt::SolidLine);
    pathItem->setPen(pen);
    pathItem->setZValue(10);

    m_graphicsItems.append(pathItem);
}

ContourItem::ContourItem(const std::vector<cv::Point>& contour, ContourType type)
    : m_type(type)
{
    // 实现像素轮廓绘制逻辑
    if (contour.empty()) return;

    QPainterPath path;
    path.moveTo(contour[0].x, contour[0].y);

    for (size_t i = 1; i < contour.size(); ++i) {
        path.lineTo(contour[i].x, contour[i].y);
    }

    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
    QPen pen(m_type == subpixelContour ? Qt::red : Qt::green);
    pen.setWidthF(m_type == subpixelContour ? 0.2 : 0.1);
    pen.setStyle(m_type == subpixelContour ? Qt::DashLine : Qt::SolidLine);
    pathItem->setPen(pen);
    pathItem->setZValue(10);

    m_graphicsItems.append(pathItem);
}

QList<QGraphicsItem*> ContourItem::getGraphicsItems() const
{
    return m_graphicsItems;
}

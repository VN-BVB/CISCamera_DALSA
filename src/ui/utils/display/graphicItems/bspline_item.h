#ifndef BSPLINE_ITEM_H
#define BSPLINE_ITEM_H

#include <QColor>
#include <QPainterPath>
#include <opencv2/core/core.hpp>
#include "tinysplinecxx.h"
#include "graphic_item_component.h"

class BSplineItem : public GraphicsItemComponent
{
public:
    // 从tinyspline::BSpline对象创建
    BSplineItem(const tinyspline::BSpline& spline,
                const QColor& color = QColor(255, 0, 255),
                double lineWidth = 0.1,
                Qt::PenStyle lineStyle = Qt::SolidLine,
                double zValue = 12.0);

    // 从控制点创建
    BSplineItem(const std::vector<cv::Point2f>& controlPoints,
                const QColor& color = QColor(255, 0, 255),
                double lineWidth = 0.1,
                Qt::PenStyle lineStyle = Qt::SolidLine,
                double zValue = 12.0);

    QList<QGraphicsItem*> getGraphicsItems() const override;

private:
    // 从tinyspline创建路径的辅助方法
    QPainterPath createPathFromBSpline(const tinyspline::BSpline& spline);
    // 从控制点创建路径的辅助方法
    QPainterPath createPathFromControlPoints(const std::vector<cv::Point2f>& controlPoints);
};

#endif // BSPLINE_ITEM_H

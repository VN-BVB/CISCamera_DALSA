#include "bspline_item.h"
#include <QGraphicsPathItem>
#include <QPen>

BSplineItem::BSplineItem(const tinyspline::BSpline& spline,
                         const QColor& color,
                         double lineWidth,
                         Qt::PenStyle lineStyle,
                         double zValue)
    : GraphicsItemComponent(color,lineWidth,lineStyle,zValue)
{
    // 创建路径
    QPainterPath path = createPathFromBSpline(spline);

    // 创建路径图元
    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);

    // 添加到基类的容器中
    m_graphicsItems.append(pathItem);
    applyProperties();
}

BSplineItem::BSplineItem(const std::vector<cv::Point2f>& controlPoints,
                         const QColor& color,
                         double lineWidth,
                         Qt::PenStyle lineStyle,
                         double zValue)
    : GraphicsItemComponent(color,lineWidth,lineStyle,zValue)
{
    // 创建路径
    QPainterPath path = createPathFromControlPoints(controlPoints);

    // 创建路径图元
    QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);

    // 添加到基类的容器中
    m_graphicsItems.append(pathItem);
    applyProperties();
}

QList<QGraphicsItem*> BSplineItem::getGraphicsItems() const
{
    // 直接返回基类中的容器
    return m_graphicsItems;
}

// 以下辅助方法保持不变
QPainterPath BSplineItem::createPathFromBSpline(const tinyspline::BSpline& spline)
{
    QPainterPath path;

    // 获取控制点和维度信息
    std::vector<tinyspline::real> controlPoints = spline.controlPoints();
    size_t dimension = spline.dimension();

    // 检查维度是否为2（二维曲线）
    if (dimension != 2 || controlPoints.size() < 2) {
        return path;
    }

    // 使用tinyspline采样功能获取曲线上的点
    std::vector<tinyspline::real> sampledPoints = spline.sample(1000); // 采样1000个点

    if (sampledPoints.size() >= 2) {
        // 移动到第一个点
        path.moveTo(sampledPoints[0], sampledPoints[1]);

        // 连接所有采样点
        for (size_t i = 2; i < sampledPoints.size(); i += 2) {
            if (i + 1 < sampledPoints.size()) {
                path.lineTo(sampledPoints[i], sampledPoints[i + 1]);
            }
        }
    } else {
        // 如果采样失败，直接连接控制点作为备用方案
        path.moveTo(controlPoints[0], controlPoints[1]);
        for (size_t i = 2; i < controlPoints.size(); i += 2) {
            if (i + 1 < controlPoints.size()) {
                path.lineTo(controlPoints[i], controlPoints[i + 1]);
            }
        }
    }

    return path;
}

QPainterPath BSplineItem::createPathFromControlPoints(const std::vector<cv::Point2f>& controlPoints)
{
    QPainterPath path;

    if (controlPoints.size() < 2) {
        return path;
    }

    // 创建一个直接的路径，连接所有控制点
    path.moveTo(controlPoints[0].x, controlPoints[0].y);
    for (size_t i = 1; i < controlPoints.size(); ++i) {
        path.lineTo(controlPoints[i].x, controlPoints[i].y);
    }

    return path;
}

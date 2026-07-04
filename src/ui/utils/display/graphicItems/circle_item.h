#ifndef CIRCLE_ITEM_H
#define CIRCLE_ITEM_H

#include <QColor>
#include <opencv2/core/core.hpp>

#include "graphic_item_component.h"

// 按实际直径绘制圆轮廓（Blob/Hough 检测结果可视化）
class CircleItem : public GraphicsItemComponent {
public:
    CircleItem(const std::vector<cv::Point2d>& centers, const std::vector<double>& radius, const QColor& color = Qt::red, double lineWidth = 1.0,
               double zValue = 15.0);

    QList<QGraphicsItem*> getGraphicsItems() const override;
};

#endif

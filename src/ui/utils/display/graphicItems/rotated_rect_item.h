#ifndef ROTATED_RECT_ITEM_H
#define ROTATED_RECT_ITEM_H

#include "graphic_item_component.h"
#include <opencv2/opencv.hpp>
#include <QColor>

class RotatedRectItem : public GraphicsItemComponent
{
public:
    RotatedRectItem(const std::vector<cv::RotatedRect>& rotatedRects,
                    const QColor& color = Qt::green,
                    double lineWidth = 2.0,
                    double zValue = 10.0);

    ~RotatedRectItem() override;

    // 获取所有图形图元项
    QList<QGraphicsItem*> getGraphicsItems() const override;

private:
    std::vector<cv::RotatedRect> m_rotatedRects;  // 旋转矩形数据
};

#endif // ROTATED_RECT_ITEM_H

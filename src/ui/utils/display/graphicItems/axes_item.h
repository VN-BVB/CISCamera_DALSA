#ifndef AXES_ITEM_H
#define AXES_ITEM_H

#include "graphic_item_component.h"

// 亚像素精度平台坐标轴组件（全 double，无 float 截断）
class AxesItem : public GraphicsItemComponent {
public:
    AxesItem(double cx, double cy,       // 旋转中心 (像素)
             double xx, double xy,       // X 轴终点
             double yx, double yy,       // Y 轴终点
             int platformId);

    QList<QGraphicsItem*> getGraphicsItems() const override;
};

#endif  // AXES_ITEM_H

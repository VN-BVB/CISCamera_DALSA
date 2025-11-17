#ifndef GRAPHIC_ITEM_COMPONENT_H
#define GRAPHIC_ITEM_COMPONENT_H

#include <QGraphicsScene>
#include <QList>
#include <memory>

// 组合模式管理各种图形图元
// 图形图元接口
class GraphicsItemComponent
{
public:
    GraphicsItemComponent();
    virtual ~GraphicsItemComponent() = default;

    // 添加到场景
    virtual void addToScene(QGraphicsScene* scene) = 0;

    // 从场景移除
    virtual void removeFromScene(QGraphicsScene* scene) = 0;

    // 获取所有图形图元项
    virtual QList<QGraphicsItem*> getGraphicsItems() const = 0;
};

#endif // GRAPHIC_ITEM_COMPONENT_H

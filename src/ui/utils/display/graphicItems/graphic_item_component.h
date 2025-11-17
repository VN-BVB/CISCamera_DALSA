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

    // 组件管理方法
    virtual void addComponent(std::shared_ptr<GraphicsItemComponent> component) {}
    virtual void removeComponent(std::shared_ptr<GraphicsItemComponent> component) {}
    virtual void clearComponents() {}

    // 获取所有图形图元项
    virtual QList<QGraphicsItem*> getGraphicsItems() const = 0;
};

#endif // GRAPHIC_ITEM_COMPONENT_H

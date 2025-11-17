#ifndef GRAPHIC_ITEM_COMPOSITE_H
#define GRAPHIC_ITEM_COMPOSITE_H

#include <QGraphicsItem>

#include "graphic_item_component.h"

class GraphicItemComposite : public GraphicsItemComponent
{
public:
    GraphicItemComposite();
    void addComponent(std::shared_ptr<GraphicsItemComponent> component) override;
    void removeComponent(std::shared_ptr<GraphicsItemComponent> component) override;
    void clearComponents() override;

    // 仅在复合类中实现场景管理
    void addToScene(QGraphicsScene* scene);
    void removeFromScene(QGraphicsScene* scene);

    QList<QGraphicsItem*> getGraphicsItems() const override;

private:
    QList<std::shared_ptr<GraphicsItemComponent>> m_components;
};

#endif // GRAPHIC_ITEM_COMPOSITE_H

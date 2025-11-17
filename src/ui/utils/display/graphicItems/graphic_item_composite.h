#ifndef GRAPHIC_ITEM_COMPOSITE_H
#define GRAPHIC_ITEM_COMPOSITE_H

#include <QGraphicsItem>

#include "graphic_item_component.h"

class GraphicItemComposite : public GraphicsItemComponent
{
public:
    GraphicItemComposite();
    void addComponent(std::shared_ptr<GraphicsItemComponent> component);
    void removeComponent(std::shared_ptr<GraphicsItemComponent> component);
    void clearComponents();

    void addToScene(QGraphicsScene* scene) override;
    void removeFromScene(QGraphicsScene* scene) override;
    QList<QGraphicsItem*> getGraphicsItems() const override;

private:
    QList<std::shared_ptr<GraphicsItemComponent>> m_components;
};

#endif // GRAPHIC_ITEM_COMPOSITE_H

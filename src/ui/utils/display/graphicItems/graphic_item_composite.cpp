#include "graphic_item_composite.h"

GraphicItemComposite::GraphicItemComposite() {}

void GraphicItemComposite::addComponent(std::shared_ptr<GraphicsItemComponent> component)
{
    if (component) {
        m_components.append(component);
    }
}

void GraphicItemComposite::removeComponent(std::shared_ptr<GraphicsItemComponent> component)
{
    m_components.removeOne(component);
}

void GraphicItemComposite::clearComponents()
{
    m_components.clear();
}

void GraphicItemComposite::addToScene(QGraphicsScene *scene)
{
    for (auto& component : m_components) {
        component->addToScene(scene);
    }
}

void GraphicItemComposite::removeFromScene(QGraphicsScene *scene)
{
    for (auto& component : m_components) {
        component->removeFromScene(scene);
    }
}

QList<QGraphicsItem*> GraphicItemComposite::getGraphicsItems() const
{
    QList<QGraphicsItem*> items;
    for (auto& component : m_components) {
        items.append(component->getGraphicsItems());
    }
    return items;
}

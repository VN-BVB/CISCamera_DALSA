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
        QList<QGraphicsItem*> items = component->getGraphicsItems();
        for (auto& item : items) {
            if (item && !scene->items().contains(item)) {
                scene->addItem(item);
            }
        }
    }
}

void GraphicItemComposite::removeFromScene(QGraphicsScene *scene)
{
    for (auto& component : m_components) {
        QList<QGraphicsItem*> items = component->getGraphicsItems();
        for (auto& item : items) {
            if (item && scene->items().contains(item)) {
                scene->removeItem(item);
            }
        }
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

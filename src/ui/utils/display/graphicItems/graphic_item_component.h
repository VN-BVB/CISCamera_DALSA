#ifndef GRAPHIC_ITEM_COMPONENT_H
#define GRAPHIC_ITEM_COMPONENT_H

#include <QGraphicsScene>
#include <QList>
#include <QColor>
#include <memory>

// 组合模式管理各种图形图元
// 图形图元接口
class GraphicsItemComponent
{
public:
    GraphicsItemComponent(const QColor& color = Qt::black,
                          double lineWidth = 1.0,
                          Qt::PenStyle lineStyle = Qt::SolidLine,
                          double zValue = 10.0);
    virtual ~GraphicsItemComponent() = default;

    // 组件管理方法
    virtual void addComponent(std::shared_ptr<GraphicsItemComponent> component) {}
    virtual void removeComponent(std::shared_ptr<GraphicsItemComponent> component) {}
    virtual void clearComponents() {}

    // 获取所有图形图元项
    virtual QList<QGraphicsItem*> getGraphicsItems() const = 0;

    // 共同属性的设置和获取
    void setColor(const QColor& color);
    QColor getColor() const;

    void setLineWidth(double width);
    double getLineWidth() const;

    void setLineStyle(Qt::PenStyle style);
    Qt::PenStyle getLineStyle() const;

    void setZValue(double zValue);
    double getZValue() const;
protected:
    mutable QList<QGraphicsItem*> m_graphicsItems;

    // 共同属性
    QColor m_color;
    double m_lineWidth;
    Qt::PenStyle m_lineStyle;
    double m_zValue;

    virtual void applyProperties();
};

#endif // GRAPHIC_ITEM_COMPONENT_H

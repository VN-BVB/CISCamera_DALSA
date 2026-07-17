#include "axes_item.h"

#include <QColor>
#include <QFont>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QPen>

AxesItem::AxesItem(double cx, double cy, double xx, double xy, double yx, double yy, int platformId) : GraphicsItemComponent() {
    // X 轴 (红) — 粗线，缩略图可见
    auto* xLine = new QGraphicsLineItem(cx, cy, xx, xy);
    xLine->setPen(QPen(QColor(220, 30, 30), 1.0, Qt::SolidLine, Qt::RoundCap));

    // Y 轴 (绿)
    auto* yLine = new QGraphicsLineItem(cx, cy, yx, yy);
    yLine->setPen(QPen(QColor(30, 180, 30), 1.0, Qt::SolidLine, Qt::RoundCap));

    // 十字准星 (白)
    auto* hCross = new QGraphicsLineItem(cx - 5, cy, cx + 5, cy);
    hCross->setPen(QPen(Qt::black, 1.0));
    auto* vCross = new QGraphicsLineItem(cx, cy - 5, cx, cy + 5);
    vCross->setPen(QPen(Qt::black, 1.0));

    // 标签 — 大字体
    auto* label = new QGraphicsTextItem(QString("P%1").arg(platformId));
    label->setPos(cx + 10, cy - 10);
    label->setDefaultTextColor(Qt::yellow);
    label->setFont(QFont("Arial", 20, QFont::Bold));

    m_graphicsItems = {xLine, yLine, hCross, vCross, label};
    applyProperties();
}

QList<QGraphicsItem*> AxesItem::getGraphicsItems() const { return m_graphicsItems; }

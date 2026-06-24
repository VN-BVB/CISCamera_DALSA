#include "axes_item.h"
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QColor>
#include <QFont>

AxesItem::AxesItem(double cx, double cy, double xx, double xy, double yx, double yy, int platformId)
    : GraphicsItemComponent() {
    // X 轴 (红) — 粗线，缩略图可见
    auto* xLine = new QGraphicsLineItem(cx, cy, xx, xy);
    xLine->setPen(QPen(QColor(220, 30, 30), 30.0, Qt::SolidLine, Qt::RoundCap));

    // Y 轴 (绿)
    auto* yLine = new QGraphicsLineItem(cx, cy, yx, yy);
    yLine->setPen(QPen(QColor(30, 180, 30), 30.0, Qt::SolidLine, Qt::RoundCap));

    // 十字准星 (白)
    auto* hCross = new QGraphicsLineItem(cx - 80, cy, cx + 80, cy);
    hCross->setPen(QPen(Qt::white, 20.0));
    auto* vCross = new QGraphicsLineItem(cx, cy - 80, cx, cy + 80);
    vCross->setPen(QPen(Qt::white, 20.0));

    // 标签 — 大字体
    auto* label = new QGraphicsTextItem(QString("P%1").arg(platformId));
    label->setPos(cx + 100, cy - 200);
    label->setDefaultTextColor(Qt::yellow);
    label->setFont(QFont("Arial", 80, QFont::Bold));

    m_graphicsItems = {xLine, yLine, hCross, vCross, label};
}

QList<QGraphicsItem*> AxesItem::getGraphicsItems() const {
    return m_graphicsItems;
}

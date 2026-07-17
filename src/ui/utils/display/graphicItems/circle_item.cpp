#include "circle_item.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QPen>

CircleItem::CircleItem(const std::vector<cv::Point2d>& centers, const std::vector<double>& radius, const QColor& color, double lineWidth,
                       double zValue, const std::vector<QString>& offsetLabels)
    : GraphicsItemComponent(color, lineWidth, Qt::SolidLine, zValue) {
    size_t n = std::min(centers.size(), radius.size());
    for (size_t i = 0; i < n; i++) {
        double r = radius[i] / 2.0;
        double cx = centers[i].x;
        double cy = centers[i].y;

        // 圆轮廓
        auto* ellipse = new QGraphicsEllipseItem(cx - r, cy - r, r * 2, r * 2);
        ellipse->setPen(QPen(color, lineWidth));
        ellipse->setBrush(Qt::NoBrush);

        // 十字线（长度 = 直径）
        auto* hLine = new QGraphicsLineItem(cx - r, cy, cx + r, cy);
        auto* vLine = new QGraphicsLineItem(cx, cy - r, cx, cy + r);
        hLine->setPen(QPen(color, lineWidth));
        vLine->setPen(QPen(color, lineWidth));

        // // 圆心小点
        // double dotSize = 1.0;
        // auto* dot = new QGraphicsEllipseItem(cx - dotSize / 2, cy - dotSize / 2, dotSize, dotSize);
        // dot->setBrush(QBrush(color));
        // dot->setPen(QPen(Qt::NoPen));

        // 偏移量文本标签
        QString text;
        if (i < offsetLabels.size() && !offsetLabels[i].isEmpty())
            text = offsetLabels[i];
        else
            text = QString("(%1,%2)").arg(cx, 0, 'f', 1).arg(cy, 0, 'f', 1);

        auto* label = new QGraphicsTextItem(text);
        label->setPos(cx + 5, cy - 10);
        label->setDefaultTextColor(Qt::black);
        label->setFont(QFont("Arial", 2, QFont::Bold));

        // 圆心坐标文本标签
        QString centerText;
        centerText = QString("(%1,%2)").arg(cx, 0, 'f', 1).arg(cy, 0, 'f', 1);

        auto* leftLabel = new QGraphicsTextItem(centerText);
        leftLabel->setPos(cx + 5, cy - 15);
        leftLabel->setDefaultTextColor(Qt::black);
        leftLabel->setFont(QFont("Arial", 2, QFont::Bold));

        m_graphicsItems.append(ellipse);
        m_graphicsItems.append(hLine);
        m_graphicsItems.append(vLine);
        m_graphicsItems.append(label);
        m_graphicsItems.append(leftLabel);
        // m_graphicsItems.append(dot);
    }
    applyProperties();
}

QList<QGraphicsItem*> CircleItem::getGraphicsItems() const { return m_graphicsItems; }

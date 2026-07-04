#include "circle_item.h"

#include <QGraphicsEllipseItem>
#include <QPen>

CircleItem::CircleItem(const std::vector<cv::Point2d>& centers, const std::vector<double>& radius, const QColor& color, double lineWidth,
                       double zValue)
    : GraphicsItemComponent(color, lineWidth, Qt::SolidLine, zValue) {
    size_t n = std::min(centers.size(), radius.size());
    for (size_t i = 0; i < n; i++) {
        double r = radius[i] / 2.0;
        auto* ellipse = new QGraphicsEllipseItem(centers[i].x - r, centers[i].y - r, r * 2, r * 2);
        ellipse->setPen(QPen(color, lineWidth));
        ellipse->setBrush(Qt::NoBrush);  // 空心，只画轮廓

        // 圆心小点
        double dotSize = 1.0;
        auto* dot = new QGraphicsEllipseItem(centers[i].x - dotSize / 2, centers[i].y - dotSize / 2, dotSize, dotSize);
        dot->setBrush(QBrush(color));  // 实心填充，跟圆轮廓同色
        dot->setPen(QPen(Qt::NoPen));  // 无边框

        auto* label = new QGraphicsTextItem(QString("(%1,%2)\n").arg(centers[i].x).arg(centers[i].y));
        label->setPos(centers[i].x + 10, centers[i].y - 10);
        label->setDefaultTextColor(Qt::black);
        label->setFont(QFont("Arial", 2, QFont::Bold));

        m_graphicsItems.append(ellipse);
        m_graphicsItems.append(label);
        m_graphicsItems.append(dot);
    }
    applyProperties();
}

QList<QGraphicsItem*> CircleItem::getGraphicsItems() const { return m_graphicsItems; }

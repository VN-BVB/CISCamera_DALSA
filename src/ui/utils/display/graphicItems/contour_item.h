#ifndef CONTOUR_ITEM_H
#define CONTOUR_ITEM_H

#include <opencv2/core/core.hpp>
#include <QColor>
#include <Qt>

#include "graphic_item_component.h"

// 轮廓组件，组合模式中的leaf
class ContourItem : public GraphicsItemComponent
{
public:
    enum ContourType {
        subpixelContour,
        pixelContour
    };

    ContourItem(const std::vector<cv::Point2f>& contour,
                ContourType type,
                const QColor& color = Qt::transparent,
                double lineWidth = -1.0,
                Qt::PenStyle lineStyle = Qt::SolidLine,
                double zValue = 10.0);

    ContourItem(const std::vector<cv::Point> &contour,
                ContourType type,
                const QColor& color = Qt::transparent,
                double lineWidth = -1.0,
                Qt::PenStyle lineStyle = Qt::SolidLine,
                double zValue = 10.0);

    QList<QGraphicsItem*> getGraphicsItems() const override;

    void applyProperties() override;

private:
    ContourType m_type;
};

#endif // CONTOUR_ITEM_H

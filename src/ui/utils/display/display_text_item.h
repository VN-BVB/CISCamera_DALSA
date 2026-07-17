#ifndef DISPLAY_TEXT_ITEM_H
#define DISPLAY_TEXT_ITEM_H

#include "display_global.h"
#include <QGraphicsTextItem>
#include <QString>
#include <QColor>

#define DisplayTextItem_Type InteractiveGraphicsItemUserType+2

class DisplayTextItem : public QGraphicsTextItem
{
    Q_OBJECT

public:
    explicit DisplayTextItem(QGraphicsItem *parent = nullptr);
    ~DisplayTextItem();

    // 设置文本内容和属性
    void setText(const QString &text, const double &size = 1, const QColor &color = QColor(Qt::green));

    // Qt图形项类型标识
    enum { Type = DisplayTextItem_Type };
    int type() const override { return Type; }
};

#endif // DISPLAY_TEXT_ITEM_H

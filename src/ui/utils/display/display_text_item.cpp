#include <QPen>
#include <QFont>
#include "display_text_item.h"

DisplayTextItem::DisplayTextItem(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
}

DisplayTextItem::~DisplayTextItem()
{}

void DisplayTextItem::setText(const QString &text, const double &size, const QColor &color)
{
    // 设置文本内容
    setPlainText(text);

    // 设置文本颜色 - 使用正确的方法
    setDefaultTextColor(color);

    // 设置文本大小
    QFont font = this->font();
    font.setPointSizeF(font.pointSizeF() * size);
    this->setFont(font);
}

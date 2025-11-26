#include "display_image_item.h"
#include <QPen>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>

/*************************/
//SDisplayText
/*************************/
// 显示文本结构体
struct SDisplayText
{
    SDisplayText()
    {
        text = "";
        pos = QPointF(0, 0);
        factor = 1.0;
        color = QColor(Qt::green);
    }

    SDisplayText(const QString &text, const QPointF &pos, const double &factor, const QColor &color)
        :text(text), pos(pos), factor(factor), color(color)
    {

    }

    QString text;   // 显示文本
    QPointF pos;    // 显示位置
    double factor;  // 显示文本大小系数
    QColor color;   // 显示文本颜色
};

/*********************************/
//DisplayImageItemPrivate
/*********************************/
class DisplayImageItemPrivate
{
    Q_DISABLE_COPY(DisplayImageItemPrivate)
    Q_DECLARE_PUBLIC(DisplayImageItem)
public:
    DisplayImageItemPrivate(DisplayImageItem *q):q_ptr(q)
    {
        lstDisplayText.clear();
    }
    virtual ~DisplayImageItemPrivate() {}

public:
    DisplayImageItem *const q_ptr;
    QList<SDisplayText> lstDisplayText;
};

/*************************/
//DisplayImageItem
/*************************/
DisplayImageItem::DisplayImageItem(QObject *parent)
    :QObject(parent),
    QGraphicsPixmapItem(nullptr),
    d_ptr(new DisplayImageItemPrivate(this))
{
    setAcceptHoverEvents(true);
}

DisplayImageItem::~DisplayImageItem()
{

}

// 显示图像
bool DisplayImageItem::displayImage(const QImage &image)
{
    if (image.isNull()) return false;
    setPixmap(QPixmap::fromImage(image));
    return true;
}

// 清除图像
void DisplayImageItem::clearImage()
{
    setPixmap(QPixmap());
}

// 添加显示文本
void DisplayImageItem::addDisplayText(const QString &text,
                                          const QPointF &pt,
                                          const double &size,
                                          const QColor &color,
                                          const bool &clear)
{
    Q_D(DisplayImageItem);  // 使用Qt的PIMPL宏获取指向私有实现类的指针
    if (clear)
    {
        d->lstDisplayText.clear();
    }
    d->lstDisplayText.append(SDisplayText(text, pt, size, color));
    this->update();
}

// 清除文本
void DisplayImageItem::clearDisplayText()
{
    Q_D(DisplayImageItem);
    d->lstDisplayText.clear();
    this->update();
}

QPointF DisplayImageItem::getDisplayImageCenter() const
{
    auto width = this->pixmap().width();
    auto height = this->pixmap().height();
    QPointF center = QPointF(width / 2.0, height / 2.0);
    return mapToScene(center);
}

// 基类绘制方法，用于自定义图元的绘制逻辑
void DisplayImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPixmapItem::paint(painter, option, widget);    // 调用基类的绘制方法，绘制基础原始图像
    Q_D(DisplayImageItem);
    foreach (auto displayText, d->lstDisplayText) {
        painter->save();
        auto pen = painter->pen();
        pen.setColor(displayText.color);
        painter->setPen(pen);

        auto font = painter->font();
        auto newPointSize = font.pointSize() * displayText.factor;
        font.setPointSize(newPointSize);
        painter->setFont(font);

        // 文本绘制大小，格式，位置，居中等属性设置
        QFontMetricsF metrics(font);
        auto size = metrics.size(Qt::TextExpandTabs, displayText.text);
        painter->drawText(QRectF(displayText.pos, size), Qt::AlignCenter, displayText.text);
        painter->restore();
    }
}

// 处理鼠标在图元上的悬停事件
void DisplayImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF scenePos = event->scenePos();     // 返回场景坐标系中的坐标，与海康与halcon的效果相同，若是返回图元坐标系中的坐标，则鼠标通过像素块中心时才会产生坐标变化
    QPointF itemPos = mapFromScene(scenePos); // 转换为图元坐标系

    // 检查坐标是否在图像范围内
    QPixmap pix = pixmap();
    if (!pix.isNull())
    {
        QRectF imageRect = boundingRect();
        if (imageRect.contains(itemPos))
        {
            // 坐标在图像范围内，发送信号
            emit this->sendHoverImagePosition(scenePos.toPoint());
        }
        else
        {
            // 坐标超出图像范围，发送离开信号
            emit this->sendHoverLeave();
        }
    }

    return QGraphicsPixmapItem::hoverMoveEvent(event);
}

// 处理鼠标离开图形项时的悬停事件
void DisplayImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit this->sendHoverLeave();
    return QGraphicsPixmapItem::hoverLeaveEvent(event);
}












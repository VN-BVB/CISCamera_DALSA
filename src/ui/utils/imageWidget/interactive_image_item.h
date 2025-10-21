#ifndef INTERACTIVE_IMAGE_ITEM_H
#define INTERACTIVE_IMAGE_ITEM_H

#include "interactive_global.h"
#include <QGraphicsPixmapItem>
#include <QPen>
#define InteractiveImageItem_Type InteractiveGraphicsItemUserType+1
class InteractiveImageItemPrivate;
class InteractiveImageItem :  public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

    friend class InteractiveScene;
public:
    explicit InteractiveImageItem(QObject *parent = nullptr);
    ~InteractiveImageItem();

protected:
    // 显示图像
    bool displayImage(const QImage &image);
    // 清除图像
    void clearImage();
    // 显示文本
    void addDisplayText(const QString &text,const QPointF &pt=QPointF(0,0),const double &size=1,
                        const QColor &color=QColor(Qt::green),const bool &clear=false);
    // 清除文本
    void clearDisplayText();
public:
    // Qt图形项（QGraphicsItem）系统中用于类型标识的标准实现
    // 在Qt图形视图框架中，每个图形项都需要有唯一的类型标识，能够调用type（）方法确定是否是InteractiveImageItem类型图元
    enum { Type = InteractiveImageItem_Type };
    int type() const override
    {
        return Type;
    }
    // 获取图像尺寸
    QSize getDisplayImageSize() const {return this->pixmap().size();}
    // 获取图像显示总线
    QPointF getDisplayImageCenter() const;
public:
    // 基类绘制方法，用于自定义图元的绘制逻辑
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
protected:
    // 处理鼠标在图元上的悬停事件
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    // 处理鼠标离开图形项时的悬停事件
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
signals:
    // 悬停时图像坐标
    void sendHoverImagePosition(const QPoint &pt);
    // 悬停离开
    void sendHoverLeave();
protected:
    const QScopedPointer<InteractiveImageItemPrivate> d_ptr;
private:
    Q_DECLARE_PRIVATE(InteractiveImageItem)
};

#endif // INTERACTIVE_IMAGE_ITEM_H

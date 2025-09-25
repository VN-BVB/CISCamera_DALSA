#ifndef INTERACTIVESCENE_H
#define INTERACTIVESCENE_H

#include <QGraphicsScene>

class InteractiveView;
class InteractiveImageItem;
class InteracitveScenePrivate;
class InteractiveScene : public QGraphicsScene
{
    Q_OBJECT

    friend class InteractiveView;   // 双向友元虽然破坏了封装性，但在View和Scene这种紧密耦合的框架组件中是合理的，可以简化实现，避免调用公共接口产生的开销
public:
    explicit InteractiveScene(InteractiveView *parentView = nullptr);
    ~InteractiveScene();
public:
    // 获取视图
    InteractiveView* getView() const {return m_parentView;}
    // 获取图像显示图元
    InteractiveImageItem* getDisplayImageItem() const {return m_displayImageItem;}
    // 获取显示的图像
    QPixmap getDisplayImage();
    // 获取显示的图像的尺寸
    QSize getDisplayImageSize();

public slots:
    // 显示图像
    bool displayImage(const QImage &image, bool bAutoFit = false);
    // 清除图像
    void clearImage();
    // 显示文本
    void addDisplayText(const QString &text, const QPointF &pt=QPointF(0,0), const double &size=1,
                        const QColor &color=QColor(Qt::green), const bool &clear=false);
    // 清除文本
    void clearDisplayText();
protected:
    // 设置显示图像图元
    void setDisplayImageItem(InteractiveImageItem* imageItem);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
signals:
    // 更新显示图像
    void sendUpdateDisplayImage(const QImage &image);
    // 清除图像
    void sendClearDisplayImage();
    // 鼠标点击信号
    void sendMousePress(QGraphicsSceneMouseEvent* event);
    void sendMouseRelease(QGraphicsSceneMouseEvent* event);
protected:
    // 父视图
    InteractiveView *m_parentView = nullptr;
    // 图像显示图元
    InteractiveImageItem *m_displayImageItem = nullptr;
protected:
    const QScopedPointer<InteracitveScenePrivate> d_ptr;    // Qt的智能指针
private:
    Q_DECLARE_PRIVATE(InteracitveScene) // PIMPL设计模式，将类的实现细节隐藏在一个单独的私有类中，隐藏实现细节，加快编译速度
};

#endif // INTERACTIVESCENE_H

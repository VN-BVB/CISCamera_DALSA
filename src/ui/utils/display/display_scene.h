#ifndef DISPLAY_SCENE_H
#define DISPLAY_SCENE_H

#include <QGraphicsScene>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QList>

#include "tinysplinecxx.h"
#include "src/jointDetection/contourProcess/methods/curve_seg.h"
#include "graphicItems/graphic_item_component.h"
#include "graphicItems/contour_item.h"

class DisplayView;
class DisplayImageItem;
class DisplayScenePrivate;
class GraphicItemComposite;
class DisplayTextItem;
class DisplayScene : public QGraphicsScene
{
    Q_OBJECT

    friend class DisplayView;   // 双向友元虽然破坏了封装性，但在View和Scene这种紧密耦合的关系组件中是合理的，可以简化实现，避免调用公共接口产生的开销
public:
    explicit DisplayScene(DisplayView *parentView = nullptr);
    ~DisplayScene();
public:
    // 获取视图
    DisplayView* getView() const {return m_parentView;}
    // 获取图像显示图元（主图元）
    DisplayImageItem* getDisplayImageItem() const {return m_displayImageItem;}
    // 获取所有图像显示图元
    QList<DisplayImageItem*> getAllDisplayImageItems() const {return m_displayImageItems;}
    // 获取显示的图像
    QPixmap getDisplayImage();
    // 获取显示的图像的尺寸
    QSize getDisplayImageSize() const;
    // 获取图形组件组合
    std::shared_ptr<GraphicItemComposite> getGraphicItemComposite() const { return m_graphicItemComposite; }

    // =====================================图像显示槽函数=====================================
public slots:
    bool whenDisplayImage(const QImage &image, bool bAutoFit = false);
    void whenClearImage();
    DisplayImageItem* whenAddDisplayImage(const QImage &image, const QPointF &pos = QPointF(0, 0), bool bAutoFit = false);
    void whenRemoveDisplayImage(DisplayImageItem* imageItem);
    void whenClearAllDisplayImages();

    // =====================================文本显示槽函数=====================================
public slots:
    // 添加文本图元
    DisplayTextItem* whenAddDisplayTextItem(const QString &text, const QPointF &pt=QPointF(0,0),
                                            const double &size=1, const QColor &color=QColor(Qt::green));
    // 移除文本项
    void whenRemoveDisplayTextItem(DisplayTextItem* textItem);
    // 清除所有文本项
    void whenClearAllDisplayTextItems();

    // =====================================图形显示槽函数=====================================
public slots:
    void whenAddGraphicComponent(std::shared_ptr<GraphicsItemComponent> component);
    void whenRemoveGraphicComponent(std::shared_ptr<GraphicsItemComponent> component);
    void whenClearAllGraphicComponents();

protected:
    // 设置显示图像图元（主图元）
    void setDisplayImageItem(DisplayImageItem* imageItem);
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
    DisplayView *m_parentView = nullptr;
    // 主图像显示图元
    DisplayImageItem *m_displayImageItem = nullptr;
    // 所有图像显示图元列表
    QList<DisplayImageItem*> m_displayImageItems;
    // 文本图元列表
    QList<DisplayTextItem*> m_displayTextItems;
    // 图形图元组合管理器
    std::shared_ptr<GraphicItemComposite> m_graphicItemComposite;
protected:
    const QScopedPointer<DisplayScenePrivate> d_ptr;    // Qt的智能指针
private:
    Q_DECLARE_PRIVATE(DisplayScene) // PIMPL设计模式，将类的实现细节隐藏在一个单独的私有类中，隐藏实现细节，加快编译速度
};

#endif // DISPLAY_SCENE_H

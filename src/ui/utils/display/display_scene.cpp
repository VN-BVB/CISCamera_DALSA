#include <QGraphicsItem>
#include <QGraphicsProxyWidget>

#include "display_scene.h"
#include "display_view.h"
#include "display_image_item.h"
#include "graphicItems/graphic_item_component.h"
#include "graphicItems/graphic_item_composite.h"
#include "graphicItems/line_item.h"
#include "graphicItems/point_item.h"
#include "graphicItems/bspline_item.h"
#include "graphicItems/rotated_rect_item.h"

/*******************************/
// [DisplayScenePrivate]
/*******************************/
class DisplayScenePrivate
{
    Q_DISABLE_COPY(DisplayScenePrivate) // 禁止拷贝赋值操作
    Q_DECLARE_PUBLIC(DisplayScene)  // 设置公共访问权限
public:
    DisplayScenePrivate(DisplayScene* q):q_ptr(q)
    {
        roiDrawing = false;
    }
    virtual ~DisplayScenePrivate(){}
public:
    DisplayScene* const q_ptr;
    bool roiDrawing;
};

/*******************************/
// [DisplayScene]
/*******************************/
DisplayScene::DisplayScene(DisplayView *parentView)
    :QGraphicsScene(parentView),    // 派生类调用父类构造函数
    m_parentView(parentView),
    m_displayImageItem(new DisplayImageItem(this)),
    m_graphicItemComposite(std::make_shared<GraphicItemComposite>()),
    d_ptr(new DisplayScenePrivate(this))
{
    m_parentView->setScene(this);
    setDisplayImageItem(m_displayImageItem);
}

DisplayScene::~DisplayScene()
{}

bool DisplayScene::whenDisplayImage(const QImage &image, bool bAutoFit)
{
    if (!m_displayImageItem) return false;
    emit sendUpdateDisplayImage(image);
    auto bRet =  m_displayImageItem->displayImage(image);
    if (!bRet) return false;
    m_parentView->whenUpdateDisplayFit();    // 更新父视图图像合适尺寸
    if (bAutoFit)
    {
        m_parentView->whenZoomToDisplayFit();
    }
    return true;
}

void DisplayScene::whenClearImage()
{
    if (!m_displayImageItem) return ;
    m_displayImageItem->clearImage();
    emit sendClearDisplayImage();
}

void DisplayScene::whenAddDisplayText(const QString &text, const QPointF &pt, const double &size,
                                      const QColor &color, const bool &clear)
{
    if (!m_displayImageItem) return;
    m_displayImageItem->addDisplayText(text, pt, size, color, clear);
}

void DisplayScene::whenClearDisplayText()
{
    if (!m_displayImageItem) return;
    m_displayImageItem->clearDisplayText();
}

QPixmap DisplayScene::getDisplayImage()
{
    return m_displayImageItem->pixmap();
}

QSize DisplayScene::getDisplayImageSize() const
{
    return m_displayImageItem->getDisplayImageSize();
}

void DisplayScene::setDisplayImageItem(DisplayImageItem* imageItem)
{
    // 遍历所有图元，确保只有一个图像图元在显示
    foreach (auto item, this->items())
    {
        if (item->type() == DisplayImageItem::Type)
        {
            this->removeItem(item);
        }
    }
    imageItem->setPos(-0.5, -0.5);  // 向左上角位移半个像素，使像素中心对准场景坐标系中的坐标，因为图元坐标系以像素块中心而不是以像素块左上角为像素坐标，
    this->addItem(imageItem);
    m_parentView->setScene(this);
    if (m_displayImageItem != imageItem)
    {
        m_displayImageItem = imageItem;
    }
}

void DisplayScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendMousePress(event);
    return QGraphicsScene::mousePressEvent(event);
}

void DisplayScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendMouseRelease(event);
    return QGraphicsScene::mouseReleaseEvent(event);
}

// 图元组件管理槽函数实现
void DisplayScene::whenAddGraphicComponent(std::shared_ptr<GraphicsItemComponent> component)
{
    if (component) {
        m_graphicItemComposite->addComponent(component);
        m_graphicItemComposite->addToScene(this);
    }
}

void DisplayScene::whenRemoveGraphicComponent(std::shared_ptr<GraphicsItemComponent> component)
{
    if (component) {
        m_graphicItemComposite->removeComponent(component);
    }
}

void DisplayScene::whenClearAllGraphicComponents()
{
    m_graphicItemComposite->removeFromScene(this);
    m_graphicItemComposite->clearComponents();
}










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
#include "display_text_item.h"

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
    // 将主图像图元添加到列表
    m_displayImageItems.append(m_displayImageItem);
}

DisplayScene::~DisplayScene()
{
    // 清理所有图像图元
    whenClearAllDisplayImages();
}

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

// 新接口实现：添加图像图元并显示图像
DisplayImageItem* DisplayScene::whenAddDisplayImage(const QImage &image, const QPointF &pos, bool bAutoFit)
{
    // 创建新的图像图元
    DisplayImageItem* newImageItem = new DisplayImageItem(this);
    newImageItem->setPos(pos.x() - 0.5, pos.y() - 0.5);  // 向左上角位移半个像素
    
    // 显示图像
    bool bRet = newImageItem->displayImage(image);
    if (bRet)
    {
        this->addItem(newImageItem);
        m_displayImageItems.append(newImageItem);
        
        if (bAutoFit)
        {
            m_parentView->whenZoomToDisplayFit();
        }
    }
    else
    {
        delete newImageItem;
        newImageItem = nullptr;
    }
    
    return newImageItem;
}

// 新接口实现：移除指定图像图元
void DisplayScene::whenRemoveDisplayImage(DisplayImageItem* imageItem)
{
    if (!imageItem) return;
    
    // 从场景中移除
    this->removeItem(imageItem);
    // 从列表中移除
    m_displayImageItems.removeOne(imageItem);
    
    // 如果移除的是主图像图元，设置新的主图像图元
    if (imageItem == m_displayImageItem)
    {
        if (!m_displayImageItems.isEmpty())
        {
            m_displayImageItem = m_displayImageItems.first();
        }
        else
        {
            m_displayImageItem = nullptr;
        }
    }
    
    // 释放内存
    delete imageItem;
}

// 新接口实现：清除所有图像图元
void DisplayScene::whenClearAllDisplayImages()
{
    // 移除并释放所有图像图元
    foreach (DisplayImageItem* item, m_displayImageItems)
    {
        this->removeItem(item);
        delete item;
    }
    
    // 清空列表并重置主图像图元
    m_displayImageItems.clear();
    m_displayImageItem = nullptr;
    
    emit sendClearDisplayImage();
}

DisplayTextItem* DisplayScene::whenAddDisplayTextItem(const QString &text, const QPointF &pt,
                                                      const double &size, const QColor &color)
{
    // 创建新的文本项
    DisplayTextItem* textItem = new DisplayTextItem();
    textItem->setText(text, size, color);
    textItem->setPos(pt);
    this->addItem(textItem);
    m_displayTextItems.append(textItem);

    return textItem;
}

void DisplayScene::whenRemoveDisplayTextItem(DisplayTextItem* textItem)
{
    if (!textItem || !m_displayTextItems.contains(textItem)) return;

    this->removeItem(textItem);
    m_displayTextItems.removeOne(textItem);
    delete textItem;
}

void DisplayScene::whenClearAllDisplayTextItems()
{
    foreach (DisplayTextItem* textItem, m_displayTextItems) {
        this->removeItem(textItem);
        delete textItem;
    }
    m_displayTextItems.clear();
}

QPixmap DisplayScene::getDisplayImage()
{
    return m_displayImageItem ? m_displayImageItem->pixmap() : QPixmap();
}

QSize DisplayScene::getDisplayImageSize() const
{
    return m_displayImageItem ? m_displayImageItem->getDisplayImageSize() : QSize();
}

void DisplayScene::setDisplayImageItem(DisplayImageItem* imageItem)
{
    // 不再强制只保留一个图像图元，仅设置主图像图元
    if (imageItem)
    {
        if (m_displayImageItem != imageItem)
        {
            // 如果新图像图元还不在场景中，添加它
            if (!this->items().contains(imageItem))
            {
                imageItem->setPos(-0.5, -0.5);  // 向左上角位移半个像素
                this->addItem(imageItem);
                
                // 如果新图像图元还不在列表中，添加到列表
                if (!m_displayImageItems.contains(imageItem))
                {
                    m_displayImageItems.append(imageItem);
                }
            }
            
            m_displayImageItem = imageItem;
        }
    }
    
    m_parentView->setScene(this);
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

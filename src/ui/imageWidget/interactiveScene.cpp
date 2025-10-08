#include "interactiveScene.h"
#include "interactiveView.h"
#include "interactiveImageItem.h"

#include <QGraphicsItem>

/*************************/
// [InteractiveScenePrivate]
/*************************/
class InteractiveScenePrivate
{
    Q_DISABLE_COPY(InteractiveScenePrivate) // 禁止拷贝赋值操作
    Q_DECLARE_PUBLIC(InteractiveScene)  // 设置公共访问权限
public:
    InteractiveScenePrivate(InteractiveScene* q):q_ptr(q)
    {
        roiDrawing = false;
    };
    virtual ~InteractiveScenePrivate(){};
public:
    InteractiveScene* const q_ptr;
    bool roiDrawing;
};

/*************************/
// [InteractiveScene]
/*************************/
InteractiveScene::InteractiveScene(InteractiveView *parentView)
    :QGraphicsScene(parentView),    // 派生类调用父类构造函数
    m_parentView(parentView),
    m_displayImageItem(new InteractiveImageItem(this)),
    d_ptr(new InteractiveScenePrivate(this))
{
    m_parentView->setScene(this);
    setDisplayImageItem(m_displayImageItem);
}

InteractiveScene::~InteractiveScene()
{}

bool InteractiveScene::whenDisplayImage(const QImage &image, bool bAutoFit)
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

void InteractiveScene::whenClearImage()
{
    if (!m_displayImageItem) return ;
    m_displayImageItem->clearImage();
    emit sendClearDisplayImage();
}

void InteractiveScene::whenAddDisplayText(const QString &text, const QPointF &pt, const double &size,
                        const QColor &color, const bool &clear)
{
    if (!m_displayImageItem) return;
    m_displayImageItem->addDisplayText(text, pt, size, color, clear);
}

void InteractiveScene::whenClearDisplayText()
{
    if (!m_displayImageItem) return;
    m_displayImageItem->clearDisplayText();
}

QPixmap InteractiveScene::getDisplayImage()
{
    return m_displayImageItem->pixmap();
}

QSize InteractiveScene::getDisplayImageSize() const
{
    return m_displayImageItem->getDisplayImageSize();
}

void InteractiveScene::setDisplayImageItem(InteractiveImageItem* imageItem)
{
    // 遍历所有图元，确保只有一个图像图元在显示
    foreach (auto item, this->items())
    {
        if (item->type() == InteractiveImageItem::Type)
        {
            this->removeItem(item);
        }
    }
    this->addItem(imageItem);
    if (m_displayImageItem != imageItem)
    {
        m_displayImageItem = imageItem;
    }
}

void InteractiveScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendMousePress(event);
    return QGraphicsScene::mousePressEvent(event);
}

void InteractiveScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit sendMouseRelease(event);
    return QGraphicsScene::mouseReleaseEvent(event);
}
























